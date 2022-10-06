
#include <cstdlib>//atoll linux

#include "Request.hpp"
#include "uString.hpp"

using namespace std;

// Constructor runs all the other bs
// Reads request splits lines by CRLF to lines deque
// Each further call to parseSomething modifies &lines removing line by line
Request::Request(int fd)
    : _fd(fd) {
    if (fd == -1)
        return;
    deque< string > lines(readRequest());
    parseStart(lines);
    if (_method == INVALID)
        return;
    parseHeaders(lines);
    parseMessage(lines);
#ifdef DEBUG
    cout << "\nMethod: " ;
    switch (_method){
        case GET:
            cout << "GET\n";
            break;
        case POST:
            cout << "POST\n";
            break;
        case DELETE:
            cout << "DELETE\n";
            break;
        default:
            cout << "INVALID\n";
    }
    cout << "URI: " << _uri << '\n';
    cout << "Protocol: " << _protocol << '\n';
    cout << "Host: " << _host << "\n\n";
    cout << "Headers:\n";
    for (deque< string >::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
        cout << *it << '\n';
    cout << "\nMessage:\n";
    cout << _message << '\n'
         << endl;
#endif
}

// Reads request, splits to deque by CRLF
deque< string > Request::readRequest() {
    string buffer;
    buffer.resize(BUFFER_SIZE);
    int bytes_read = read(_fd, ( void * ) buffer.data(), BUFFER_SIZE);
    if (bytes_read == BUFFER_SIZE)
        throw(runtime_error("NEED A BIGGER BUFFER OR SMTH\n"));
    buffer.resize(bytes_read);
    return split(buffer, "\r\n");
}

//Debug junk
void Request::printRequest(ostream &stream) const {
    stream << "REQUEST:\n\n";
    stringstream ss;
    switch (_method) {
        case GET:
            ss << "GET ";
            break;
        case POST:
            ss << "POST ";
            break;
        case DELETE:
            ss << "DELETE ";
            break;
        default:
            ss << "INVALID ";
    }
    ss << _uri << ' ' << _protocol << '\n';
    for (deque< string >::const_iterator it = _headers.begin();
         it != _headers.end(); ++it) {
        ss << *it << '\n';
    }
    stream << ss.rdbuf() << '\n' << _message;
}

// Splits first line of request to method/URI/protocol
void Request::parseStart(deque< string > &lines) {
    deque< string > firstline(split(lines.front(), " "));
    if (firstline.size() != 3) {
        _method = INVALID;
        return;
    }
    _method = toEmethod(firstline[0]);
    _uri = firstline[1];
    _protocol = firstline[2];
    lines.pop_front();
}

// Saves variables line by line
void Request::parseHeaders(deque< string > &lines) {
    while (lines.size() != 0 && lines.front().size() != 0) {
        deque< string > current(split(lines.front(), ": "));
        if (current.size() != 2) {
            _method = INVALID;
            return;
        }
        _headers.push_back(lines.front());
        if (current[0] == "Host")
            _host = split(current[1], ":")[0];
        if (current[0] == "Content-Type")
            
        lines.pop_front();
    }
}

// Parses message based on Content Length if there is one and the length is specified
void Request::parseMessage(deque< string > &lines) {
    if (lines.size() != 0)
        ;
//     if (_method != POST)
//         return;
//     map< string, string >::const_iterator it(_variables.find("Content-Length"));
//     if (it == _variables.end())
//         return;
//     _contentlength = atoll(it->second.c_str());
//     it = _variables.find("Content-Type");
//     if (it == _variables.end())
//         return;
//     while (lines.size() != 0 && _message.length() + lines.front().length() <= content_length) {
//         _message.append(lines.front());
//         lines.pop_front();
//     }
//     if (lines.size() != 0 && _message.length() < _contentlength)
//         _message.append(lines.front(), _contentlength - _message.length());
// #ifdef DEBUG
//     cout << "\nEXPECTED LENGTH: " << _contentlength << '\n';
//     cout << "FINAL MESSAGE LENGTH: " << _message.length() << "\n\n";
// #endif
}
