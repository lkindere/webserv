#pragma once

#include <set>
#include <map>
#include <deque>
#include <string>

#include "uMethod.hpp"

#define BUFFER_SIZE 65536

enum e_rstat
{
    READING,
    POSTING,
    RESPONDING,
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
    std::string query;
    std::string protocol;
    std::string host;
};

struct reqContent
{
    size_t contentlength;
    size_t readlength;
    size_t postedlength;
    size_t sentlength;
    size_t responselength;
    std::string boundary;
    std::set< std::string> types;
    std::vector<std::string> encodings;
    std::string message;
    std::map<std::string, std::string>  headers;
};

struct reqForms
{
    std::map<std::string, std::string> forms;
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
    const std::string&  query() const { return _info.query; }
    const std::string&  protocol() const { return _info.protocol; }
    const std::string&  host() const { return _info.host; }
    size_t              contentlength() const { return _content.contentlength; }
    size_t              readlength() const { return _content.readlength; }
    size_t              postedlength() const { return _content.postedlength; }
    size_t              sentlength() const { return _content.sentlength; }
    size_t              responselength() const { return _content.responselength; }
    const std::string&  boundary() const { return _content.boundary; }
    const std::string&  message() const { return _content.message; }
    const std::set< std::string > &types() const { return _content.types; }
    const std::map<std::string, std::string>& headers() const { return _content.headers; }
    const std::map<std::string, std::string>& forms() const { return _forms.forms; }
   
    std::string         getHeader(const std::string& header) const;
    void                setPosted(size_t len) { _content.postedlength = len; }

    void                readMessage();
    void                sendResponse();
    void                generateResponse(const std::string &status, const std::string &type, 
        const std::string& message, const std::vector<std::string>& headers = std::vector<std::string>());
   

private:
    void init();
    std::deque< std::string > readRequest();
    void parseStart(std::deque< std::string > &lines);
    void parseHeaders(std::deque< std::string > &lines);
    void parseMessage(std::deque< std::string > &lines);
    void setError(int error);
    void addHeader(const std::deque<std::string>& line);

private:
    int         _fd;
    reqStatus   _status;
    reqInfo     _info;
    reqContent  _content;
    reqForms    _forms;
    std::string _response;
};
