#pragma once

#include <deque>
#include <string>

std::deque< std::string > split(const std::string &str, const std::string &delim, bool noempty = false);

size_t getMatches(std::deque< std::string > &path, std::deque< std::string > &uri);

#include <sstream>

template < typename T >
std::string ToString(const T &v) {
    std::ostringstream ss;
    ss << v;
    return ss.str();
}

template < typename T >
T FromString(const std::string &str) {
    std::istringstream ss(str);
    T ret;
    ss >> ret;
    return ret;
}