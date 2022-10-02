#include "ConfigParser.hpp"

#include <fstream>
#include <deque>
#include <set>

// #ifdef DEBUG
# include <iostream>
// #endif

#define CLIENT_BODY_SIZE_MAX 1073741824 //1GB
#define CONFIG_FILE_SIZE_MAX 100000

using namespace std;

enum line_status
{
    VALID,
    PARSEABLE,
    INVALID
};

enum e_token
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

struct Token
{
    e_token type;
    string  value;

    Token(e_token type, const string& value) : type(type), value(value) {}
};

struct Line
{
    deque<Token>  tokens;
    int           index;

    Line(const deque<Token>& tokens, size_t index) : tokens(tokens), index(index) {}
};

/**
 * @brief Because initializer list constructors are c++11
 * @return vector<string> pairs [index] to e_token except for VALUE
 */
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

/**
 * @brief Splits lines by delim
 * @param str input str
 * @param delim delimiter
 * @return deque<string> split lines
 */
static deque<string> split(const string& str, const string& delim) {
    deque<string> split;
    size_t start = 0;
    size_t end = str.find(delim);
    size_t index = 1;
    while (end != str.npos) {
        split.push_back(str.substr(start, end - start));
        start = end + delim.length();
        end = str.find(delim, start);
    }
    split.push_back(str.substr(start));
    return split;
}

/**
 * @brief Converts single like to deque<Token>
 * @param line
 * @return deque<Token> 
 */
static deque<Token> tokenize(const string& line){
    deque<Token> tokens;
    vector<string> tstr(tokenStr());
    deque<string> segments(split(line, " "));
    for (size_t j = 0; j < segments.size(); ++j){
        if (segments[j].length() == 0)
            continue;
        for (size_t k = 0; k < tstr.size() + 1; ++k){
            if (k == tstr.size())
                tokens.push_back(Token(VALUE, segments[j]));
            else if (segments[j] == tstr[k]){
                tokens.push_back(Token((e_token)k, segments[j]));
                break;
            }
        }
    }
    return tokens;
}

/**
 * @brief atoll but accepts multipliers: K/KB, M/MB, G/GB
 * @param str 
 * @return long long 
 */
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

/**
 * @brief Saves lines
 * @param conf ConfigData to update
 * @param msg message to use
 * @return int always 1
 */
static int setError(ConfigData& conf, int index, const string& msg){
    conf.status.error_line = index;
    conf.status.error_msg = msg;
    return 1;
}

static string noSemicolon(const std::string& str){
    if (str.length() != 0 && str.back() == ';')
        return str.substr(0, str.length() - 1);
    return str;
}

static int addLocation(deque<Line>& lines, ServerConfig& server, ConfigData& conf){
    LocationConfig location;
    location.uri = lines[0].tokens[1].value;
    lines.pop_front();
#ifdef DEBUG
    cerr << "\n===============LOCATION:===============";
    for (deque<Line>::const_iterator it = lines.begin(); it != lines.end(); ++it){
        cerr << "\nLine: " << it->index << std::endl;
        for (deque<Token>::const_iterator tkn = it->tokens.begin(); tkn != it->tokens.end(); ++tkn)
            cerr << "Token type: " << tkn->type << " value: " << tkn->value << std::endl;
    }
#endif
    while (lines.size() != 0){
        deque<Token>& tokens = lines[0].tokens;
        switch (tokens[0].type){
            case ROOT:{
                if (tokens.size() != 2)
                    return setError(conf, lines[0].index, "Invalid format");
                location.root = noSemicolon(tokens[1].value);
                lines.pop_front();
                break;
            }
            default:
                return setError(conf, lines[0].index, "Invalid token");
        }
    }
    server.locations.push_back(location);
    return 0;
}

static int validBlockStart(const Line& line){
    if (line.tokens.size() < 2)
        return 0;
    if (line.tokens.size() == 2){
        if (line.tokens[0].type == SERVER && line.tokens[1].type == BR_OPEN)
            return 1;
        return 0;
    }
    if (line.tokens.size() == 3){
        if (line.tokens[0].type == LOCATION && line.tokens[1].type == VALUE
            && line.tokens[2].type == BR_OPEN)
            return 1;
        return 0;
    }
    return 0;
}

