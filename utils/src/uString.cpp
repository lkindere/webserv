
#include <sstream>

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
 * @brief Count number of equal strings from the start to the end, everything past last match is mismatch
 * @param d1 deque<str>
 * @param d2 deque<str>
 * @return pair<matches, mismatches>
 */
pair<size_t, size_t> getMatches(deque< string > &d1, deque< string > &d2) {
    size_t matches = 0;
    size_t mismatches = 0;
    for (size_t i = 0; i < d1.size() && i < d2.size(); ++i) {
        if (d1[i] == d2[i])
            ++matches;
        else {
            mismatches = max(d1.size(), d2.size()) - (i + 1);
            return make_pair(matches, mismatches);
        }
    }
    return make_pair(matches, max(d1.size(), d2.size()) - min(d1.size(), d2.size()));
}

/**
 * @brief long long to string
 */
string itostr(long long n){
    stringstream ss;
    ss << n;
    return ss.str();
}
