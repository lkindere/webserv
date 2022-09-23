#pragma once

#include <iostream>
#include <string>
#include <utility>
#include <vector>

enum e_methods
{
    GET,
    POST,
    DELETE,
    INVALID
};

class Request
{
    public:
        Request(const std::string& input);
    
    private:
        std::string parseStart(const std::string& input);
        std::string parseVariables(const std::string& input);
        std::string parseMessage(const std::string& input);
        
        int getMethod(const std::string& input);
        int getURI(const std::string& input, int index);
        int getProtocol(const std::string& input, int index);
        std::string nextLine(const std::string& input) const;


    private:
        e_methods                                           method;
        std::string                                         URI;
        std::string                                         protocol;
        std::vector<std::pair<std::string, std::string> >   variables;
        std::string                                         message;

};
