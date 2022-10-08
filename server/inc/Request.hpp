#pragma once

#include <set>
#include <deque>
#include <string>

#include "Config.hpp"

#define BUFFER_SIZE 65536

enum e_rstat
{
    READING,
    WRITING,
    COMPLETED
};

struct reqStatus
{
    int error;
    e_rstat status;
    size_t max_size;
};

struct reqInfo
{
    e_method method;
    std::string uri;
    std::string protocol;
    std::string host;
};

struct reqContent
{
    size_t contentlength;
    size_t readlength;
    size_t writtenlength;
    std::string boundary;
    std::set< std::string> types;
    std::string message;
    std::deque< std::string > headers;
};

class Request {
public:
    Request(int fd, size_t size_max);
    void setStatus(e_rstat status) { _status.status = status; }

public:
    int                 fd() const { return _fd; }
    e_rstat             status() const { return _status.status; }
    int                 error() const { return _status.error; }

    e_method            method() const { return _info.method; }
    const std::string&  uri() const { return _info.uri; }
    const std::string&  protocol() const { return _info.protocol; }
    const std::string&  host() const { return _info.host; }

    size_t              contentlength() const { return _content.contentlength; }
    size_t              readlength() const { return _content.readlength; }
    size_t              writtenlength() const { return _content.writtenlength; }
    const std::string&  boundary() const { return _content.boundary; }
    const std::string&  message() const { return _content.message; }
    const std::set< std::string > &types() const { return _content.types; }
    const std::deque< std::string> &headers() { return _content.headers; }

    void                setWritten(size_t len) { _content.writtenlength = len; }
    void                readMessage();

private:
    void init();
    std::deque< std::string > readRequest();
    void parseStart(std::deque< std::string > &lines);
    void parseHeaders(std::deque< std::string > &lines);
    void parseMessage(std::deque< std::string > &lines);
    void setError(int error);

private:
    int         _fd;
    reqStatus   _status;
    reqInfo     _info;
    reqContent  _content;
};
