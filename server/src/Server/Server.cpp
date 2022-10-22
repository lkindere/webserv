
#include <unistd.h>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <stdio.h>

#ifdef DEBUG
    #include <iostream>
#endif

#include "Server.hpp"
#include "uServer.hpp"
#include "uString.hpp"
#include "Types.hpp"
#include "Cgi.hpp"

using namespace std;

Server::Server(GlobalConfig *global, const ServerConfig &conf)
    : _global(global),
      _config(conf) {
    for (size_t i = 0; i < conf.locations.size(); ++i)
        _locations.push_back(Location(conf.locations[i]));
}

/**
 * @brief Sends a response based on request
 * @param request 
 * @return int 0 on success
 */
int Server::serve(Request &request) const {
    if (request.method() == INVALID)
        return serveError(request, request.error());
    const Location *location(getLocation(request.uri()));
    if (location == NULL)
        return serveRoot(request);
    cout << "Got location: " << location->uri() << endl;
    return serveLocation(request, *location);
}

/**
 * @brief Serves from root path of server
 * @param request 
 * @return int 0 on success
 */
int Server::serveRoot(Request &request) const {
    string path(getCWD() + root() + request.uri());
    if (request.method() == GET)
        return mget(request, path);
    if (request.method() == DELETE)
        return mdelete(request, path);
    return 0;
}

/**
 * @brief Serves from server location
 * @param request 
 * @param location 
 * @return int 0 on success
 */
int Server::serveLocation(Request &request, const Location &location) const {
    string path(generateLocationURI(location.root(), location.uri(), request.uri()));
    if (validMethod(location.methods(), request.method()) == 0)
        return serveError(request, 405);
    if (location.redirect().length() != 0)
        return serveRedirect(request, location);
    if (isDirectory(path))
        return serveDirectory(request, location);
    if (isCGI(location.cgi_extensions(), path))
        return serveCGI(request, path);
    if (request.method() == GET)
        return mget(request, path);
    if (request.method() == DELETE)
        return mdelete(request, path);
    if (request.method() == POST)
        return mpost(request, location);
    return 0;
}

int bufferToFile(Request& request, FILE* filebuffer) {
    fwrite(request.message().data(), request.message().length(), 1, filebuffer);
    request.setPosted(request.postedlength() + request.message().length());
    if (request.postedlength() >= request.contentlength())
        rewind(filebuffer);
    return 0;
}

int parseResponse(string& message, map<string, string>& headers, FILE* filebuffer) {
    message.resize(BUFFER_SIZE);
    size_t bytes_read = fread((void*)message.data(), 1, BUFFER_SIZE, filebuffer);
    if (bytes_read < BUFFER_SIZE)
        message = message.substr(0, bytes_read);
    while (bytes_read == BUFFER_SIZE) {
        string tmp(BUFFER_SIZE, 0);
        bytes_read = fread((void*)tmp.data(), 1, BUFFER_SIZE, filebuffer);
        message += tmp.substr(0, bytes_read);
    }
    size_t start = 0;
    size_t end = 0;
    while (end == 0){
        if (message.substr(start, 2) == "\n\n")
            end = start + 2;
        else if (start + 3 < message.size()){
            if (message.substr(start, 4) == "\r\n\r\n")
                end = start + 4;
        }
        if (++start + 1 >= message.size())
            return 0;
    }
    deque<string> lines = split(message.substr(0, start), "\n", true);
    for (size_t i = 0; i < lines.size(); ++i){
        deque<string> current = split(lines[i], ": ", true);
        for (size_t j = 0; j < current.size(); ++j)
            cout << current[j] << endl;
        if (current.size() != 2)
            return 1;
        headers.insert(make_pair(current[0], current[1]));
    }
    message = message.substr(end);
    return 0;
}

/**
 * @brief Will serve CGI
 * @param request 
 * @param path 
 * @return int 
 */
