#pragma once

#include <set>
#include <deque>
#include <string>

#include "Config.hpp"

#define BUFFER_SIZE 65536

struct ContentInfo
{
    size_t length;
    std::string boundary;
    std::set< std::string> types;
};

class Request {
public:
    Request(int fd, size_t size_max);

public:
    int fd() const { return _fd; }
    e_method method() const { return _method; }

    const std::string &uri() const { return _uri; }
    const std::string &protocol() const { return _protocol; }
    const std::deque< std::string> &headers() { return _headers; }
    const std::string &message() const { return _message; }
    const std::string &host() const { return _host; }
    const std::set< std::string > &types() const { return _content.types; }
    const std::string& boundary() const { return _content.boundary; }
    size_t length() const { return _content.length; }
    int error() const { return _error; }

    void printRequest(std::ostream &stream) const;

private:
    void init();
    std::deque< std::string > readRequest();
    void parseStart(std::deque< std::string > &lines);
    void parseHeaders(std::deque< std::string > &lines);
    void parseMessage(std::deque< std::string > &lines);
    void setError(int error);

private:
    int _fd;
    int _error;
    size_t _size_max;
    e_method _method;
    std::string _uri;
    std::string _protocol;
    std::string _host;
    ContentInfo _content;
    std::string _message;
    std::deque< std::string > _headers;
};