static int validBlockEnd(const Line& line){
    if (line.tokens.size() == 1 && line.tokens[0].type == BR_CLOSE)
        return 1;
    return 0;
}

static int getSegment(deque<Line>& lines, deque<Line>& segment, ConfigData& conf){
    if (validBlockStart == 0)
        return setError(conf, lines[0].index, "Syntax error");
    size_t i = 0;
    int braces_open = 1;
    int braces_closed = 0;
    while (++i < lines.size()){
        if (validBlockEnd(lines[i])){
            ++braces_closed;
            if (braces_open == braces_closed){
                segment.assign(lines.begin(), lines.begin() + i);
                lines.erase(lines.begin(), lines.begin() + i + 1);
                return 0;
            }
        }
        if (validBlockStart(lines[i]))
            ++braces_open;
    }
    return setError(conf, lines[0].index, "Unclosed braces");
}

static int addServer(deque<Line>& lines, ConfigData& conf){
    lines.pop_front();
#ifdef DEBUG
    cerr << "\n===============SERVER:===============";
    for (deque<Line>::const_iterator it = lines.begin(); it != lines.end(); ++it){
        cerr << "\nLine: " << it->index << std::endl;
        for (deque<Token>::const_iterator tkn = it->tokens.begin(); tkn != it->tokens.end(); ++tkn)
            cerr << "Token type: " << tkn->type << " value: " << tkn->value << std::endl;
    }
#endif
    ServerConfig server;
    while (lines.size() != 0){
        deque<Token>& tokens = lines[0].tokens;
        switch (tokens[0].type){
            case ROOT:{
                if (tokens.size() != 2)
                    return setError(conf, lines[0].index, "Invalid format");
                server.root = noSemicolon(tokens[1].value);
                lines.pop_front();
                break;
            }
            case HOST:{
                if (tokens.size() != 2)
                    return setError(conf, lines[0].index, "Invalid format");
                server.host = noSemicolon(tokens[1].value);
                lines.pop_front();
                break;
            }
            case LISTEN:{
                if (tokens.size() != 2)
                    return setError(conf, lines[0].index, "Invalid format");
                server.port = atoll(tokens[1].value.data());
                lines.pop_front();
                break;
            }
            case LOCATION:{
                deque<Line> segment;
                if (getSegment(lines, segment, conf) != 0)
                    return 1;
                if (addLocation(segment, server, conf) != 0)
                    return setError(conf, lines[0].index, "Syntax error");
                break;
            }
            default:
                return setError(conf, lines[0].index, "Invalid token");
        }
    }
    conf.servers.push_back(server);
    return 0;
}

static int addGlobal(Line& line, ConfigData& conf){
    if (line.tokens.size() < 2)
        return setError(conf, line.index, "Invalid format");
    line.tokens.back().value = noSemicolon(line.tokens.back().value);
    if (line.tokens[0].value == "client_max_body_size"){
        if (line.tokens.size() != 2)
            return setError(conf, line.index, "Invalid format");
        long long number = multitoll(line.tokens[1].value);
        if (number == 0)
            return setError(conf, line.index, "Numeric value above 0 is required");
        if (number == -1)
            return setError(conf, line.index, "Invalid multiplier format");
        if (number > CLIENT_BODY_SIZE_MAX)
            return setError(conf, line.index, "Value too high");
        conf.global.client_max_body_size = number;
        return 0;
    }
    if (line.tokens[0].value == "error_root"){
        if (line.tokens.size() != 2)
            return setError(conf, line.index, "Invalid format");
        conf.global.error_root = line.tokens[1].value;
        return 0;
    }
    if (line.tokens[0].value == "error_page"){
        if (line.tokens.size() < 3)
            return setError(conf, line.index, "Invalid format");
        for (size_t i = 1; i < line.tokens.size() - 1; ++i){
            long long errorcode = atoll(line.tokens[i].value.data());
            if (errorcode < 100 || errorcode > 599)
                return setError(conf, line.index, "Invalid error code");
            conf.global.error_pages.insert(make_pair(errorcode, line.tokens.back().value));
        }
        return 0;
    }
    return setError(conf, line.index, "Unknown parameter");
}

/**
 * @brief Checks if all tokens are VALUE type
 * @param tokens 
 * @return int 1 true 0 false
 */
