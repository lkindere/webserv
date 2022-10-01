#include "ConfigParser.hpp"

#include <fstream>
#include <deque>
#include <set>

// #ifdef DEBUG
# include <iostream>
// #endif

#define CLIENT_BODY_SIZE_MAX 1073741824 //1GB

using namespace std;

enum line_status
{
    VALID,
    PARSEABLE,
    INVALID
};

enum token
{
    BR_OPEN = 0,
    BR_CLOSE = 1,
    ROOT = 2,
    HOST = 3,
    LISTEN = 4,
    SERVER = 5,
    LOCATION = 6,
    VALUE = 7
};

vector<string> tokenStr(){
    vector<string> tstr;
    tstr.reserve(7);
    tstr.push_back("{");
    tstr.push_back("}");
    tstr.push_back("root");
    tstr.push_back("host");
    tstr.push_back("listen");
    tstr.push_back("server");
    tstr.push_back("location");
    return tstr;
}

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

static deque<pair<token, string> > tokenize(const deque<string>& lines){
    deque<pair<token, string> > tokens;
    vector<string> tstr(tokenStr());
    for (size_t i = 0; i < lines.size(); ++i){
        deque<string> segments(split(lines[i], " "));
        for (size_t j = 0; j < segments.size(); ++j){
            if (segments[j].length() == 0)
                continue;
            for (size_t k = 0; k < tstr.size() + 1; ++k){
                if (k == tstr.size())
                    tokens.push_back(make_pair(VALUE, segments[j]));
                else if (segments[j] == tstr[k]){
                    tokens.push_back(make_pair((token)k, segments[j]));
                    break;
                }
            }
        }
    }
#ifdef DEBUG
    for (deque<pair<token, string> >::iterator it = tokens.begin(); it != tokens.end(); ++it)
        cout << "Token type: " << it->first << " value: " << it->second << endl;
#endif
    return tokens;
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

static long long multitoll(const std::string& str){
    long long number = atoll(str.data());
    if (number == 0)
        return 0;
    size_t i = 0;
    while (i < str.length() && isdigit(str[i]))
        ++i;
    if (i == str.length())
        return number;
    string sizetype = str.substr(i);
    if (sizetype == "K" || sizetype == "KB")
        return number * 1024;
    if (sizetype == "M" || sizetype == "MB")
        return number * 1024 * 1024;
    if (sizetype == "G" || sizetype == "GB")
        return number * 1024 * 1024 * 1024;
    return -1;
}

static int addGlobal(const string& line, ConfigData& conf){
    deque<string> var(split(line, " "));
    if (var.size() < 2)
        return setErrorMsg(conf, "Invalid format");
    var.back().pop_back();
    if (var[0] == "client_max_body_size"){
        if (var.size() != 2)
            return setErrorMsg(conf, "Invalid format");
        long long number = multitoll(var[1]);
        if (number == 0)
            return setErrorMsg(conf, "Numeric value above 0 is required");
        if (number == -1)
            return setErrorMsg(conf, "Invalid multiplier format");
        if (number > CLIENT_BODY_SIZE_MAX)
            return setErrorMsg(conf, "Value too high");
        conf.global.client_max_body_size = number;
        return 0;
    }
    if (var[0] == "error_root"){
        if (var.size() != 2)
            return setErrorMsg(conf, "Invalid format");
        conf.global.error_root = var[1];
        return 0;
    }
    if (var[0] == "error_page"){
        if (var.size() < 3)
            return setErrorMsg(conf, "Invalid format");
        for (size_t i = 1; i < var.size() - 1; ++i){
            long long errorcode = atoll(var[i].data());
            if (errorcode < 100 || errorcode > 599)
                return setErrorMsg(conf, "Invalid error code");
            conf.global.error_pages.insert(make_pair(errorcode, var.back()));
        }
        return 0;
    }
    std::cout << "Parameter: " << var[0] << std::endl;
    return setErrorMsg(conf, "Unknown parameter");
}

static int endSemicolon(const std::string& str){
    if (str.size() == 0 || *str.rbegin() != ';')
        return 0;
    return 1;
}

static int addLocation(deque<pair<token, string> >& tokens, ServerConfig& server){
    cout << "Adding location\n";
    cout << "Tokens received:\n";
    for (deque<pair<token, string> >::iterator it = tokens.begin(); it != tokens.end(); ++it)
        cout << "Token type: " << it->first << " value: " << it->second << endl;
    LocationConfig location;
    string url(tokens[0].second);
    tokens.pop_front();
    tokens.pop_front();
    while (tokens.size() != 0 && tokens[0].first != BR_CLOSE){
         switch (tokens[0].first){
            case ROOT:
                tokens.pop_front();
                if (tokens.size() == 0 || tokens[0].first != VALUE || !endSemicolon(tokens[0].second)){
                    cout << "Error on location token: " << tokens[0].second << std::endl;
                    return 1;
                }
                location.root = tokens[0].second;
                tokens.pop_front();
                break;
            default:
                cout << "Error on location token: " << tokens[0].second << std::endl;
                return 1;
         }
    }
    if (tokens.size() == 0)
        return 1;
    server.locations.insert(make_pair(url, location));
    tokens.pop_front();
    return 0;
}

static int addServer(deque<pair<token, string> >& tokens, ConfigData& conf){
    ServerConfig server;
    if (tokens.size() < 2 || tokens[0].first != SERVER || tokens[1].first != BR_OPEN)
        return setErrorMsg(conf, "Syntax error");
    tokens.pop_front();
    tokens.pop_front();
    while (tokens.size() != 0 && tokens[0].first != BR_CLOSE){
        switch (tokens[0].first){
            case ROOT:
                tokens.pop_front();
                if (tokens.size() == 0 || tokens[0].first != VALUE || !endSemicolon(tokens[0].second)){
                    cout << "Error on token: " << tokens[0].second << std::endl;
                    return setErrorMsg(conf, "Syntax error");
                }
                server.root = tokens[0].second;
                tokens.pop_front();
                break;
            case HOST:
                tokens.pop_front();
                if (tokens.size() == 0 || tokens[0].first != VALUE || !endSemicolon(tokens[0].second)){
                    cout << "Error on token: " << tokens[0].second << std::endl;
                    return setErrorMsg(conf, "Syntax error");
                }
                server.host = tokens[0].second;
                tokens.pop_front();
                break;
            case LISTEN:
                tokens.pop_front();
                if (tokens.size() == 0 || tokens[0].first != VALUE || !endSemicolon(tokens[0].second)){
                    cout << "Error on token: " << tokens[0].second << std::endl;
                    return setErrorMsg(conf, "Syntax error");
                }
                server.port = atoll(tokens[0].second.data());
                tokens.pop_front();
                break;
            case LOCATION:
                tokens.pop_front();
                if (tokens.size() < 2 || tokens[0].first != VALUE || tokens[1].first != BR_OPEN){
                    cout << "Error on token: " << tokens[0].second << std::endl;
                    return setErrorMsg(conf, "Syntax error");
                }
                if (addLocation(tokens, server) != 0){
                    cout << "Error on token: " << tokens[0].second << std::endl;
                    return setErrorMsg(conf, "Syntax error");
                }
                break;
            default:
                cout << "Error on token: " << tokens[0].second << std::endl;
                return setErrorMsg(conf, "Invalid token");
        }
    }
    return 0;
}

static int parseSegment(string& buffer, ConfigData& conf){
    deque<string> lines(split(buffer, "\n"));
    if (lines.size() == 1)
        return addGlobal(lines[0], conf);
    deque<pair<token, string> > tokens(tokenize(lines));
    if (tokens[0].first == SERVER)
        return addServer(tokens, conf);
    return 1;
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
    while (getline(file, line) && ++ret.status.error_line){
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
            return invalid(ret.status.error_line, "Line ends with invalid token");
        if (status == PARSEABLE){
            if (parseSegment(buffer, ret) != 0)
                return invalid(ret.status.error_line, ret.status.error_msg);
            buffer.clear();
        }
        line.clear();
    }
    ret.status.success = true;
    ret.status.error_line = 0;
    return ret;
}
