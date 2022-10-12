#include <unistd.h>
#include <cstdlib>//atoll linux
#include <algorithm>

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "Request.hpp"
#include "uString.hpp"

using namespace std;

Request::Request(int fd, size_t max_size)
    : _fd(fd) {
    _status.status = READING;
    _status.max_size = max_size;
    _content.contentlength = 0;
    _content.readlength = 0;
    _content.postedlength = 0;
    _content.sentlength = 0;
    _content.responselength = 0;
    init();
}

void Request::setError(int error){
    _info.method = INVALID;
    _status.error = error;
    _status.status = COMPLETED;
    return;
}

void Request::init() {
    deque< string > lines(readRequest());
    if (lines.size() == 0)
        return setError(400);
    parseStart(lines);
    if (_info.method == INVALID)
        return;
    parseHeaders(lines);
    _status.status = POSTING;
#ifdef DEBUG
    cout << "\nMethod: " << toSmethod(_info.method) << '\n';
    cout << "URI: " << _info.uri << '\n';
    cout << "Protocol: " << _info.protocol << '\n';
    cout << "Host: " << _info.host << "\n\n";
    cout << "Headers:\n";
    for (deque< string >::const_iterator it = _content.headers.begin();
        it != _content.headers.end(); ++it)
        cout << *it << '\n';
    cout << "\nContent length: " << _content.contentlength << '\n';
    cout << "Message length: " << _content.message.length() << '\n';
    cout << "Content types: ";
    for (set< string>::const_iterator it = _content.types.begin(); it != _content.types.end(); ++it)
        cout << *it << " - ";
    cout << "\nBoundary: " << _content.boundary << '\n';
    cout << "\nMessage:\n";
    cout << _content.message << '\n'
         << endl;
    cout << "\n\n";
#endif
}

// Reads request, splits to deque by CRLF, splits to message
deque< string > Request::readRequest() {
    string buffer;
    buffer.resize(BUFFER_SIZE);
    ssize_t bytes_read = read(_fd, ( void * ) buffer.data(), BUFFER_SIZE);
    cout << "Bytes read: " << bytes_read;
    if (bytes_read <= 0)
        return deque<string>();
    buffer.resize(bytes_read);
    size_t msgstart = buffer.find("\r\n\r\n");
    if (msgstart != string::npos && msgstart + 4 < buffer.size())
        _content.message = buffer.substr(msgstart + 4);
    _content.readlength += _content.message.length();
    return split(buffer.substr(0, msgstart), "\r\n", true);
}


// Splits first line of request to method/URI/protocol
void Request::parseStart(deque< string > &lines) {
    deque< string > firstline(split(lines.front(), " "));
    if (firstline.size() != 3)
        return setError(400);
    _info.method = toEmethod(firstline[0]);
    if (_info.method == INVALID)
        return setError(405);
    _info.uri = firstline[1];
    if (_info.uri.length() > 255)
        return setError(414);
    _info.protocol = firstline[2];
    if (_info.protocol != "HTTP/1.1")
        return setError(505);
    lines.pop_front();
}

// Saves variables line by line
void Request::parseHeaders(deque< string > &lines) {
    bool haslength = false;
    while (lines.size() != 0 && lines.front().size() != 0) {
        deque< string > current(split(lines.front(), ": "));
        if (current.size() != 2)
            return setError(400);
        _content.headers.push_back(lines.front());
        if (current[0] == "Host")
            _info.host = split(current[1], ":")[0];
        else if (current[0] == "Content-Length"){
            _content.contentlength = atoll(current[1].data());
            if (_content.contentlength > _status.max_size)
                return setError(413);
            haslength = true;
        }
        else if (current[0] == "Content-Type"){
            deque< string > segment = split(current[1], ";", true);
            for (size_t i = 0; i < segment.size(); ++i){
                string::iterator it(remove(segment[i].begin(), segment[i].end(), ' '));
                if (it != segment[i].end())
                    segment[i].erase(it);
                if (segment[i].compare(0, 9, "boundary=") == 0)
                    _content.boundary = segment[i].substr(9);
                else
                    _content.types.insert(segment[i]);
            }
        }
        else if (current[0] == "Transfer-Encoding"){        //UNTESTED
            deque< string > segment = split(current[1], ",", true);
            for (size_t i = 0; i < segment.size(); ++i){
                string::iterator it(remove(segment[i].begin(), segment[i].end(), ' '));
                if (it != segment[i].end())
                    segment[i].erase(it);
                _content.encodings.push_back(segment[i]);
            }
        }
        lines.pop_front();
    }
    cout << "\n\nINFO HOST: " << _info.host << "\n\n";
    cout << "INFO HOST LEN: " << _info.host.length() << std::endl;
    if (_info.host.length() == 0)
        return setError(400);
    for (size_t i = 0; i < _content.encodings.size(); ++i)
        if (_content.encodings[i] != "chunked")
            return setError(415);
    if (_info.method == POST && haslength == false && _content.encodings.size() == 0)
        return setError(411);
}

void Request::readMessage() {
    _content.message.resize(BUFFER_SIZE);
    ssize_t bytes_read = read(_fd, ( void * ) _content.message.data(), BUFFER_SIZE);
    cout << "Bytes read: " << bytes_read;
    if (bytes_read < 0)
        return;
    _content.message.resize(bytes_read);
    _content.readlength += bytes_read;
    _status.status = POSTING;
// #ifdef DEBUG
//     cout << "\nRead message:\n" << _content.message << "\n\n";
// #endif 
}

void Request::sendResponse() {
    cout << "SENT RESPONSE\n";
    size_t bytes_wrote = write(_fd, _response.data(), _response.length());
    if (bytes_wrote <= 0)
        return;
    _content.sentlength += bytes_wrote;
    if (_content.sentlength < _content.responselength)
        _response = _response.substr(bytes_wrote);
    else {
        cout << "COMPLETED RESPONSE\n";
        _status.status = COMPLETED;
    }
}

void Request::generateResponse(const string &status, const string &type, const string& message, const vector<string>& headers) {
    cout << "GENERATED RESPONSE\n";
    stringstream ss;
    ss << "HTTP/1.1" << ' ' << status << "\r\n"
       << "Content-Type: " << type << "\r\n"
       << "Content-Length: " << message.length() << "\r\n";
    for (size_t i = 0; i < headers.size(); ++i)
        ss << headers[i] << "\r\n";
    ss << "\r\n" << message;
    _response = ss.str();
    _content.responselength = _response.length();
    _status.status = RESPONDING;
}
