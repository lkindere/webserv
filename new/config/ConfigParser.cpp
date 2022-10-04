#include "Config.hpp"
#include "ConfigParser.hpp"

#include <fstream>
#include <deque>
#include <set>

#ifdef DEBUG
# include <iostream>
#endif

#define CLIENT_BODY_SIZE_MAX 1073741824 //1GB
#define CONFIG_FILE_SIZE_MAX 100000

using namespace std;

/**
 * @brief Because initializer list constructors are c++11
 * @return vector<string> pairs [index] to e_token except for VALUE
 */
static vector<string> tokenStr(){
    vector<string> tstr;
    tstr.reserve(12);
    tstr.push_back("{");
    tstr.push_back("}");
    tstr.push_back("root");
    tstr.push_back("host");
    tstr.push_back("listen");
    tstr.push_back("server");
    tstr.push_back("location");
    tstr.push_back("uploads");
    tstr.push_back("methods");
    tstr.push_back("server_name");
    tstr.push_back("cgi_extensions");
    tstr.push_back("redirect");
    tstr.push_back("index");
    tstr.push_back("autoindex");
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

/**
 * @brief Returns invalid ConfigData
 * @param conf ConfigData to use for error_line and error_msg
 * @return ConfigData 
 */
ConfigData invalid(ConfigData& conf){
    ConfigData ret;
    ret.status.success = false;
    ret.status.error_line = conf.status.error_line;
    ret.status.error_msg = conf.status.error_msg;
    return ret;
}

/**
 * @brief Removes a semicolon from the end if there is one
 */
static string noSemicolon(const std::string& str){
    if (str.length() != 0 && str.back() == ';')
        return str.substr(0, str.length() - 1);
    return str;
}

/**
 * @brief Checks for valid block start for SERVER/LOCATION
 * @param line a line
 * @return int 1 on valid 0 on invalid
 */
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

/**
 * @brief Checks for valid block end
 * @param line a line
 * @return int 1 on valid 0 on invalid
 */
static int validBlockEnd(const Line& line){
    if (line.tokens.size() == 1 && line.tokens[0].type == BR_CLOSE)
        return 1;
    return 0;
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

/**
 * @brief Splits part of lines cutting it off into segment
 * @param lines all lines input
 * @param segment segment block
 * @param conf 
 * @return int 1 on error 0 on success
 */
static int getSegment(deque<Line>& lines, deque<Line>& segment, ConfigData& conf){
    if (validBlockStart(lines[0]) == 0)
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
 * @brief Adds a location block
 * @param lines pre-split location segment
 * @param server server struct to add to
 * @param conf 
 * @return int 1 on error 0 on success
 */
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
            case REDIRECT:{
                if (tokens.size() != 2)
                    return setError(conf, lines[0].index, "Invalid format");
                location.redirect = noSemicolon(tokens[1].value);
                lines.pop_front();
                break;
            }
            case UPLOADS:{
                if (tokens.size() != 2)
                    return setError(conf, lines[0].index, "Invalid format");
                location.uploads = noSemicolon(tokens[1].value);
                lines.pop_front();
                break;
            }
            case INDEX:{
                if (tokens.size() != 2)
                    return setError(conf, lines[0].index, "Invalid format");
                location.index = noSemicolon(tokens[1].value);
                lines.pop_front();
                break;
            }
            case AUTO_INDEX:{
                if (tokens.size() != 2)
                    return setError(conf, lines[0].index, "Invalid format");
                if (noSemicolon(tokens[1].value) == "on")
                    location.autoindex = true;
                else
                    return setError(conf, lines[0].index, "Invalid value");
                lines.pop_front();
                break;
            }
            case METHODS:{
                if (tokens.size() < 2)
                    return setError(conf, lines[0].index, "Invalid format");
                for (size_t i = 1; i < tokens.size(); ++i){
                    e_method method = toEmethod(noSemicolon(tokens[i].value));
                    if (method == INVALID)
                        return setError(conf, lines[0].index, "Invalid method");
                    if (find(location.methods.begin(), location.methods.end(), method)
                        != location.methods.end())
                        return setError(conf, lines[0].index, "Multiple definitions of same method");
                    location.methods.push_back(method);
                }
                lines.pop_front();
                break;
            }
            case CGI_EXTENSIONS:{
                if (tokens.size() < 2)
                    return setError(conf, lines[0].index, "Invalid format");
                for (size_t i = 1; i < tokens.size(); ++i){
                    if (find(location.cgi_extensions.begin(), location.cgi_extensions.end(),
                        noSemicolon(tokens[i].value)) != location.cgi_extensions.end())
                        return setError(conf, lines[0].index, "Multiple definitions of same extension");
                    location.cgi_extensions.push_back(noSemicolon(tokens[i].value));
                }
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

/**
 * @brief Adds a server block
 * @param lines pre-split server segment
 * @param conf conf block to add to
 * @return int 1 on error 0 on success
 */
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
            case SERVER_NAME:{
                if (tokens.size() < 2)
                    return setError(conf, lines[0].index, "Invalid format");
                for (size_t i = 1; i < tokens.size(); ++i)
                    server.server_names.push_back(noSemicolon(tokens[i].value));
                lines.pop_front();
                break;
            }
            case LOCATION:{
                deque<Line> segment;
                if (getSegment(lines, segment, conf) != 0)
                    return 1;
                if (addLocation(segment, server, conf) != 0)
                    return 1;
                break;
            }
            default:
                return setError(conf, lines[0].index, "Invalid token");
        }
    }
    conf.servers.push_back(server);
    return 0;
}

/**
 * @brief Adds a global value
 * @param lines single line
 * @param conf conf to add to
 * @return int 1 on error 0 on success
 */
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
 * @brief Checks line by line adding global/server blocks
 * @param lines all lines
 * @param conf 
 * @return int 1 on error 0 on success
 */
int parseLines(deque<Line>& lines, ConfigData& conf){
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

/**
 * @brief Reads from path, splits by newline, outputs to lines (ignores comments and newlines)
 * @param path input path
 * @param lines deque<lines>& to write to
 * @param conf ConfigData& to write to
 * @return int 0 on success, 1 while setting conf status error_msg on error
 */
int readTokens(const string& path, deque<Line>& lines, ConfigData& conf){
    string tmp;
    tmp.resize(CONFIG_FILE_SIZE_MAX);
    ifstream file(path);
    if (file.is_open() == false)
        return setError(conf, 0, "Config file failed to open");
    file.read(&tmp[0], CONFIG_FILE_SIZE_MAX);
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

int checkConfig(ConfigData& conf){
    for (size_t i = 0; i < conf.servers.size(); ++i){
        ServerConfig& server = conf.servers[i];
        if (server.host.length() == 0) 
            return setError(conf, 0, "Server host not set");
        if (server.port == 0)
            return setError(conf, 0, "Server port not set");
        if (server.root.length() == 0)
            return setError(conf, 0, "Server root not set");
        if (server.locations.size() == 0)
            return setError(conf, 0, "At least one location per server required");
        for (size_t j = 0; j < server.locations.size(); ++j){
            if (server.locations[j].uri.length() == 0)
                return setError(conf, 0, "Location URI not set");
            if (server.locations[j].root.empty()
                && server.locations[j].redirect.empty()){
                if (server.root.empty() == false){
                    server.locations[j].root = server.root;
                    continue;
                }
                return setError(conf, 0, "Root not set");
            }
        }
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
            cerr << "Server names: ";
            for (vector<string>::const_iterator nm = it->server_names.begin();
                nm != it->server_names.end(); ++nm){
                cerr << *nm << ' ';
            }
            cerr << '\n';
            cerr << "\n=======LOCATIONS:======\n";
            for (vector<LocationConfig>::const_iterator loc = it->locations.begin();
                loc != it->locations.end(); ++loc){
                cerr << "\n====LOCATION:====\n";
                cerr << "URI:       " << loc->uri << '\n';
                cerr << "Root:      " << loc->root << '\n';
                cerr << "Index:     " << loc->index << '\n';
                cerr << "Autoindex: " << loc->autoindex << '\n';
                cerr << "Redirect:  " << loc->redirect << '\n';
                cerr << "Uploads:   " << loc->uploads << '\n';
                cerr << "Methods:   ";
                for (size_t i = 0; i < loc->methods.size(); ++i){
                    switch (loc->methods[i]){
                        case GET:
                            cerr << "GET ";
                            break;
                        case POST:
                            cerr << "POST ";
                            break;
                        default:
                            cerr << "DELETE ";
                    }
                }
                cerr << "\nCgi extensions: ";
                for (size_t i = 0; i < loc->cgi_extensions.size(); ++i)
                    cerr << loc->cgi_extensions[i] << ' ';
                cerr << '\n';
            }
        }
        cerr << "\n\n";
    }
#endif