#include <unistd.h>
#include <fstream>
#include <sstream>

#ifdef DEBUG
    #include <iostream>
#endif

#include "Server.hpp"
#include "uServer.hpp"
#include "Types.hpp"

using namespace std;

/**
 * @brief Serves get request
 * @param request 
 * @param path 
 * @return int 0 on success
 */
int Server::mget(Request &request, const string& path) const {
    cout << "GETTING PATH: " << path << endl;
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
    request.generateResponse("200 OK", getType(path), ss.str());
    request.sendResponse();
    return 0;
}

int Server::mpost(Request& request, const Location& location) const {
    const set<string>& types = request.types();
    if (types.find("multipart/form-data") != types.end())
        return multipartUploader(request, location);
    return serveError(request, 403);
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
        request.generateResponse("204 No Content", "text/html", "No content");
        request.sendResponse();
        return 0;
    }
    request.generateResponse("200 OK", "text/html", "File deleted");
    request.sendResponse();
    return 0;
}
