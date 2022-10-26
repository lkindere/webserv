#include "Sessions.hpp"
#include "uServer.hpp"

using namespace std;

/**
 * @brief Generates a random cookie and returns it, returns empty string if no such user
 * @return const string& 
 */
string Sessions::generateCookie(const string& username) {
    const User* user = getUser(username);
    if (user == NULL)
        return string();
    string cookie = generateRandom(100);
    while (_cookies.find(cookie) != _cookies.end())
        cookie = generateRandom(100);
    return _cookies.insert(make_pair(cookie, username)).first->first;
}

#include <iostream>

/**
 * @brief Returns cookie level if valid 0 if not
 * @param cookie 
 * @return true 
 * @return false 
 */
int Sessions::validCookie(const string& key, const string& value) const {
    if (key != "PotatoServUSER")
        return 0;
    map<string, string>::const_iterator it = _cookies.find(value);
    if (it != _cookies.end()) {
        const User* user = getUser(it->second);
        if (user == NULL)
            return 0;
        return user->level();
    }
    return 0;
}

/**
 * @brief Returns 1 if removed 0 if not
 * @param cookie 
 * @return true 
 * @return false 
 */
bool Sessions::removeCookie(const string& value) {
    return _cookies.erase(value) == 1;
}

/**
 * @brief Adds a new user, true on success, false if already exists
 * @param user 
 * @param pass 
 * @return true 
 * @return false 
 */
bool Sessions::addUser(const std::string& user, const std::string& pass, int level) {
    pair<set<User>::iterator, bool> ret = _users.insert(User(user, pass, level));
    return ret.second;
}

/**
 * @brief Returns a user based on username, or NULL if not found
 * @param user 
 * @return const User* 
 */
const User* Sessions::getUser(const std::string& user) const {
    set<User>::const_iterator it = _users.find(user);
    if (it == _users.end())
        return NULL;
    return &(*it);
}

const User* Sessions::getUserByCookie(const std::string& cookie) const {
    map<string, string>::const_iterator it = _cookies.find(cookie);
    if (it == _cookies.end())
        return NULL;
    return getUser(it->second);
}