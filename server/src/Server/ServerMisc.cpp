#include <unistd.h>
#include <sys/stat.h>
#include <sstream>

// #ifdef DEBUG
    #include <iostream>
// #endif

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
