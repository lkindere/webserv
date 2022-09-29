#pragma once

#include <set>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "Structs.hpp"
#include "Request.hpp"
#include "Response.hpp"

#include "CGIthingy.hpp"

class Server
{
    public:
        Server(const char* configpath){
            parseConfig(configpath);
#ifdef DEBUG
            std::cout << "Path: " << _paths.pages << std::endl;
            std::cout << "CGI: " << _paths.CGI << std::endl;
            std::cout << "Upload: " << _paths.upload << std::endl;
            std::cout << "Index: " << _index << std::endl;
            for (std::set<std::string>::iterator it = _pages.begin(); it != _pages.end(); ++it)
                std::cout << "Page: " << *it << std::endl;
            for (std::map<int, std::string>::iterator it = _errors.begin(); it != _errors.end(); ++it)
                std::cout << "Error " << it->first << ": " << it->second << std::endl;
#endif
        }

    public:
        int sendResponse(const Request& request){
            if (request.method() == GET)
                return getMethod(request);
            if (request.method() == POST)
                return postMethod(request);
            if (request.method() == DELETE)
                return deleteMethod(request);
            return errorPage(request, 400, "400 Bad Request");
        }

    private:
        int getMethod(const Request& request) {
            // if (getType(request.URI()) == "CGI"){
            //     std::set<std::string>::const_iterator it(std::find(_CGI.begin(), _CGI.end(), request.URI()));
            //     if (it == _CGI.end())
            //         return errorPage(request, 404, "404 Not Found");
            //     CGIstatus status = CGIthingy().generateResponse(request, _paths, _paths.CGI + *it);
            //     for (std::set<std::string>::iterator it = status.uploads.begin(); it != status.uploads.end(); ++it)
            //         addUpload(*it);
            //     return Response(request.fd(), status.status, status.type, status.message).send();
            // }
            // if (request.URI().find(_paths.upload) != std::string::npos){
            //     // std::cout << "Searching for: " << request.URI() << std::endl;
            //     // for (std::set<std::string>::iterator it = _uploads.begin(); it != _uploads.end(); ++it){
            //     //     std::cout << "UPLOAD: " << *it << std::endl;
            //     // }
            // }
            if (request.URI() == "/")
                return Response(request.fd(), "200 OK", getType(_index), getString(_paths.pages + _index)).send();
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
            return Response(request.fd(), "200 OK", getType(_paths.pages + *it), getString(_paths.pages + *it)).send();
        }

        int postMethod(const Request& request) {
            if (getType(request.URI()) == "CGI"){
                std::set<std::string>::const_iterator it(std::find(_CGI.begin(), _CGI.end(), request.URI()));
                if (it == _CGI.end())
                    return errorPage(request, 404, "404 Not Found");
                CGIstatus status = CGIthingy().generateResponse(request, _paths, _paths.CGI + *it);
                // for (std::set<std::string>::iterator it = status.uploads.begin(); it != status.uploads.end(); ++it)
                //     addUpload(*it);
                return Response(request.fd(), status.status, status.type, status.message).send();
            }
            return Response(request.fd(), "0", "0", "0").send(); //Placeholder
        }

        int deleteMethod(const Request& request) const {
            return Response(request.fd(), "0", "0", "0").send(); //Placeholder
        }

        int errorPage(const Request& request, int errorcode, const char* errormsg) const {
            std::map<int, std::string>::const_iterator it(_errors.find(errorcode));
            if (it == _errors.end())
                return Response(request.fd(), errormsg, "text/plain", errormsg).send();
            return Response(request.fd(), errormsg, getType(it->second), getString(_paths.pages + it->second)).send();
        }

        //Switch to mime.types thingy like NGINX
        std::string getType(const std::string& path) const {
            if (path.find(".gif") != path.npos)
                return std::string("image/gif");
            if (path.find(".jpg") != path.npos)
                return std::string("image/gif");
            if (path.find(".jpeg") != path.npos)
                return std::string("image/gif");
            if (path.find(".html") != path.npos)
                return std::string("text/html");
            if (path.find(".cgi") != path.npos)
                return std::string("CGI");
            return std::string("text/plain");
        }

        std::string getString(const std::string& path) const {
            std::ifstream file(path);
            if (file.is_open() == false)
                return std::string();
            std::stringstream ss;
            ss << file.rdbuf();
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
                if (line.compare(0, 7, "INDEX: ") == 0){
                    _index = "/" + std::string(&line[7]);
                    continue;
                }
                if (line.compare(0, 6, "PATH: ") == 0){
                    _paths.pages = &line[6];
                    continue;
                }
                if (line.compare(0, 8, "UPLOAD: ") == 0){
                    _paths.upload = &line[8];
                    continue;
                }
                if (line.compare(0, 10, "CGI_PATH: ") == 0){
                    _paths.CGI = std::string(std::string(&line[10]));
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
                    continue;
                }
                if (line.compare(0, 5, "CGI: ") == 0){
                    line = &line[5];
                    while (line.size() != 0){
                        size_t space = line.find(' ', 0);
                        if (space == std::string::npos){
                            _CGI.insert("/" + line);
                            break;
                        }
                        _CGI.insert("/" + line.substr(0, space));
                        line = &line[++space];
                    }
                    continue;
                }
                if (isnumber(line[0])){
                    int code = atoi(line.data());
                    if (code < 100 || code > 999)
                        return -1;
                    _errors.insert(std::make_pair(code, "/" + std::string(&line[5])));
                }  
            }
            return 0; //Add some kind of error hadling in misconfigurations later?
        }

        void addUpload(const std::string& path){
            // _uploads.insert("/" + _paths.upload + "/" + path);
        }

    private:
        PATHS                               _paths;
        std::string                         _index;
        std::set<std::string>               _CGI;
        std::set<std::string>               _pages;
        std::set<std::string>               _uploads;
        std::map<int, std::string>          _errors;
};