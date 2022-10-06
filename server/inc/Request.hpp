#pragma once

#include <unistd.h>

#include <deque>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "Config.hpp"

#define BUFFER_SIZE 65536

enum e_method;

struct ContentInfo
{
    ssize_t length;
    std::vector< std::string> type;
};

class Request {
public:
    Request(int fd);

public:
    int fd() const { return _fd; }
    e_method method() const { return _method; }

    const std::string &uri() const { return _uri; }
    const std::string &protocol() const { return _protocol; }
    const std::deque< std::string> &headers() { return _headers; }
    const std::string &message() const { return _message; }
    const std::string &host() const { return _host; }
    const std::vector< std::string > &type() const { return _content.type; }
    ssize_t length() { return _content.length; }

    void printRequest(std::ostream &stream) const;

private:
    std::deque< std::string > readRequest();
    void parseStart(std::deque< std::string > &lines);
    void parseHeaders(std::deque< std::string > &lines);
    void parseMessage(std::deque< std::string > &lines);

private:
    int _fd;
    e_method _method;
    std::string _uri;
    std::string _protocol;
    std::string _host;
    ContentInfo _content;
    std::string _message;
    std::deque< std::string > _headers;
};
