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
    _info.authentication = 0;
    _status.status = READING;
    _status.headers_parsed = false;
    _status.content_unchunked = false;
    _status.max_size = max_size;
    _content.contentlength = 0;
    _content.readlength = 0;
    _content.postedlength = 0;
    _content.sentlength = 0;
    _content.responselength = 0;
    readRequest();
}

void Request::setError(int error){
    _info.method = INVALID;
    _status.error = error;
}

void Request::init(const string& input) {
    deque<string> lines = split(input, "\r\n", true);
    if (lines.size() == 0)
        return setError(400);
    parseStart(lines);
    if (_info.method == INVALID)
        return;
    parseHeaders(lines);
#ifdef DEBUG
    cout << "\nMethod: " << toSmethod(_info.method) << '\n';
    cout << "URI: " << _info.uri << '\n';
    cout << "Protocol: " << _info.protocol << '\n';
    cout << "Host: " << _info.host << "\n\n";
    cout << "Headers:\n";
    for (map< string, string >::const_iterator it = _content.headers.begin();
        it != _content.headers.end(); ++it)
        cout << it->first << " , " << it->second << '\n';
    cout << "Cookies:\n";
    for (vector<pair<string, string> >::const_iterator it = _content.cookies.begin();
        it != _content.cookies.end(); ++it)
        cout << "Cookie: " << it->first << " val: " << it->second << '\n';
    cout << "\nContent length: " << _content.contentlength << '\n';
    cout << "Message length: " << _content.message.length() << '\n';
    cout << "Content types: ";
    for (set< string>::const_iterator it = _content.types.begin(); it != _content.types.end(); ++it)
        cout << *it << " - ";
    cout << "\nBoundary: " << _content.boundary << '\n';
#endif
}

void Request::readRequest() {
    if (_status.headers_parsed == false) {
        string buffer(BUFFER_SIZE, 0);
        ssize_t bytes_read = read(_fd, (void*)buffer.data(), BUFFER_SIZE);
        if (bytes_read <= 0)
            return;
        _content.message.append(buffer, 0, bytes_read);
        size_t msgstart = _content.message.find("\r\n\r\n");
        if (msgstart == string::npos)
            return;
        init(_content.message.substr(0, msgstart));
        _content.message = _content.message.substr(msgstart + 4);
        _content.readlength = _content.message.length();
        _status.headers_parsed = true;
    }
    else {
        if (_content.encoding == "chunked"){    //Almost untested
            string buffer(BUFFER_SIZE, 0);
            ssize_t bytes_read = read(_fd, (void*) buffer.data(), BUFFER_SIZE);
            if (bytes_read <= 0)
                return;
            _content.contentlength = 0;
            while (buffer.length() != 0 && _status.content_unchunked == false) {
                size_t i = buffer.find("\r\n");
                if (i == string::npos)
                    break;
                size_t chunklen = 0;
                istringstream(buffer.substr(0, i)) >> hex >> chunklen;
                if (chunklen == 0)
                    _status.content_unchunked = true;
                _content.message.append(buffer, i + 2, chunklen);
                i = buffer.find("\r\n", i + 2 + chunklen);
                if (i == string::npos)
                    break;
                buffer = buffer.substr(i + 2);
            }
        }
        else {
            _content.message.resize(BUFFER_SIZE);
            ssize_t bytes_read = read(_fd, ( void * ) _content.message.data(), BUFFER_SIZE);
            if (bytes_read <= 0)
                return;
            _content.message.resize(bytes_read);
            _content.readlength += bytes_read;
        }
    }
    if (_info.method != POST)
        _status.status = POSTING;
    else if (_info.method == DELETE)
        _status.status = POSTING; //I guess could be more than this
    else if (_info.method == POST){
        if ((_content.contentlength != 0 && _content.readlength >= _content.contentlength)
            || _status.content_unchunked == true
            || _content.types.find("multipart/form-data") !=  _content.types.end())
                _status.status = POSTING;
    }
// #ifdef DEBUG
//     cout << "\n\nMessage:\n";
//     cout << _content.message << "\n\n" << endl;
// #endif
}

// Splits first line of request to method/URI/protocol
void Request::parseStart(deque< string > &lines) {
    deque< string > firstline(split(lines.front(), " "));
    if (firstline.size() != 3)
        return setError(400);
    _info.method = toEmethod(firstline[0]);
    if (_info.method == INVALID)
        return setError(405);
    size_t qstart = firstline[1].find('&');
    if (qstart != string::npos){
        _info.query = decode_special(firstline[1].substr(qstart + 1));
        _info.uri = decode_special(firstline[1].substr(0, qstart));
    }
    else
        _info.uri = decode_special(firstline[1]);
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
        else if (current[0] == "Transfer-Encoding"){
            deque< string > segment = split(current[1], ",", true);
            for (size_t i = 0; i < segment.size(); ++i){
                string::iterator it(remove(segment[i].begin(), segment[i].end(), ' '));
                if (it != segment[i].end())
                    segment[i].erase(it);
                if (segment[i] != "chunked")
                    return setError(415);
                _content.encoding = segment[i];
            }
        }
        else if (current[0] == "Cookie") {
            deque<string> keyval = split(current[1], "=", true);
            if (keyval.size() == 2)
                _content.cookies.push_back(make_pair(keyval[0], keyval[1]));
        }
        else
            addHeader(current);
        lines.pop_front();
    }
    if (_info.host.length() == 0)
        return setError(400);
    if (_info.method == POST && haslength == false && _content.encoding != "chunked")
        return setError(411);
}

void Request::addHeader(const deque<string>& line) {
    if (_content.headers[line[0]].length() == 0)
        _content.headers[line[0]] = line[1];
    else
        _content.headers[line[0]].append(", " + line[1]);
}


void Request::sendResponse() {
    size_t bytes_wrote = write(_fd, _response.data(), _response.length());
    if (bytes_wrote <= 0)
        return;
    _content.sentlength += bytes_wrote;
    if (_content.sentlength < _content.responselength)
        _response = _response.substr(bytes_wrote);
    else 
        _status.status = COMPLETED;
}

void Request::generateResponse(const string &status, const string &type, const string& message, const vector<string>& headers) {
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

string Request::getHeader(const string& header) const {
    map<string, string>::const_iterator it = headers().find(header);
    if (it != headers().end())
        return it->second;
    return string();
}