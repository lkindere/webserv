#pragma once

#include <set>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "Request.hpp"
#include "Response.hpp"

class Server
{
    public:
        Server(const char* configpath){
            parseConfig(configpath);
#ifdef DEBUG
            std::cout << "Path: " << _path << std::endl;
            std::cout << "Root: " << _root << std::endl;
            for (std::set<std::string>::iterator it = _pages.begin(); it != _pages.end(); ++it)
                std::cout << "Page: " << *it << std::endl;
            for (std::map<int, std::string>::iterator it = _errors.begin(); it != _errors.end(); ++it)
                std::cout << "Error " << it->first << ": " << it->second << std::endl;
#endif
        }

    public:
        Response sendResponse(const Request& request){
            if (request.method() == GET)
                return getMethod(request);
            if (request.method() == POST)
                return postMethod(request);
            if (request.method() == DELETE)
                return deleteMethod(request);
            return errorPage(request, 400, "400 Bad Request");
        }

    private:

        Response getMethod(const Request& request) const {
            std::cout << "Root: " << _root << std::endl;
            if (request.URI() == "/")
                return Response(request.fd(), "200 OK", getType(_root), getString(_path + _root));
#ifdef DEBUG
            std::cout << "ALL PAGES:\n";
            for (std::set<std::string>::const_iterator it = _pages.begin(); it != _pages.end(); ++it)
                std::cout << *it << std::endl;
            std::cout << "SEARCHING FOR:\n";
            std::cout << request.URI() << std::endl;
#endif
            std::set<std::string>::const_iterator it(std::find(_pages.begin(), _pages.end(), request.URI()));
            if (it == _pages.end())
                return errorPage(request, 404, "404 Not Found");
            return Response(request.fd(), "200 OK", getType(_path + *it), getString(_path + *it));
        }
        
        Response postMethod(const Request& request) const {
            return Response(request.fd(), "0", "0", "0"); //Placeholder
        }

        Response deleteMethod(const Request& request) const {
            return Response(request.fd(), "0", "0", "0"); //Placeholder
        }

        Response errorPage(const Request& request, int errorcode, const char* errormsg) const {
            std::map<int, std::string>::const_iterator it(_errors.find(errorcode));
            if (it == _errors.end())
                return Response(request.fd(), errormsg, "text/plain", errormsg);
            std::cout << "Sending response: " << getString(it->second);
            return Response(request.fd(), errormsg, getType(it->second), getString(_path + it->second));
        }

        //Switch to mime.types thingy like NGINX
        std::string getType(const std::string& path) const {
            if (path.find(".gif", 0) != path.npos)
                return std::string("image/gif");
            if (path.find(".jpg", 0) != path.npos)
                return std::string("image/gif");
            if (path.find(".jpeg", 0) != path.npos)
                return std::string("image/gif");
            if (path.find(".html", 0) != path.npos)
                return std::string("text/html");
            return std::string("text/plain");
        }

        std::string getString(const std::string& path) const {
            std::cout << "\nGETTING STRING FROM: " << path << std::endl;
            std::ifstream file(path);
            std::stringstream ss;
            ss << file.rdbuf();
            std::cout << "STRING: " << ss << std::endl;
            return ss.str();
        }

        int parseConfig(const char* path){
            std::ifstream file(path);
            std::string line;
            while (1){
                std::getline(file, line);
                if (line.size() == 0)
                    break;
                if (line.compare(0, 2, "//") == 0)
                    continue;
                if (line.compare(0, 6, "PATH: ") == 0){
                    _path = &line[6];
                    continue;
                }
                if (line.compare(0, 6, "ROOT: ") == 0){
                    _root = "/" + std::string(&line[6]);
                    continue;
                }
                if (line.compare(0, 7, "PAGES: ") == 0){
                    line = &line[7];
                    while (line.size() != 0){
                        size_t space = line.find(' ', 0);
                        if (space == std::string::npos){
                            _pages.insert("/" + line);
                            break;
                        }
                        _pages.insert("/" + line.substr(0, space));
                        line = &line[++space];
                    }
                }
                if (isnumber(line[0])){
                    int code = atoi(line.data());
                    if (code < 100 || code > 999)
                        return -1;
                    _errors.insert(std::make_pair(code, "/" + std::string(&line[5])));
                    continue;
                }  
            }
            return 0; //Add some kind of error hadling in misconfigurations later?
        }

    private:
        std::string                         _path;
        std::string                         _root;
        std::set<std::string>               _pages;
        std::map<int, std::string>          _errors;
};