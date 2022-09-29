#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <string>

#include "Request.hpp"

#define HTTP "HTTP/1.1"

class Response
{
    public:
        Response(int fd, const std::string& status, const std::string& type, const std::string& message)
            : _fd(fd), _status(status), _type(type), _message(message) {}

        int send() const {
#ifdef DEBUG
            std::cout << "\n\nRESPONSE:\n\n";
            std::cout << "Message size: " << _message.size() << std::endl;
            std::cout << "Sending response:\n";
            std::cout << HTTP << ' ' << _status << '\n' << "Content-Type: " << _type << '\n'
                    << "Content-Length: " << _message.size() << "\n\n" << _message << std::endl;
#endif
            std::stringstream ss;
            ss << HTTP << ' ' << _status << '\n' << "Content-Type: " << _type << '\n'
                    << "Content-Length: " << _message.size() << "\n\n" << _message;
            std::string response(ss.str());
            return write(_fd, response.c_str(), response.length());
        }


    private:
        int         _fd;
        std::string _status;
        std::string _type;
        std::string _message;

};