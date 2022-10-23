#include <deque>

#include "ConfigParser.hpp"

using namespace std;

/**
 * @brief Parses config file
 * @param path config file path
 * @return ConfigData (check ret.status before use)
 */
ConfigData parseConfig(const string &path) {
    ConfigData conf;
    deque< Line > lines;
    conf.status.error_line = 0;
    if (readTokens(path, lines, conf) != 0)
        return invalid(conf);
    if (parseLines(lines, conf) != 0)
        return invalid(conf);
    if (checkConfig(conf) != 0)
        return invalid(conf);
    conf.status.success = true;
    conf.status.error_line = 0;
#ifdef DEBUG
    printConfig(conf);
#endif
    return conf;
}
