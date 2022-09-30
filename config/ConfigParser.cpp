#include "ConfigParser.hpp"

#include <fstream>

#ifdef DEBUG
# include <iostream>
#endif

using namespace std;

enum line_status
{
    VALID,
    PARSEABLE,
    INVALID
};

line_status checkLine(string& buffer, const string& line){
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

ConfigParser::ConfigParser(const std::string& path)
    : _path(path) {}

ConfigData ConfigParser::invalid(int line, const std::string& msg) const{
    ConfigData ret;
    ret.status.error_line = line;
    ret.status.error_msg = msg;
    return ret;
}


ConfigData ConfigParser::parse() const{
    ifstream file(_path);
    if (file.is_open() == false)
        return invalid(-1, "Config file failed to open");
    string buffer;
    string line;
    size_t line_index = 0;
    while (getline(file, line)){
        line_status status = checkLine(buffer, line);
#ifdef DEBUG
        cout << "READ LINE:\n" << line << '\n';
        if (status == VALID)
            cout << "STATUS: VALID\n";
        else if (status == PARSEABLE)
            cout << "STATUS: PARSEABLE\n";
        else
            cout << "STATUS: INVALID\n";
        cout << "FULL BUFFER:\n" << buffer << '\n';
        cout << "\n-------------------------------------------\n";
#endif
        if (status == INVALID)
            return invalid(line_index, "Line ends with invalid token");
        if (status == PARSEABLE)
            buffer.clear();
            // parseline &buffer
        line.clear();
        ++line_index;
    }


    
    ConfigData ret;
    ret.status.success = true;
    return ret;
}
