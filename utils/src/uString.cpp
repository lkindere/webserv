
#include "uString.hpp"

using namespace std;

/**
 * @brief Splits lines by delim
 * @param str input str
 * @param delim delimiter
 * @param noempty don't return empty strings
 * @return deque<string> split lines
 */
deque< string > split(const string &str, const string &delim, bool noempty) {
    deque< string > split;
    size_t start = 0;
    size_t end = str.find(delim);
    while (end != str.npos) {
        string segment(str.substr(start, end - start));
        if (noempty == false || segment.empty() == false)
            split.push_back(segment);
        start = end + delim.length();
        end = str.find(delim, start);
    }
    string segment(str.substr(start));
    if (noempty == false || segment.empty() == false)
        split.push_back(segment);
    return split;
}

/**
 * @brief Count number of equal strings from the start to the end
 * @param path deque<str>
 * @param uri deque<str>
 * @return size_t number of matches or (size_t)-1 on perfect match (all strings match)
 */
size_t getMatches(deque< string > &path, deque< string > &uri) {
    size_t matches = 0;
    for (size_t i = 0; i < path.size() && i < uri.size(); ++i) {
        if (path[i] == uri[i])
            ++matches;
        else
            break;
    }
    if (matches == path.size() && matches == uri.size())
        return -1;
    return matches;
}
