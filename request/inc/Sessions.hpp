#pragma once

#include <string>
#include <set>
#include <map>

class User
{
    public:
        User(const std::string& user) : _username(user), _password(""), _level(0) {}
        User(const std::string& user, const std::string& pass, int level = 0)
            : _username(user), _password(pass), _level(level) {}

        bool login(const std::string& pass) const { return pass == _password; }

        const std::string& username() const { return _username; }
        int level() const { return _level; }
        void setLevel(int level) { _level = level; }
    
        bool operator<(const User& user) const { return _username < user._username; }
        bool operator>(const User& user) const { return _username > user._username; }
        bool operator==(const User& user) const { return _username == user._username; }
        bool operator!=(const User& user) const { return _username != user._username; }
        bool operator<(const std::string& user) const { return _username < user; }
        bool operator>(const std::string& user) const { return _username > user; }
        bool operator==(const std::string& user) const { return _username == user; }
        bool operator!=(const std::string& user) const { return _username != user; }

    private:
        std::string _username;
        std::string _password;
        int         _level;
};

class Sessions
{
    public:
        Sessions() {}
        
        std::string        generateCookie(const std::string& username);
        int                validCookie(const std::string& key, const std::string& value) const;
        bool               removeCookie(const std::string& key);

        bool               addUser(const std::string& user, const std::string& pass, int level = 0);
        const User*        getUser(const std::string& user) const;
        const User*        getUserByCookie(const std::string& cookie) const;

    private:
        std::set<User> _users;
        std::map<std::string, std::string> _cookies;
};
