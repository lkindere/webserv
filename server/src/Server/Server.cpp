
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
#include "serveCGI.hpp"
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
int Server::serve(Request &request) {
    if (request.method() == INVALID)
        return serveError(request, request.error());
    for (vector<pair<string, string> >::const_iterator it = request.cookies().begin();
        it != request.cookies().end(); ++it) {
        int level = _sessions.validCookie(it->first, it->second);
        if (level > request.authentication())
            request.setAuthentication(level);
    }
    const Location *location(getLocation(request.uri()));
    if (location == NULL)
        return serveRoot(request);
    return serveLocation(request, *location);
}

/**
 * @brief Serves from root path of server
 * @param request 
 * @return int 0 on success
 */
int Server::serveRoot(Request &request) {
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
int Server::serveLocation(Request &request, const Location &location) {
    string path(generateLocationURI(location.root(), location.uri(), request.uri()));
    if (request.authentication() < location.authentication())
        return serveCustom(request, "401 Unauthorized", "Your access level is too low to visit this page");
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

/**
 * @brief Will serve CGI
 * @param request 
 * @param path 
 * @return int 
 */
int Server::serveCGI(Request& request, const string& fullpath) const {
    static map<int, pair<FILE*, FILE*> > filebuffers;
    string path = fullpath.substr(0, fullpath.rfind(getPathInfo(fullpath)));
    if (access(path.data(), F_OK) != 0)
        return serveError(request, 404);
    if (access(path.data(), X_OK) != 0)
        return serveError(request, 403);
    map<int, pair<FILE*, FILE*> >::iterator it = filebuffers.find(request.fd());
    if (it == filebuffers.end())
        it = filebuffers.insert(make_pair(request.fd(), make_pair(tmpfile(), tmpfile()))).first;
    if (request.postedlength() < request.contentlength())
        return bufferToFile(request, it->second.first);
    Cgi cgi(generateENV(request, fullpath));
    if (cgi.execute(path, it->second.first, it->second.second) != 0){
        removeFilebuffer(filebuffers, it);
        return serveError(request, 500);
    }
    string message;
    map<string, string> headers;
    if (parseResponse(message, headers, it->second.second) != 0) {
        removeFilebuffer(filebuffers, it);
        return serveError(request, 500);
    }
    removeFilebuffer(filebuffers, it);
    request.setStatus(RESPONDING);
    map<string, string>::const_iterator type = headers.find("Content-Type");
    if (type == headers.end())
        return serveError(request, 500);
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
int Server::serveDirectory(Request &request, const Location &location) {
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
    return serveError(request, 404);
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

/**
 * @brief Custom status and message
 * @param request 
 * @param status 
 * @param message 
 * @return int 
 */
int Server::serveCustom(Request& request, const string& status, const string& message) const {
    string ret = "<!DOCTYPE html><html><head><title>Error</title></head><body><h1>";
    ret += message + "</h1></body></html>";
    request.generateResponse(status, "text/html", ret);
    request.sendResponse();
    return 0;
}
