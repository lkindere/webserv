#include <unistd.h>
#include <sys/stat.h>
#include <sstream>
#include <algorithm>

// #ifdef DEBUG
    #include <iostream>
// #endif

#include "Server.hpp"
#include "uServer.hpp"
#include "uString.hpp"

using namespace std;

/**
 * @brief Checks if server_name matches with host
 * @param name 
 * @return int 1 on match 0 if no match
 */
int Server::checkNames(const string &name) const {
    if (find(_config.server_names.begin(), _config.server_names.end(), name.c_str())
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
    pair<size_t, size_t>    current(make_pair(0, (size_t)-1));
    for (size_t i = 0; i < _locations.size(); ++i) {
        deque< string > loc(split(_locations[i].uri(), "/", true));
        pair<size_t, size_t> match = getMatches(path, loc);
        if (match.first > current.first
            || (match.first == current.first && match.second < current.second)) {
            ptr = &_locations[i];
            current = match;
        }
    }
    return ptr;
}

/**
 * @brief Generates ENV for CGI
 * @param request 
 * @param location 
 * @param path 
 * @return map<string, string> 
 */
map<string, string> Server::generateENV(const Request& request, const string& path) const {
    map<string, string> env;
    env.insert(make_pair("SERVER_SOFTWARE", "potatoserv/0.01"));
    env.insert(make_pair("SERVER_PROTOCOL", "HTTP/1.1"));
    env.insert(make_pair("GATEWAY_INTERFACE", "CGI/1.1"));
    env.insert(make_pair("SERVER_NAME", host()));
    env.insert(make_pair("SERVER_PORT", itostr(port())));
    env.insert(make_pair("REQUEST_METHOD", toSmethod(request.method())));
    env.insert(make_pair("CONTENT_LENGTH", itostr(request.contentlength())));
    env.insert(make_pair("REMOTE_HOST", getPeer(request.fd())));
    env.insert(make_pair("REMOTE_ADDR", getPeer(request.fd())));
    env.insert(make_pair("SCRIPT_NAME", getScriptname(path)));
    env.insert(make_pair("QUERY_STRING", request.query()));
    env.insert(make_pair("CONTENT_TYPE", request.getHeader("Content-Type")));
    env.insert(make_pair("HTTP_ACCEPT", request.getHeader("Accept")));
    env.insert(make_pair("HTTP_USER_AGENT", request.getHeader("User-Agent")));
    env.insert(make_pair("HTTP_REFERER", request.getHeader("Referer")));
    //Is it really needed?
    env.insert(make_pair("PATH_INFO", ""));
    env.insert(make_pair("PATH_TRANSLATED", ""));
    //Auth not supported
    env.insert(make_pair("AUTH_TYPE", ""));
    env.insert(make_pair("REMOTE_USER", ""));
    env.insert(make_pair("REMOTE_IDENT", ""));


    cout << "\n---ENV:---\n";
    for (map<string, string>::const_iterator it = env.begin();
        it != env.end(); ++it)
        cout << it->first << " : " << it->second << '\n';
    cout << endl;
    return env;
}