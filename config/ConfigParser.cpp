#include "ConfigParser.hpp"

#include <fstream>
#include <deque>

#ifdef DEBUG
# include <iostream>
#endif

#define CLIENT_BODY_SIZE_MAX 1073741824 //1GB

using namespace std;

enum line_status
{
    VALID,
    PARSEABLE,
    INVALID
};

static deque<string> split(const string& str, const string& delim) {
    deque<string> split;
    size_t start = 0;
    size_t end = str.find(delim);
    while (end != str.npos) {
        split.push_back(str.substr(start, end - start));
        start = end + delim.length();
        end = str.find(delim, start);
    }
    split.push_back(str.substr(start));
    return split;
}

static line_status checkLine(string& buffer, const string& line){
    static int braces_open;
    static int braces_close;
    if (line.size() == 0 || line[0] == '#')
        return VALID;
    char end = line[line.length() -1];
    if (end == ';'){
        buffer.append(line);
        if (braces_open == 0)
            return PARSEABLE;
        buffer.append("\n");
        return VALID;
    }
    if (end == '{'){
        ++braces_open;
        buffer.append(line + "\n");
        return VALID;
    }
    if (end == '}'){
        buffer.append(line);
        if (++braces_close == braces_open){
            braces_close = 0;
            braces_open = 0;
            return PARSEABLE;
        }
        buffer.append("\n");
        return VALID;
    }
    return INVALID;
}

static int setErrorMsg(ConfigData& conf, const std::string& msg){
    conf.status.error_msg = msg;
    return 1;
}

static int addGlobal(const string& line, ConfigData& conf){
    deque<string> var(split(line, " "));
    var[1].pop_back();
    if (var.size() != 2)
        return setErrorMsg(conf, "Invalid format");
    if (var[0] == "client_max_body_size:"){
    
    //Put all this into a function converting to numeric
        size_t number = atoll(var[1].data());
        if (number == 0)
            return setErrorMsg(conf, "Numeric value above 0 is required");
        size_t i = 0;
        while (i < var[1].length() && isdigit(var[1][i]))
            ++i;
        if (i == var[1].length()){
            if (number > CLIENT_BODY_SIZE_MAX);
                return setErrorMsg(conf, "Value too high");
            conf.global.client_max_body_size = number;
            return 0;
        }
        string sizetype = var[1].substr(i);
        if (sizetype == "K" || sizetype == "KB"){
            if (number * 1024 > CLIENT_BODY_SIZE_MAX);
                return setErrorMsg(conf, "Value too high");
            conf.global.client_max_body_size = number * 1024;
            return 0;
        }
        if (sizetype == "M" || sizetype == "MB"){
            if (number * 1024 * 1024 > CLIENT_BODY_SIZE_MAX)
                return setErrorMsg(conf, "Value too high");
            conf.global.client_max_body_size = number * 1024 * 1024;
            return 0;
        }
        return setErrorMsg(conf, "Invalid format");
    }
    return setErrorMsg(conf, "Unknown parameter");
}

static int parseSegment(string& buffer, ConfigData& conf){
    deque<string> lines(split(buffer, "\n"));
    if (lines.size() == 1)
        return addGlobal(lines.front(), conf);
    return 0;
}

ConfigParser::ConfigParser(const string& path)
    : _path(path) {}

ConfigData ConfigParser::invalid(int line, const string& msg) const{
    ConfigData ret;
    ret.status.success = false;
    ret.status.error_line = line;
    ret.status.error_msg = msg;
    return ret;
}

ConfigData ConfigParser::parse() const{
    ConfigData ret;
    ifstream file(_path);
    if (file.is_open() == false)
        return invalid(0, "Config file failed to open");
    string buffer;
    string line;
    size_t line_index = 0;
    while (getline(file, line) && ++line_index){
        line_status status = checkLine(buffer, line);
#ifdef DEBUG
        cout << "READ LINE\n" << line << '\n';
        if (status == VALID)
            cout << "STATUS VALID\n";
        else if (status == PARSEABLE)
            cout << "STATUS PARSEABLE\n";
        else
            cout << "STATUS INVALID\n";
        cout << "FULL BUFFER\n" << buffer << '\n';
        cout << "\n-------------------------------------------\n";
#endif
        if (status == INVALID)
            return invalid(line_index, "Line ends with invalid token");
        if (status == PARSEABLE){
            if (parseSegment(buffer, ret) != 0)
                return invalid(line_index, ret.status.error_msg);
            buffer.clear();
        }
        line.clear();
    }
    ret.status.success = true;
    ret.status.error_line = 0;
    return ret;
}
