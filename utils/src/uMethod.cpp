#include <algorithm>

#include "uMethod.hpp"

using namespace std;

/**
 * @brief Converts str to e_method
 * @param method: STR 
 * @return e_method
 */
e_method toEmethod(const string &method) {
    if (method == "GET")
        return GET;
    if (method == "POST")
        return POST;
    if (method == "DELETE")
        return DELETE;
    return INVALID;
}

/**
 * @brief Converts e_method to str
 * @param method: e_method
 * @return str
 */
string toSmethod(e_method method) {
    if (method == GET)
        return "GET";
    if (method == POST)
        return "POST";
    if (method == DELETE)
        return "DELETE";
    return "INVALID";
}

/**
 * @brief Check if method is valid
 * @param allowed: vector of allowed methods, if empty all are allowed
 * @param method: method to test
 * @return int 1 on valid 0 on invalid
 */
int validMethod(const vector< e_method > &allowed, e_method method) {
    if (allowed.size() != 0
        && find(allowed.begin(), allowed.end(), method) == allowed.end())
        return 0;
    return 1;
}
