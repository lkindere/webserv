#include "Request.hpp"

#include <cstdlib>//atoll linux

using namespace std;

//Basically ft_split but in c++
static deque< string > split(const string &str, const string &delim) {
    deque< string > split;
    size_t start = 0;
    size_t end = str.find(delim);
    while (end != str.npos) {
        split.push_back(str.substr(start, end - start));
        start = end + delim.length();
        end = str.find(delim, start);
    }
    split.push_back(str.substr(start));
    return split;
}

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
    parseVariables(lines);
    parseMessage(lines);
#ifdef DEBUG
    cout << "Method: " << _method << '\n';
    cout << "URI: " << _uri << '\n';
    cout << "Protocol: " << _protocol << '\n';
    cout << "Host: " << _host << '\n'
         << '\n';
    cout << "Obtained variables:\n";
    for (map< string, string >::const_iterator it = _variables.begin(); it != _variables.end(); ++it)
        cout << it->first << ' ' << it->second << '\n';
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
    for (map< string, string >::const_iterator it = _variables.begin();
         it != _variables.end(); ++it) {
        ss << it->first << ": " << it->second << '\n';
    }
    stream << ss.rdbuf() << '\n'
           << _message;
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
void Request::parseVariables(deque< string > &lines) {
    while (lines.size() != 0 && lines.front().size() != 0) {
        deque< string > current(split(lines.front(), ": "));
        if (current.size() != 2) {
            _method = INVALID;
            return;
        }
        _variables.insert(make_pair(current[0], current[1]));
        if (current[0] == "Host")
            _host = split(current[1], ":")[0];
        lines.pop_front();
    }
    while (lines.size() == 0 && lines.front().size() == 0)
        lines.pop_front();
}

// Parses message based on Content Length if there is one and the length is specified
void Request::parseMessage(deque< string > &lines) {
    if (_method == GET || _method == INVALID)
        return;
    map< string, string >::const_iterator it(_variables.find("Content-Length"));
    if (it == _variables.end())
        return;
    size_t content_length = atoll(it->second.c_str());
    while (lines.size() != 0 && _message.length() + lines.front().length() <= content_length) {
        _message.append(lines.front());
        lines.pop_front();
    }
    if (lines.size() != 0 && _message.length() < content_length)
        _message.append(lines.front(), content_length - _message.length());
#ifdef DEBUG
    cout << "\n\nEXPECTED LENGTH: " << content_length << "\n\n";
    cout << "\n\nFINAL MESSAGE LENGTH: " << _message.length() << "\n\n";
#endif
}
