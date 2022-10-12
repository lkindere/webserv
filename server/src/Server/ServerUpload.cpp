#include <unistd.h>
#include <fstream>

#ifdef DEBUG
    #include <iostream>
#endif

#include "Server.hpp"

using namespace std;

/**
 * @brief Uploads a multipart/form file to request URI
 * @param request 
 * @param path 
 * @return int 
 */
int Server::multipartUploader(Request& request, const string& path) const {
    size_t start = 0;
    size_t end = request.message().find("\r\n--" + request.boundary() + "--");
    if (end == string::npos)
        end = request.message().length();
    if (request.postedlength() == 0){
        if (access(path.data(), F_OK) == 0)
            return serveError(request, 405);
        start = request.message().find("\r\n\r\n");
        if (start == string::npos)
            return serveError(request, 400);
        start += 4;
    }
    ofstream file(path.data(), ofstream::app);
    if (file.is_open() == false)
        return serveError(request, 500);
    file.write(&request.message()[start], end - start);
    request.setPosted(request.postedlength() + request.message().length());
#ifdef DEBUG
    cout << "CONTENT LEN: " << request.contentlength() << std::endl;
    cout << "READ LEN   : " << request.readlength() << std::endl;
    cout << "POSTED LEN: " << request.postedlength() << std::endl;
#endif
    if (request.postedlength() >= request.contentlength()){
        string msg("<!DOCTYPE html><html><head><title>201 Created</title></head><body><h1>201 Created</h1></body></html>");
        request.generateResponse("201 Created", "text/html", msg);
        request.sendResponse();
        return 0;
    }
    request.setStatus(READING);
    return 0;
}

/**
 * @brief Uploads plain post data to a file (Not sure how to repliate via form, works with postman POST binary)
 * @param request 
 * @param path 
 * @return int 
 */
int Server::plainUploader(Request& request, const string& path) const {
    if (request.postedlength() == 0 && access(path.data(), F_OK) == 0)
        return serveError(request, 405);
    ofstream file(path.data(), ofstream::app);
    if (file.is_open() == false)
        return serveError(request, 500);
    file.write(request.message().data(), request.message().size());
    request.setPosted(request.postedlength() + request.message().length());
#ifdef DEBUG
    cout << "CONTENT LEN: " << request.contentlength() << std::endl;
    cout << "READ LEN   : " << request.readlength() << std::endl;
    cout << "POSTED LEN: " << request.postedlength() << std::endl;
#endif
    if (request.postedlength() >= request.contentlength()){
        string msg("<!DOCTYPE html><html><head><title>201 Created</title></head><body><h1>201 Created</h1></body></html>");
        request.generateResponse("201 Created", "text/html", msg);
        request.sendResponse();
        return 0;
    }
    request.setStatus(READING);
    return 0;
}