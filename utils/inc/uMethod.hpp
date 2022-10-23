#pragma once

#include <string>
#include <vector>

/**
 * @brief Defines the HTTP method
 * @param methods: GET, POST, DELETE, INVALID
 */
enum e_method {
    GET,
    POST,
    DELETE,
    INVALID
};

e_method toEmethod(const std::string &method);
std::string toSmethod(e_method method);

int validMethod(const std::vector< e_method > &allowed, e_method method);