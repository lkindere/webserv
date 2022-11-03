#pragma once

#include <deque>
#include <string>

std::deque< std::string > split(const std::string &str, const std::string &delim, bool noempty = false);
std::pair<size_t, size_t> getMatches(std::deque< std::string > &path, std::deque< std::string > &uri);
std::string itostr(long long n);
char percentToAscii(const std::string& prcnt);
std::string decode_special(const std::string& input);