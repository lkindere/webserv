#include "Request.hpp"

//Basically ft_split but in c++
static std::deque<std::string> split(const std::string& str, const std::string& delim) {
    std::deque<std::string> split;
    size_t start = 0;
    size_t end = str.find(delim);
    while (end != str.npos) {
        split.push_back(str.substr(start, end - start));
        start = end + delim.length();
        end = str.find(delim, start);
    }
    for (std::deque<std::string>::iterator it = split.begin(); it != split.end(); ++it)
        std::cout << *it << std::endl;
    split.push_back(str.substr(start));
    return split;
}

//Str to e_method
static e_methods toEnum(const std::string& method) {
    if (method == "GET")
        return GET;
    if (method == "POST")
        return POST;
    if (method == "DELETE")
        return DELETE;
    return INVALID;
}

// Constructor runs all the other bs
// Reads request splits lines by CRLF to lines deque
// Each further call to parseSomething modifies &lines removing line by line
Request::Request(int fd)
    : _fd(fd) {
    std::deque<std::string> lines(readRequest());
    parseStart(lines);
    if (_method == INVALID)
        return;
    parseVariables(lines);
    parseMessage(lines);
#ifdef DEBUG
    std::cout << "Method: " << _method << std::endl;
    std::cout << "URI: " << _URI << std::endl;
    std::cout << "Protocol: " << _protocol << '\n' << std::endl;
    std::cout << "Obtained variables:\n";
    for (std::map<std::string, std::string>::const_iterator it = _variables.begin(); it != _variables.end(); ++it)
        std::cout << it->first << ' ' << it->second << std::endl;
    std::cout << "\nMessage:\n";
    std::cout << _message << '\n' <<  std::endl;
#endif
}

// Reads request, splits to deque by CRLF
std::deque<std::string> Request::readRequest() {
    std::string buffer;
    buffer.resize(BUFFER_SIZE);
    int bytes_read = read(_fd, (void*)buffer.data(), BUFFER_SIZE);
    if (bytes_read == BUFFER_SIZE)
        throw(std::runtime_error("NEED A BIGGER BUFFER OR SMTH\n"));
    buffer.resize(bytes_read);
    return split(buffer, "\r\n");
}

//Debug junk
void Request::printRequest(std::ostream& stream) const {
    stream << "REQUEST:\n\n";
    std::stringstream ss;
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
    ss << _URI << ' ' << _protocol << '\n';
    for (std::map<std::string, std::string>::const_iterator it = _variables.begin();
        it != _variables.end(); ++it) {
        ss << it->first << ": " << it->second << '\n';
    }
    stream << ss.rdbuf() << '\n' << _message;
}

// Splits first line of request to method/URI/protocol
void Request::parseStart(std::deque<std::string>& lines) {
    std::deque<std::string> firstline(split(lines.front(), " "));
    if (firstline.size() != 3){
        _method = INVALID;
        return;
    }
    _method = toEnum(firstline.at(0));
    _URI = firstline.at(1);
    _protocol = firstline.at(2);
    lines.pop_front();
}

// Saves variables line by line
void Request::parseVariables(std::deque<std::string>& lines) {
    while (lines.size() != 0 && lines.front().size() != 0){
        std::deque<std::string> current(split(lines.front(), ": "));
        if (current.size() != 2){
            _method = INVALID;
            return;
        }
        _variables.insert(std::make_pair(current.at(0), current.at(1)));
        std::cout << "\nLine: " << lines.front() << std::endl;;
        lines.pop_front();
    }
    while (lines.size() == 0 && lines.front().size() == 0)
        lines.pop_front();
}

// Parses message based on Content Length if there is one and the length is specified
void Request::parseMessage(std::deque<std::string>& lines) {
    if (_method == GET || _method == INVALID)
        return;
    std::map<std::string, std::string>::const_iterator it(_variables.find("Content-Length"));
    if (it == _variables.end())
        return;
    size_t content_length = atoll(it->second.c_str());
    while (lines.size() != 0 && _message.length() + lines.front().length() <= content_length){
        _message.append(lines.front());
        lines.pop_front();
    }
    if (lines.size() != 0 && _message.length() < content_length)
        _message.append(lines.front(), content_length - _message.length());
#ifdef DEBUG
    std::cout << "\n\nEXPECTED LENGTH: " << content_length << "\n\n";
    std::cout << "\n\nFINAL MESSAGE LENGTH: " << _message.length() << "\n\n";
#endif
}
