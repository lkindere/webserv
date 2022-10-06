#include <unistd.h>
#include <cstdlib>//atoll linux
#include <algorithm>

#include <iostream>
#include <sstream>

#include "Request.hpp"
#include "uString.hpp"

using namespace std;

void Request::setError(int error){
    _method = INVALID;
    _error = error;
    return;
}

// Constructor runs all the other bs
// Reads request splits lines by CRLF to lines deque
// Each further call to parseSomething modifies &lines removing line by line
Request::Request(int fd, size_t size_max)
    : _fd(fd), _size_max(size_max) {
    _content.length = (size_t)-1;
    init();
}

void Request::init() {
    deque< string > lines(readRequest());
    parseStart(lines);
    if (_method == INVALID)
        return setError(400);
    parseHeaders(lines);
    parseMessage(lines);
#ifdef DEBUG
    cout << "\nMethod: " << toSmethod(_method) << '\n';
    cout << "URI: " << _uri << '\n';
    cout << "Protocol: " << _protocol << '\n';
    cout << "Host: " << _host << "\n\n";
    cout << "Headers:\n";
    for (deque< string >::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
        cout << *it << '\n';
    cout << "\nContent length: " << _content.length << '\n';
    cout << "Content types: ";
    for (set< string>::const_iterator it = _content.types.begin(); it != _content.types.end(); ++it)
        cout << *it << " - ";
    cout << "\nBoundary: " << _content.boundary << '\n';
    cout << "\nMessage:\n";
    cout << _message << '\n'
         << endl;
    cout << "\n\n";
#endif
}

// Reads request, splits to deque by CRLF
deque< string > Request::readRequest() {
    string buffer;
    buffer.resize(BUFFER_SIZE);
    ssize_t bytes_total = 0;
    ssize_t bytes_read = read(_fd, ( void * ) buffer.data(), BUFFER_SIZE);
    while (bytes_read == BUFFER_SIZE){
        throw("Does this even happen?");
        bytes_total += bytes_read;
        buffer.resize(buffer.size() * 2);
        bytes_read = read(_fd, ( void * ) &buffer[bytes_total], BUFFER_SIZE);
    }
    if (bytes_read > 0)
        bytes_total += bytes_read;
    buffer.resize(bytes_total);
    return split(buffer, "\r\n");
}

// Splits first line of request to method/URI/protocol
void Request::parseStart(deque< string > &lines) {
    deque< string > firstline(split(lines.front(), " "));
    if (firstline.size() != 3)
        return setError(400);
    _method = toEmethod(firstline[0]);
    _uri = firstline[1];
    _protocol = firstline[2];
    lines.pop_front();
}

// Saves variables line by line
void Request::parseHeaders(deque< string > &lines) {
    bool haslength = false;
    while (lines.size() != 0 && lines.front().size() != 0) {
        deque< string > current(split(lines.front(), ": "));
        if (current.size() != 2)
            return setError(400);
        _headers.push_back(lines.front());
        if (current[0] == "Host")
            _host = split(current[1], ":")[0];
        if (current[0] == "Content-Length"){
            _content.length = atoll(current[1].data());
            haslength = true;
        }
        if (current[0] == "Content-Type"){
            deque< string > segment = split(current[1], ";", true);
            for (size_t i = 0; i < segment.size(); ++i){
                segment[i].erase(remove(segment[i].begin(), segment[i].end(), ' '));
                if (segment[i].compare(0, 9, "boundary=") == 0)
                    _content.boundary = segment[i].substr(9);
                else
                    _content.types.insert(segment[i]);
            }
        }
        lines.pop_front();
    }
    if (_method == POST && haslength == false) // && !CHUNKED later
        return setError(411);
}

// Parses message based on Content Length if there is one and the length is specified
void Request::parseMessage(deque< string > &lines) {
    //Add check for client size body size max later
    //Need to store an error code somewhere though on invalid
    //Or remake constructor to only take fd then init() with int ret
    if (_method != POST || _content.length == 0)
        return;
    if (_content.length > _size_max)
        return setError(413);
    while (lines.size() != 0 && _message.length() + lines.front().length() <= _content.length) {
        _message.append(lines.front());
        lines.pop_front();
    }
    if (lines.size() != 0 && _message.length() < _content.length)
        _message.append(lines.front(), _content.length - _message.length());
#ifdef DEBUG
    cout << "\nEXPECTED LENGTH: " << _content.length << '\n';
    cout << "FINAL MESSAGE LENGTH: " << _content.length << "\n\n";
#endif
}
