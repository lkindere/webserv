#pragma once

//Only used for actual parsing code no need to include anywhere outside of Config.cpp

#include <string>
#include <deque>

#include "Config.hpp"

enum e_token
{
    BR_OPEN = 0,
    BR_CLOSE = 1,
    ROOT = 2,
    HOST = 3,
    LISTEN = 4,
    SERVER = 5,
    LOCATION = 6,
    UPLOADS = 7,
    METHODS = 8,
    SERVER_NAME = 9,
    CGI_EXTENSIONS = 10,
    REDIRECT = 11,
    INDEX = 12,
    AUTO_INDEX = 13,
    VALUE = 14
};

struct Token
{
    e_token         type;
    std::string     value;

    Token(e_token type, const std::string& value) : type(type), value(value) {}
};

struct Line
{
    std::deque<Token>   tokens;
    int                 index;

    Line(const std::deque<Token>& tokens, size_t index) : tokens(tokens), index(index) {}
};


int         readTokens(const std::string& path, std::deque<Line>& lines, ConfigData& conf);
int         parseLines(std::deque<Line>& lines, ConfigData& conf);
int         checkConfig(ConfigData& conf);
ConfigData  invalid(ConfigData& conf);

#ifdef DEBUG
    void printConfig(const ConfigData& conf);  
#endif