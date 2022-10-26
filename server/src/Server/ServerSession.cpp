#include "Server.hpp"
#include "uString.hpp"

using namespace std;

int Server::serveCustom(Request& request, const string& status, const string& message) const {
    string ret = "<!DOCTYPE html><html><head><title>Error</title></head><body><h1>";
    ret += message + "</h1></body></html>";
    request.generateResponse(status, "text/html", ret);
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
        return serveError(request, 500);
    deque<string> user = split(vars[0], "=", true);
    deque<string> pass = split(vars[1], "=", true);
    int level = 0;
    if (user.size() != 2 || pass.size() != 2)
        return serveCustom(request, "400 Bad Request", "Invalid input");
    if (vars.size() == 3) {
        deque<string> lvl = split(vars[2], "=", true);
        if (lvl.size() != 2)
            return serveCustom(request, "400 Bad Request", "No level selected");
        for (size_t i = 0; i < lvl[1].length(); ++i) {
            if (!isdigit(lvl[1][i]))
                return serveCustom(request, "400 Bad Request", "Level must be non-negative numeric");
        }
        level = atoi(lvl[1].data());
    }
    if (user[1].length() < 6)
        return serveCustom(request, "400 Bad Request", "Username must be at least 6 characters");
    if (pass[1].length() < 6)
        return serveCustom(request, "400 Bad Request", "Password must be at least 6 characters");
    if (_sessions.addUser(user[1], pass[1], level) == false) 
        return serveCustom(request, "401 Unauthorized", "This username already exists");
    vector<string> headers;
    headers.push_back(string("Set-Cookie: ") + "PotatoServUSER=" + _sessions.generateCookie(level));
    request.generateResponse("200 OK", "text/html", "User created", headers);
    request.sendResponse();
    return 0;
}

int Server::login(Request& request, const Location& location) {
    if (location.autoindex())
        ;
    if (request.message().length() < request.contentlength())
        return 0;
    deque<string> vars = split(request.message(), "&", true);
    if (vars.size() != 2)
        return serveError(request, 500);
    deque<string> username = split(vars[0], "=", true);
    deque<string> password = split(vars[1], "=", true);
    if (username.size() != 2 || password.size() != 2)
        return serveCustom(request, "400 Bad Request", "Invalid input");
    const User* user = _sessions.getUser(username[1]);
    if (user == NULL)
        return serveCustom(request, "401 Unauthorized", "User not found");
    if (user->login(password[1]) == false)
        return serveCustom(request, "401 Unauthorized", "Invalid password");
    vector<string> headers;
    headers.push_back(string("Set-Cookie: ") + "PotatoServUSER=" + _sessions.generateCookie(user->level()));
    request.generateResponse("200 OK", "text/html", "Login successful", headers);
    request.sendResponse();
    return 0;
}
