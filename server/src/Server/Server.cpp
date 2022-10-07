#include "Server.hpp"

//Stat
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio> // remove()

#include <deque>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "Types.hpp"
#include "uString.hpp"
#include "uMethod.hpp"

using namespace std;

//HELPERS

/**
 * @brief Checks if path is a directory
 * @param path 
 * @return int 1 on directory 0 if not
 */
int isDirectory(const string &path) {
    struct stat sb;
    stat(path.data(), &sb);
    return (S_ISDIR(sb.st_mode));
}

/**
 * @brief Generates new URI on matching location
 * @param root location root
 * @param location location uri
 * @param request request uri
 * @return string uri
 */
string generateLocationURI(const std::string &root, const std::string &location, const std::string &request) {
    size_t i = 0;
    while (i < location.size() && location[i] == request[i])
        ++i;
    string req(request.substr(i));
    if (req.size() == 0)
        req += "/";
    return string("." + root + req);
}

/**
 * @brief Sends a respones to fd
 * @param fd 
 * @param status 
 * @param type 
 * @param message 
 * @return ssize_t bytes written 
 */
ssize_t sendResponse(int fd, const std::string &status, const std::string &type, const std::string &message) {
#ifdef DEBUG
    std::cout << "Sending response:\n";
    std::cout << "HTTP/1.1" << ' ' << status << '\n'
              << "Content-Type: " << type << '\n'
              << "Content-Length: " << message.size() << "\n\n"
              << message << std::endl;
#endif
    std::stringstream ss;
    ss << "HTTP/1.1" << ' ' << status << '\n'
       << "Content-Type: " << type << '\n'
       << "Content-Length: " << message.size() << "\n\n"
       << message;
    std::string response(ss.str());
    return write(fd, response.c_str(), response.length());
}

//LOCATION

Location::Location(const LocationConfig &conf)
    : _config(conf) {}

//SERVER

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
    cout << "Location found: " << (location != NULL) << '\n';
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
    if (request.method() == POST){
        ; //?? probably more checks later
        return mpost(request, path);
    }
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
    if (request.method() == GET)
        return mget(request, path);
    if (request.method() == DELETE)
        return mdelete(request, path);
    if (request.method() == POST){
        ; //?? probably more checks later
        mpost(request, path);
    }
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
        path += location.index();
        return mget(request, path);
    }
    // else if (location.autoindex() == true)
    //     ; //Return autoindex cgi html?
    return serveError(request, 403);
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

/**
 * @brief Serves get request
 * @param request 
 * @param path 
 * @return int 0 on success
 */
int Server::mget(Request &request, const string& path) const {
    if (access(path.data(), F_OK) != 0)
        return serveError(request, 404);
    if (access(path.data(), R_OK) != 0)
        return serveError(request, 403);
    if (isDirectory(path))
        return serveError(request, 403);
    ifstream file(path.data());
    if (file.is_open() == false)
        return serveError(request, 500);
    stringstream ss;
    ss << file.rdbuf();
    string str(ss.str());
    sendResponse(request.fd(), "200 OK", getType(path), str);
    request.setStatus(COMPLETED);
    return 0;
}

int Server::multipartUploader(Request& request, const string& path) const {
    request.setStatus(COMPLETED);
    if (request.length() == 0)
        return 0;
    if (path.length() == 0)
        return 0;
    return 0;
}

int Server::mpost(Request& request, const string& path) const {
    request.setStatus(COMPLETED);


    const set<string>& types = request.types();
    for (set<string>::const_iterator it = types.begin();
        it != types.end(); ++it)
        cout << "TYPE: " << *it << '\n';
    cout << "BOUNDARY: " << request.boundary() << '\n';
    cout << "SIZE: " << request.length() << std::endl;

    if (types.find("application/x-www-form-urlencoded") != types.end())
        return 0; //Not inplemented
    else if (types.find("multipart/form-data") != types.end())
        return multipartUploader(request, path);
        
    return 0; //Not implemented - text/plain




    if (request.fd())
        return 0;
    if (path.empty())
        return 0;
}

/**
 * @brief Deletes a file, sends a response
 * @param request 
 * @param path 
 * @return int always 0
 */
int Server::mdelete(Request& request, const string& path) const {
    if (access(path.data(), F_OK) != 0)
        return serveError(request, 404);
    if (access(path.data(), W_OK) != 0)
        return serveError(request, 403);
    if (isDirectory(path))
        return serveError(request, 403);
    if (remove(path.data()) != 0){
        sendResponse(request.fd(), "204 No Content", "text/html", "No content");
        return 0;
    }
    sendResponse(request.fd(), "200 OK", "text/html", "File deleted");
    request.setStatus(COMPLETED);
    return 0;
}

/**
 * @brief Checks if server_name matches with host
 * @param name 
 * @return int 1 on match 0 if no match
 */
int Server::checkNames(const std::string &name) const {
    if (find(_config.server_names.begin(), _config.server_names.end(), name)
        != _config.server_names.end())
        return 1;
    return 0;
}

/**
 * @brief Picks a matching location from server
 * @param uri 
 * @return const Location* if found, NULL if not
 */
const Location *Server::getLocation(const std::string &uri) const {
    if (_locations.size() == 0)
        return NULL;
    deque< string > path(split(uri, "/", true));
    const Location *ptr = NULL;
    size_t current = 0;
    for (size_t i = 0; i < _locations.size(); ++i) {
        deque< string > loc(split(_locations[i].uri(), "/", true));
        size_t match = getMatches(path, loc);
        if (match == (size_t)-1)
            return &_locations[i];
        if (match > current) {
            ptr = &_locations[i];
            current = match;
        }
    }
    return ptr;
}
