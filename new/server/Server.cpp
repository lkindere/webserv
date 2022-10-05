#include "Server.hpp"

//Stat
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <deque>
#include <sstream>
#include <fstream>

using namespace std;

//HELPERS

/**
 * @brief Splits lines by delim
 * @param str input str
 * @param delim delimiter
 * @param noempty don't return empty strings
 * @return deque<string> split lines
 */
static deque<string> split(const string& str, const string& delim, bool noempty = false) {
    deque<string> split;
    size_t start = 0;
    size_t end = str.find(delim);
    size_t index = 1;
    while (end != str.npos) {
        string segment(str.substr(start, end - start));
        if (noempty == false || segment.empty() == false)
            split.push_back(segment);
        start = end + delim.length();
        end = str.find(delim, start);
    }
    string segment(str.substr(start));
    if (noempty == false || segment.empty() == false)
        split.push_back(segment);
    return split;
}

/**
 * @brief Count number of equal strings from the start to the end
 * @param path deque<str>
 * @param uri deque<str>
 * @return size_t number of matches or (size_t)-1 on perfect match (all strings match)
 */
static size_t getMatches(deque<string>& path, deque<string>& uri){
    size_t matches = 0;
    for (size_t i = 0; i < path.size() && i < uri.size(); ++i){
        if (path[i] == uri[i])
            ++matches;
        else
            break;
    }
    if (matches == path.size() && matches == uri.size())
        return -1;
    return matches;
}

ssize_t sendResponse(int fd, const std::string& status, const std::string& type, const std::string& message){
#ifdef DEBUG
            std::cout << "Sending response:\n";
            std::cout << "HTTP/1.1" << ' ' << status << '\n' << "Content-Type: " << type << '\n'
                    << "Content-Length: " << message.size() << "\n\n" << message << std::endl;
#endif
            std::stringstream ss;
            ss << "HTTP/1.1" << ' ' << status << '\n' << "Content-Type: " << type << '\n'
                    << "Content-Length: " << message.size() << "\n\n" << message;
            std::string response(ss.str());
            return write(fd, response.c_str(), response.length());
}

//LOCATION

Location::Location(const LocationConfig& conf)
    : _config(conf){}

//SERVER

Server::Server(GlobalConfig* global, const ServerConfig& conf)
    : _global(global), _config(conf){
    for (size_t i = 0; i < conf.locations.size(); ++i)
        _locations.push_back(Location(conf.locations[i]));
}

int     Server::serve(const Request& request) const{
#ifdef DEBUG
    cout << "Server   " << host() << ':' << port() << " received request\n";
    cout << "Request: " << request.host() << ' ' << request.uri() << '\n';
#endif
    if (request.method() == INVALID)
        return serveError(request, 400);
    const Location* location(getLocation(request.uri()));
    cout << "Location found: " << (location != NULL) << '\n';
    if (location == NULL)
        return serveRoot(request);
    return serveLocation(request, *location);
}

int     isDirectory(const string& path){
    struct stat sb;
    stat(path.data(), &sb);
    return (S_ISDIR(sb.st_mode));
}

int     Server::serveRoot(const Request& request) const{
    cout << "Serving root\n";
    string path("." + root() + request.uri());
#ifdef DEBUG
    cout << "Checking path: " << path << std::endl;
    cout << "Access: " << access(path.data(), R_OK) << std::endl;
    cout << "Is directory: " << isDirectory(path) << std::endl;
#endif
    if (request.method() == GET){
        if (access(path.data(), R_OK) != 0)
            return serveError(request, 404);
        if (isDirectory(path))
            return serveError(request, 403);
        get(request, path);
    }
    return 0;
}

int     Server::serveLocation(const Request& request, const Location& location) const{
    cout << "Serving location\n";
    string path("." + location.root() + request.uri());
#ifdef DEBUG
    cout << "Checking path: " << path << std::endl;
    cout << "Access: " << access(path.data(), R_OK) << std::endl;
    cout << "Is directory: " << isDirectory(path) << std::endl;
#endif
    if (request.method() == GET){
        if (access(path.data(), R_OK) != 0)
            return serveError(request, 404);
        if (isDirectory(path))
            return serveDirectory(request, location);
        get(request, path);
    }
    return 0;
}

int     Server::serveDirectory(const Request& request, const Location& location) const{
    cout << "Serving directory\n";
    string path("." + location.root() + request.uri());
    if (path[path.length() - 1] != '/')
        path += "/";
    if (location.index().length() != 0){
        path += location.index();
        cout << "Indexed path: " << path << std::endl;
        if (access(path.data(), R_OK != 0)){
            cout << "No access\n";
            return serveError(request, 404);
        }
        get(request, path);
    }
    else if (location.autoindex() == true)
        return 0; //Autoindex directory
    return serveError(request, 403);
}

int     Server::serveError(const Request& request, int error) const{
    sendResponse(request.fd(), "404 NOT OKAY", "text/html", "Oops");
    return 0;
}

int     Server::get(const Request& request, const std::string& path) const{
    ifstream file(path);
    if (file.is_open() == false){
        cout << "Failed to open\n";
        return serveError(request, 404);
    }
    stringstream ss;
    ss << file.rdbuf();
    string str(ss.str());
    sendResponse(request.fd(), "200 OK", "text/html", str);
    return 0;
}

int     Server::checkNames(const std::string& name) const{
    if (find(_config.server_names.begin(), _config.server_names.end(), name)
        != _config.server_names.end())
        return 1;
    return 0;
}

const Location* Server::getLocation(const std::string& uri) const{
    if (_locations.size() == 0)
        return NULL;
    deque<string>   path(split(uri, "/", true));
    const Location* ptr = NULL;
    size_t          current = 0;
    for (size_t i = 0; i < _locations.size(); ++i){
        deque<string> loc(split(_locations[i].uri(), "/", true));
        size_t match = getMatches(path, loc);
        if (match == -1)
            return &_locations[i];
        if (match > current){
            ptr = &_locations[i];
            current = match;
        }
    }
    return ptr;
}

