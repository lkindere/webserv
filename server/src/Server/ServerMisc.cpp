#include <unistd.h>
#include <sys/stat.h>
#include <sstream>
#include <algorithm>
#include <stdlib.h>

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
        pair<size_t, size_t> match = getMatches(loc, path);
        if (match.first > current.first
            || (match.first == current.first && match.second < current.second)) {
            ptr = &_locations[i];
            current = match;
        }
    }
    return ptr;
}

string getUser(const Request& request, const Sessions& sessions) {
    for (vector<pair<string, string> >::const_iterator it = request.cookies().begin();
        it != request.cookies().end(); ++it) {
        if (it->first == "PotatoServUSER") {
            const User* user = sessions.getUserByCookie(it->second);
            if (user == NULL)
                return string();
            return user->username();
        }
    }
    return string();
}

/**
 * @brief Generates ENV for CGI
 * @param request 
 * @param location 
 * @param path 
 * @return map<string, string> 
 */
vector<string> Server::generateENV(const Request& request, const string& path) const {
    vector<string> env;
    env.push_back(string("SERVER_SOFTWARE") + "=" + "potatoserv/0.01");
    env.push_back(string("SERVER_PROTOCOL") + "=" + "HTTP/1.1");
    env.push_back(string("GATEWAY_INTERFACE") + "=" + "CGI/1.1");
    env.push_back(string("SERVER_SOFTWARE") + "=" + "potatoserv/0.01");
    env.push_back(string("SERVER_NAME") + "=" + host());
    env.push_back(string("SERVER_PORT") + "=" + itostr(port()));
    env.push_back(string("REQUEST_METHOD") + "=" + toSmethod(request.method()));
    env.push_back(string("CONTENT_LENGTH") + "=" + itostr(request.contentlength()));
    env.push_back(string("REMOTE_ADDR") + "=" + getPeer(request.fd()));
    env.push_back(string("SCRIPT_NAME") + "=" + getScriptname(path));
    env.push_back(string("QUERY_STRING") + "=" + request.query());
    env.push_back(string("CONTENT_TYPE") + "=" + request.getHeader("Content-Type"));
    env.push_back(string("HTTP_ACCEPT") + "=" + request.getHeader("Accept"));
    env.push_back(string("HTTP_USER_AGENT") + "=" + request.getHeader("User-Agent"));
    env.push_back(string("HTTP_REFERER") + "=" + request.getHeader("Referer"));
    env.push_back(string("PATH_INFO") + "=" + getPathInfo(path));
    env.push_back(string("PATH_TRANSLATED") + "= " + path);
    env.push_back(string("REMOTE_USER") + "=" + getUser(request, _sessions));
    
    env.push_back(string("REMOTE_HOST") + "=" + "");
    env.push_back(string("AUTH_TYPE") + "=" + "");
    env.push_back(string("REMOTE_IDENT") + "=" + "");
    return env;
}