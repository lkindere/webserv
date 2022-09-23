#include "Request.hpp"

//Constructor runs all the other bs
Request::Request(const std::string& input){
    std::string remains = parseStart(input);
    if (method == INVALID){
        //Obviously not the correct way to handle it
        std::cout << "INVALID METHOD\n";
        return;
    }
    remains = parseVariables(remains);
    message = parseMessage(remains);


    std::cout << "Method: " << method << std::endl;
    std::cout << "URI: " << URI << std::endl;
    std::cout << "Protocol: " << protocol << '\n' << std::endl;
    std::cout << "Obtained variables:\n";
    for (std::vector<std::pair<std::string, std::string> >::iterator it = variables.begin(); it != variables.end(); ++it)
        std::cout << it->first << ' ' << it->second << std::endl;
    std::cout << "\nMessage:\n";
    std::cout << message << '\n' <<  std::endl;
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
        variables.push_back(std::make_pair(var, value));
        remains = nextLine(remains);
        std::cout << var << std::endl;
        std::cout << value << std::endl;
    }
    return std::string();
}

//Parses message based on Content Length if there is one and the length is specified
std::string Request::parseMessage(const std::string& input){
    if (input.size() == 0)
        return std::string();
    std::vector<std::pair<std::string, std::string> >::iterator it = variables.begin();
    for (; it != variables.end(); ++it){
        if (it->first == "Content-Length:")
            break;
    }
    if (it == variables.end())
        return std::string();
    size_t length = atoll(it->second.c_str());
    return std::string(input.substr(0, length));
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
        method = GET;
        return 4;
    }
    else if (input.compare(0, 5, "POST ") == 0){
        method = POST;
        return 5;
    }
    else if (input.compare(0, 7, "DELETE ") == 0){
        method = DELETE;
        return 7;
    }
    method = INVALID;
    return -1;
}

//Gets first line URI
int Request::getURI(const std::string& input, int index){
    size_t separator = input.find(' ', index);
    if (separator == std::string::npos){
        method = INVALID;
        return -1;
    }
    URI = input.substr(index, separator - index);
    return ++separator;
}

//Gets first line protocol
int Request::getProtocol(const std::string& input, int index){
    size_t separator = input.find('\n', index);
    if (separator == std::string::npos){
        method = INVALID;
        return -1;
    }
    protocol = input.substr(index, separator - index);
    return ++separator;
}
