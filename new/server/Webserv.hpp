#pragma once

#include <vector>
#include <list>
#include <poll.h>

#include "Config.hpp"
#include "Socket.hpp"
#include "Request.hpp"

class Location
{
    public:
        Location(const LocationConfig& conf);

        const LocationConfig&   config() const{ return _config; }

        const std::string& uri() const { return _config.uri; }
        const std::string& root() const { return _config.root }
        const std::string& index() const {return _config.index; }
        bool               autoindex() const { return _config.autoindex; }
        const std::string& redirect() const { return _config.redirect; }
        const std::string& uploads() const { return _config.uploads; }
        
        //Method getter/checker
        //Cgi extension getter/checker
    private:
        LocationConfig  _config;
};

class Server
{
    public:
        Server(GlobalConfig* global, const ServerConfig& conf);

        int     process();

        void    addRequest(const Request& request);
        int     checkNames(const std::string& name) const;


        const Location*                 getLocation(const std::string& uri) const;
        const std::string&              host() const { return _config.host; }
        int                             port() const { return _config.port; }
        const std::string&              root() const { return _config.root; }
        const std::vector<std::string>& names() { return _config.server_names; }

    private:
        GlobalConfig*           _global;
        ServerConfig            _config;
        std::vector<Location>   _locations;
        std::list<Request>      _requests;
};

class Listener
{
    public:
        Listener(const std::string& host, int port);

        int     init();
        int     accept();
        int     process();
        
        void    addServer(const Server& server) { _servers.push_back(server); }

        const std::string&  host() const { return _host; }
        int                 port() const { return _port; }

    private:
        void    matchServer(const Request& request);

    private:
        std::string         _host;
        int                 _port;
        Socket              _socket;
        std::vector<Server> _servers;
        std::list<pollfd>   _connections;
};

class Webserv
{
    public:
        Webserv(const ConfigData& conf);

        int init();
        int accept();
        int process();

    private:
        GlobalConfig            _global;
        std::vector<Listener>   _listeners;
};