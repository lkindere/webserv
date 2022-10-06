#include <deque>
#include <string>

std::deque< std::string > split(const std::string &str, const std::string &delim, bool noempty = false);
size_t getMatches(std::deque< std::string > &path, std::deque< std::string > &uri);