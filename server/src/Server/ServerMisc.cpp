#include <unistd.h>
#include <sys/stat.h>
#include <sstream>

#ifdef DEBUG
    #include <iostream>
#endif

#include "Server.hpp"
#include "uString.hpp"


using namespace std;

/**
 * @brief Checks if server_name matches with host
 * @param name 
 * @return int 1 on match 0 if no match
 */
int Server::checkNames(const string &name) const {
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
const Location *Server::getLocation(const string &uri) const {
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
string generateLocationURI(const string &root, const string &location, const string &request) {
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
ssize_t sendResponse(int fd, const string &status, const string &type,
    const string &message, const vector<string>& headers) {
#ifdef DEBUG
    std::cout << "Sending response:\n";
    std::cout << "HTTP/1.1" << ' ' << status << '\n'
              << "Content-Type: " << type << '\n'
              << "Content-Length: " << message.size() << "\n\n"
              << message << std::endl;
#endif
    std::stringstream ss;
    ss << "HTTP/1.1" << ' ' << status << "\r\n"
       << "Content-Type: " << type << "\r\n"
       << "Content-Length: " << message.size() << "\r\n";
    for (size_t i = 0; i < headers.size(); ++i)
        ss << headers[i] << "\r\n";
    ss << "\r\n" << message;
    std::string response(ss.str());
    return write(fd, response.c_str(), response.length());
}