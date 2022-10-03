#include <deque>

#include "ConfigParser.hpp"

using namespace std;

/**
 * @brief Converts str to e_method
 * @param method: STR 
 * @return e_method
 */
e_method toEmethod(const std::string& method){
    if (method == "GET")
        return GET;
    if (method == "POST")
        return POST;
    if (method == "DELETE")
        return DELETE;
    return INVALID;
}

/**
 * @brief Parses config file
 * @param path config file path
 * @return ConfigData (check ret.status before use)
 */
ConfigData parseConfig(const string& path){
    ConfigData conf;
    deque<Line> lines;
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
