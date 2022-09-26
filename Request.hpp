#pragma once

#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#define BUFFER_SIZE 50000

enum e_methods
{
    GET,
    POST,
    DELETE,
    INVALID
};

class Request
{
    public:
        Request(int fd);
    
    public:
        int fd() const { return _fd; }
        e_methods method() const { return _method; }
        const std::string& URI() const { return _URI; }
        const std::string& protocol() const { return _protocol; }
        const std::vector<std::pair<std::string, std::string> >& variables() { return _variables; }
        const std::string& message() const { return _message; }

        void printRequest(std::ostream& stream) const;

    private:
        void readRequest();
        std::string parseStart(const std::string& input);
        std::string parseVariables(const std::string& input);
        std::string parseMessage(const std::string& input);

        int getMethod(const std::string& input);
        int getURI(const std::string& input, int index);
        int getProtocol(const std::string& input, int index);
        std::string nextLine(const std::string& input) const;


    private:
        int                                                 _fd;
        std::string                                         _buffer;
        e_methods                                           _method;
        std::string                                         _URI;
        std::string                                         _protocol;
        std::vector<std::pair<std::string, std::string> >   _variables;
        std::string                                         _message;

};