static int allValues(const deque<Token>& tokens){
    for (size_t i = 0; i < tokens.size(); ++i)
        if (tokens[i].type != VALUE)
            return 0;
    return 1;
}

static int parseLines(deque<Line>& lines, ConfigData& conf){
    while (lines.size() != 0){
        if (allValues(lines[0].tokens)){
            if (addGlobal(lines[0], conf) != 0)
                return 1;
            lines.pop_front();
        }
        else if (lines[0].tokens[0].type == SERVER){
            deque<Line> segment;
            if (getSegment(lines, segment, conf) != 0)
                return 1;
            if (addServer(segment, conf) != 0)
                return 1;
        }
        else
            return setError(conf, lines[0].index, "Invalid format");
    }
    return 0;
}

static ConfigData invalid(ConfigData& conf){
    ConfigData ret;
    ret.status.success = false;
    ret.status.error_line = conf.status.error_line;
    ret.status.error_msg = conf.status.error_msg;
    return ret;
}

/**
 * @brief Reads from path, splits by newline, outputs to lines (ignores comments and newlines)
 * @param path input path
 * @param lines deque<lines>& to write to
 * @param conf ConfigData& to write to
 * @return int 0 on success, 1 while setting conf status error_msg on error
 */
static int readTokens(const string& path, deque<Line>& lines, ConfigData& conf){
    string tmp;
    tmp.resize(CONFIG_FILE_SIZE_MAX);
    ifstream file(path);
    if (file.is_open() == false)
        return setError(conf, 0, "Config file failed to open");
    file.read(tmp.data(), CONFIG_FILE_SIZE_MAX);
    ssize_t bytes_read = file.gcount();
    if (bytes_read <= 0)
        return setError(conf, 0, "Error while reading config file");
    if (bytes_read == CONFIG_FILE_SIZE_MAX && file.eof() == false)
        return setError(conf, 0, "Config file too large");
    deque<string> strs(split(tmp.substr(0, bytes_read), "\n"));
    for (size_t i = 0; i < strs.size(); ++i){
        if (strs[i].length() == 0 || strs[i][0] == '#')
            continue;
        lines.push_back(Line(tokenize(strs[i]), i + 1));
    }
    return 0;
}

#ifdef DEBUG
    void printConfig(const ConfigData& conf){
        cerr << "\n\n";
        cerr << "\n===============CONFIG STATUS:===============\n";
        cerr << "Success:    " << conf.status.success << '\n';
        cerr << "Error line: " << conf.status.error_line << '\n';
        cerr << "Error msg:  " << conf.status.error_msg << '\n';
        cerr << "\n==================GLOBAL:===================\n";
        cerr << "client_max_body_size: " << conf.global.client_max_body_size << '\n';
        cerr << "error_root: " << conf.global.error_root << '\n';
        cerr << "error_pages:\n";
        for (map<short, string>::const_iterator it = conf.global.error_pages.begin();
            it != conf.global.error_pages.end(); ++it)
                cerr << it->first << " : " << it->second << '\n';
        cerr << "\n==================SERVERS:==================\n";
        for (vector<ServerConfig>::const_iterator it = conf.servers.begin();
            it != conf.servers.end(); ++it){
            cerr << "\n==========SERVER:==========\n";
            cerr << "Host: " << it->host << '\n';
            cerr << "Port: " << it->port << '\n';
            cerr << "Root: " << it->root << '\n';
            cerr << "\n=======LOCATIONS:======\n";
            for (vector<LocationConfig>::const_iterator loc = it->locations.begin();
            loc != it->locations.end(); ++loc){
                cerr << "\n====LOCATION:====\n";
                cerr << "URI:  " << loc->uri << '\n';
                cerr << "Root: " << loc->root << '\n';
            }
        }
        cerr << "\n\n";
    }
#endif

ConfigData parseConfig(const string& path){
    ConfigData conf;
    deque<Line> lines;
    conf.status.error_line = 0;
    if (readTokens(path, lines, conf) != 0)
        return invalid(conf);
    if (parseLines(lines, conf) != 0)
        return invalid(conf);
    //Add root routing if none on location check server > if none on server check default > if no default error 
    //Add more documentation I guess and split to files
    conf.status.success = true;
    conf.status.error_line = 0;
#ifdef DEBUG
    printConfig(conf);
#endif
    return conf;
}
