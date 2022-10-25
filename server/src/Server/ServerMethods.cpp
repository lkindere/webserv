#include <unistd.h>
#include <fstream>
#include <sstream>

#ifdef DEBUG
    #include <iostream>
#endif

#include "Server.hpp"
#include "uString.hpp"
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
    if (access(path.data(), F_OK) != 0)
        return serveError(request, 404);
    if (access(path.data(), R_OK) != 0)
        return serveError(request, 403);
    if (isDirectory(path))
        return serveError(request, 404);
    ifstream file(path.data());
    if (file.is_open() == false)
        return serveError(request, 500);
    stringstream ss;
    ss << file.rdbuf();
    request.generateResponse("200 OK", getType(path), ss.str());
    request.sendResponse();
    return 0;
}

int Server::registration(Request& request, const Location& location) {
    if (location.autoindex())
        ;
    if (request.message().length() < request.contentlength())
        return 0;
    deque<string> vars = split(request.message(), "&", true);
    if (vars.size() < 2 || vars.size() > 3)
        return serveError(request, 500); //Invalid input
    deque<string> user = split(vars[0], "=", true);
    deque<string> pass = split(vars[1], "=", true);
    int level = 0;
    if (vars.size() == 3) {
        deque<string> lvl = split(vars[2], "=", true);
        if (lvl.size() != 2)
            return serveError(request, 500); //Invalid input
        level = atoi(lvl[1].data());
        if (level < 0)
            return serveError(request, 500); //Invalid input
    }
    if (user.size() != 2 || pass.size() != 2)
        return serveError(request, 500);
    if (_sessions.addUser(user[1], pass[1], level) == false)
        return serveError(request, 500); //Already exists
    vector<string> headers;
    headers.push_back(string("Set-Cookie: ") + "PotatoServUSER=" + _sessions.generateCookie(level));
    request.generateResponse("200 OK", "text/html", "User created", headers);
    request.sendResponse();
    return 0;
}

/**
 * @brief Only handles the "builtin" multipart uploader and registration form data
 * @param request 
 * @param location 
 * @return int 
 */
int Server::mpost(Request& request, const Location& location) {
    const set<string>& types = request.types();
    if (types.find("multipart/form-data") != types.end()
        && request.uri() == "/upload.html")
        return multipartUploader(request, location);
    if (types.find("application/x-www-form-urlencoded") != types.end()
        && request.uri() == "/register.html")
        return registration(request, location);
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
