#pragma once

#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <deque>
#include <vector>
#include <deque>
#include <map>

#include "Config.hpp"

#define BUFFER_SIZE 65536

enum e_method;

class Request
{
    public:
        Request(int fd);
    
    public:
        int fd() const { return _fd; }
        e_method method() const { return _method; }
        const std::string& URI() const { return _URI; }
        const std::string& protocol() const { return _protocol; }
        const std::map<std::string, std::string>& variables() { return _variables; }
        const std::string& message() const { return _message; }
        const std::string& host() const { return _host; }

        void printRequest(std::ostream& stream) const;

    private:
        std::deque<std::string> readRequest();
        void parseStart(std::deque<std::string>& lines);
        void parseVariables(std::deque<std::string>& lines);
        void parseMessage(std::deque<std::string>& lines);

    private:
        int                                     _fd;
        e_method                                _method;
        std::string                             _URI;
        std::string                             _protocol;
        std::string                             _host;
        std::map<std::string, std::string>      _variables;
        std::string                             _message;
};
