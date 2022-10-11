
#include <unistd.h>
#include <dirent.h>
#include <fstream>
#include <sstream>

#ifdef DEBUG
    #include <iostream>
#endif

#include "Server.hpp"
#include "Types.hpp"

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
#ifdef DEBUG
    cout << "Server   " << host() << ':' << port() << " received request\n";
    cout << "Request: " << request.host() << ' ' << request.uri() << '\n';
#endif
    if (request.method() == INVALID)
        return serveError(request, request.error());
    const Location *location(getLocation(request.uri()));
    if (location == NULL)
        return serveRoot(request);
    return serveLocation(request, *location);

    //ADD REDIRECTION
}

/**
 * @brief Serves from root path of server
 * @param request 
 * @return int 0 on success
 */
int Server::serveRoot(Request &request) const {
    cout << "Serving root\n";
    string path("." + root() + request.uri());
#ifdef DEBUG
    cout << "Checking path: " << path << std::endl;
    cout << "Access: " << access(path.data(), R_OK) << std::endl;
    cout << "Is directory: " << isDirectory(path) << std::endl;
#endif
    if (request.method() == GET)
        return mget(request, path);
    if (request.method() == DELETE)
        return mdelete(request, path);
    if (request.method() == POST)
        return mpost(request, path);
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
    if (request.method() == GET)
        return mget(request, path);
    if (request.method() == DELETE)
        return mdelete(request, path);
    if (request.method() == POST)
        return mpost(request, path);
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
        return serveAutoindex(request, path);
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
    sendResponse(request.fd(), "301 Moved Permanently", "text/html", string(), headers);
    return 0;
}

/**
 * @brief Serves a basic autoindex listing
 * @param request 
 * @param path full filepath for reading
 * @return int 0 on success
 */
int Server::serveAutoindex(Request& request, const string& path) const {
    stringstream ss;
    DIR* dir = opendir(path.data());
    if (dir == NULL)
        return serveError(request, 500);
    ss << "<!DOCTYPE html><html>";
    ss << "<head><title>Autoindex</title></head><body>";
    ss << "<h1>Index " << request.uri() << "\n\n\n\n</h1>";
    for (dirent* ent = readdir(dir); ent != NULL; ent = readdir(dir)){
        string filename(ent->d_name);
        if (filename != "." && filename != "..")
            ss << "<p><a href=\"/" << filename << "\">" << filename << "</a></p>";
    }
    ss << "</body></html>";
    sendResponse(request.fd(), "200 OK", "text/html", ss.str());
    request.setStatus(COMPLETED);
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
        serveDefaultError(request, status);
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
    string str(ss.str());
    sendResponse(request.fd(), status, "text/html", str);
    request.setStatus(COMPLETED);
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
    sendResponse(request.fd(), status, "text/html", msg);
    request.setStatus(COMPLETED);
	return 0;
}