int Server::serveCGI(Request& request, const string& path) const {
    static map<int, pair<FILE*, FILE*> > filebuffers;
    if (access(path.data(), F_OK) != 0)
        return serveError(request, 404);
    if (access(path.data(), X_OK) != 0)
        return serveError(request, 403);
    map<int, pair<FILE*, FILE*> >::iterator it = filebuffers.find(request.fd());
    if (it == filebuffers.end())
        it = filebuffers.insert(make_pair(request.fd(), make_pair(tmpfile(), tmpfile()))).first;
    if (request.postedlength() < request.contentlength())
        return bufferToFile(request, it->second.first);
    Cgi cgi(generateENV(request, path));
    if (cgi.execute(path, it->second.first, it->second.second) != 0){
        fclose(it->second.first);
        fclose(it->second.second);
        filebuffers.erase(request.fd());
        return serveError(request, 500);
    }
    string message;
    map<string, string> headers;
    if (parseResponse(message, headers, it->second.second)) {
        fclose(it->second.first);
        fclose(it->second.second);
        filebuffers.erase(request.fd());
        return serveError(request, 500);
    }
    fclose(it->second.first);
    fclose(it->second.second);
    filebuffers.erase(request.fd());
    request.setStatus(RESPONDING);
    map<string, string>::const_iterator type = headers.find("Content-Type");
    if (type == headers.end())
        return serveError(request, 500);
    cout << "TYPE: " << type->second << endl;
    request.generateResponse("200 OK", type->second, message);
    request.sendResponse();
    return 0;
}

/**
 * @brief Serves a directory if index is set or autoindex is on
 * @param request 
 * @param location 
 * @return int 0 on success
 */
int Server::serveDirectory(Request &request, const Location &location) const {
    string path(generateLocationURI(location.root(), location.uri(), request.uri()));
    if (path[path.length() - 1] != '/')
        path += "/";
    if (request.method() != GET)
        return serveError(request, 405);
    if (location.index().length() != 0) {
        string indexpath = path + location.index();
        if (access(indexpath.data(), F_OK) == 0)
            return mget(request, indexpath);
    }
    if (location.autoindex() == true)
        return serveAutoindex(request, location, path);
    return serveError(request, 403);
}

/**
 * @brief Sends a 301 redirect, creates location header
 * @param request 
 * @param location 
 * @return int always 0 for now
 */
int Server::serveRedirect(Request& request, const Location& location) const {
    vector<string> headers;
    headers.push_back("Location: " + location.redirect());
    request.generateResponse("301 Moved Permanently", "text/html", string(), headers);
    request.sendResponse();
    return 0;
}

/**
 * @brief Serves a basic autoindex listing
 * @param request 
 * @param path full filepath for reading
 * @return int 0 on success
 */
int Server::serveAutoindex(Request& request, const Location& location, const string& path) const {
    stringstream ss;
    DIR* dir = opendir(path.data());
    if (dir == NULL)
        return serveError(request, 500);
    ss << "<!DOCTYPE html><html>";
    ss << "<head><title>Autoindex</title></head><body>";
    ss << "<h1>Index " << request.uri() << "</h1>";
    for (dirent* ent = readdir(dir); ent != NULL; ent = readdir(dir)){
        string filename(ent->d_name);
        if (filename.size() > 0 && filename[0] != '.')
            ss << "<p><a href=\"" << location.uri() << filename << "\">" << filename << "</a></p>";
    }
    ss << "</body></html>";
    request.generateResponse("200 OK", "text/html", ss.str());
    request.sendResponse();
    return 0;
}

/**
 * @brief Serves an error page
 * @param request 
 * @param error errorcode
 * @return int 0 on success
 */
int Server::serveError(Request &request, short error) const {
    string status(getStatus(error));
    if (errorRoot().size() == 0)
        return serveDefaultError(request, status);
    map< short, string >::const_iterator it(errorPages().find(error));
    if (it == errorPages().end())
        return serveDefaultError(request, status);
    string path("." + errorRoot() + it->second);
    if (isDirectory(path) || access(path.data(), R_OK) != 0)
        return serveDefaultError(request, status);
    ifstream file(path.data());
    if (file.is_open() == false)
        return serveDefaultError(request, status);
    stringstream ss;
    ss << file.rdbuf();
    request.generateResponse(status, "text/html", ss.str());
    request.sendResponse();
    return 0;
}


/**
 * @brief Sends a default errorpage based on error code
 * @param request 
 * @param status 
 * @return int 0 on success
 */
int Server::serveDefaultError(Request &request, const string &status) const {
    string msg("<!DOCTYPE html><html><head><title>");
    msg += status + "</title></head><body><h1>";
    msg += status + "</h1></body></html>";
    request.generateResponse(status, "text/html", msg);
    request.sendResponse();
	return 0;
}
