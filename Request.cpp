#include "Request.hpp"

//Constructor runs all the other bs
Request::Request(int fd)
    : _fd(fd) {
    readRequest();
    std::string remains = parseStart(_buffer);
    if (_method == INVALID){
        //Obviously not the correct way to handle it
        std::cout << "INVALID METHOD\n";
        return;
    }
    remains = parseVariables(remains);
    _message = parseMessage(remains);
#ifdef DEBUG
    std::cout << "Method: " << _method << std::endl;
    std::cout << "URI: " << _URI << std::endl;
    std::cout << "Protocol: " << _protocol << '\n' << std::endl;
    std::cout << "Obtained variables:\n";
    for (std::vector<std::pair<std::string, std::string> >::iterator it = _variables.begin(); it != _variables.end(); ++it)
        std::cout << it->first << ' ' << it->second << std::endl;
    std::cout << "\nMessage:\n";
    std::cout << _message << '\n' <<  std::endl;
#endif
}

void Request::readRequest(){
    _buffer.resize(BUFFER_SIZE);
    int bytes_read = read(_fd, (void*)_buffer.data(), BUFFER_SIZE);
}

void Request::printRequest(std::ostream& stream) const {
    stream << "REQUEST:\n\n";
    std::stringstream ss;
    switch(_method){
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
    for (std::vector<std::pair<std::string, std::string> >::const_iterator it = _variables.begin();
        it != _variables.end(); ++it){
        ss << it->first << ": " << it->second << '\n';
    }
    stream << ss.rdbuf() << '\n' << _message;
    // stream.write(_message.data(), _message.length());
}

//Parses the first line of the request
std::string Request::parseStart(const std::string& input){
    int index = getMethod(input);
    if (index == -1)
        return std::string();
    index = getURI(input, index);
    index = getProtocol(input, index);
    return std::string(&input[index]);
}

//Parses variables into a vector (std::map would be better for searching, but vector saves the ordering same way as received)
std::string Request::parseVariables(const std::string& input){
    std::string CRLF;
    CRLF.resize(2);
    CRLF[0] = 13;
    CRLF[1] = 10;
    std::string remains(input);
    while (remains.size() != 0){
        if (remains.compare(0, 2, CRLF) == 0)
            return std::string(&remains[2]);
        size_t space = remains.find(' ', 0);
        if (space == std::string::npos)
            return std::string();
        size_t end = remains.find(CRLF, space);
        if (end == std::string::npos)
            return std::string();
        std::string var(remains.substr(0, space++));
        std::string value(remains.substr(space, end - space));
        _variables.push_back(std::make_pair(var, value));
        remains = nextLine(remains);
    }
    return std::string();
}

//Parses message based on Content Length if there is one and the length is specified
std::string Request::parseMessage(const std::string& input){
    // if (input.size() == 0){
    //     std::cout << "\n\nEMPTY MSG RET\n\n";
    //     return std::string();
    // }
    std::vector<std::pair<std::string, std::string> >::iterator it = _variables.begin();
    for (; it != _variables.end(); ++it){
        if (it->first == "Content-Length:")
            break;
    }
    if (it == _variables.end()){
        //No content length specified, not reading further
        return std::string();
    }
    size_t length = atoll(it->second.c_str());
    std::string message(input.substr(0, length));
    while (message.length() < length){
        length -= message.length();
        int bytes_read = read(_fd, (void*)_buffer.data(), BUFFER_SIZE);
        if (bytes_read <= 0)
            break;
        message += _buffer.substr(0, length);
    }
#ifdef DEBUG
    std::cout << "\n\nEXPECTED LENGTH: " << length << "\n\n";
    std::cout << "\n\nFINAL MESSAGE LENGTH: " << message.length() << "\n\n";
#endif
    _buffer.clear();
    return message;
}

//Skips to the next CRLF line
std::string Request::nextLine(const std::string& input) const {
    std::string CRLF;
    CRLF.resize(2);
    CRLF[0] = 13;
    CRLF[1] = 10;
    size_t index = input.find(CRLF, 0);
    if (index == std::string::npos)
        return std::string();
    return std::string(&input[index + 2]);
}

//Gets first line method
int Request::getMethod(const std::string& input){
    if (input.compare(0, 4, "GET ") == 0){
        _method = GET;
        return 4;
    }
    else if (input.compare(0, 5, "POST ") == 0){
        _method = POST;
        return 5;
    }
    else if (input.compare(0, 7, "DELETE ") == 0){
        _method = DELETE;
        return 7;
    }
    _method = INVALID;
    return -1;
}

//Gets first line URI
int Request::getURI(const std::string& input, int index){
    size_t separator = input.find(' ', index);
    if (separator == std::string::npos){
        _method = INVALID;
        return -1;
    }
    _URI = input.substr(index, separator - index);
    return ++separator;
}

//Gets first line protocol
int Request::getProtocol(const std::string& input, int index){
    size_t separator = input.find('\n', index);
    if (separator == std::string::npos){
        _method = INVALID;
        return -1;
    }
    _protocol = input.substr(index, separator - index);
    return ++separator;
}
