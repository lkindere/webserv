#include <deque>
#include <string>
#include <utility>

std::deque< std::string > split(const std::string &str, const std::string &delim, bool noempty = false);
std::pair<size_t, size_t> getMatches(std::deque< std::string > &d1, std::deque< std::string > &d2);