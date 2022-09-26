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
        Response(int fd, const std::string& status, const std::string& type, const std::string& message){
            std::stringstream ss;
#ifdef DEBUG
            std::cout << "Message size: " << message.size() << std::endl;
            std::cout << "Sending response:\n";
            std::cout << HTTP << ' ' << status << '\n' << "Content-Type: " << type << '\n'
                    << "Content-Length: " << message.size() << "\n\n" << message << std::endl;

#endif
            ss << HTTP << ' ' << status << '\n' << "Content-Type: " << type << '\n'
                    << "Content-Length: " << message.size() << "\n\n" << message;
            std::string response(ss.str());
            write(fd, response.c_str(), response.length());
        }


    private:
        // std::string pageStr(const char* path) const {
        //     std::ifstream page(path);
        //     std::stringstream ss;
        //     ss << page.rdbuf();
        //     return ss.str();
        // }

        // void badRequest() const {
        //     ;
        // }

        // void notFound() const {
        //     send_response(_fd, pageStr("pages/404.html"));
        // }

        // void index() const {
        //     send_response(_fd, pageStr("pages/index.html"));
        // }

        // void nyan() const {
        //     send_response(_fd, pageStr("pages/nyan.gif"), "image/gif");
        // }


    private:
        std::string _status;
        std::string _type;
        std::string _message;

};