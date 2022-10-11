#include <unistd.h>
#include <fstream>
#include <sstream>

#ifdef DEBUG
    #include <iostream>
#endif

#include "Server.hpp"
#include "Types.hpp"

using namespace std;

/**
 * @brief Serves get request
 * @param request 
 * @param path 
 * @return int 0 on success
 */
int Server::mget(Request &request, const string& path) const {
    if (access(path.data(), F_OK) != 0)
        return serveError(request, 404);
    if (access(path.data(), R_OK) != 0)
        return serveError(request, 403);
    if (isDirectory(path))
        return serveError(request, 403);
    ifstream file(path.data());
    if (file.is_open() == false)
        return serveError(request, 500);
    stringstream ss;
    ss << file.rdbuf();
    string str(ss.str());
    sendResponse(request.fd(), "200 OK", getType(path), str);
    request.setStatus(COMPLETED);
    return 0;
}

int Server::mpost(Request& request, const string& path) const {
    const set<string>& types = request.types();
    if (types.find("application/x-www-form-urlencoded") != types.end()){
        request.setStatus(COMPLETED);
        return 0; //Not inplemented
    }
    else if (types.find("multipart/form-data") != types.end())
        return multipartUploader(request, path);
    else
        return plainUploader(request, path);
    return 0; //Not implemented - text/plain
}

/**
 * @brief Deletes a file, sends a response
 * @param request 
 * @param path 
 * @return int always 0
 */
int Server::mdelete(Request& request, const string& path) const {
    if (access(path.data(), F_OK) != 0)
        return serveError(request, 404);
    if (access(path.data(), W_OK) != 0)
        return serveError(request, 403);
    if (isDirectory(path))
        return serveError(request, 403);
    if (remove(path.data()) != 0){
        sendResponse(request.fd(), "204 No Content", "text/html", "No content");
        return 0;
    }
    sendResponse(request.fd(), "200 OK", "text/html", "File deleted");
    request.setStatus(COMPLETED);
    return 0;
}
