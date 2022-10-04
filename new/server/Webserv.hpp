#pragma once

#include <vector>
#include <list>
#include <poll.h>

#include "Config.hpp"
#include "Socket.hpp"
#include "Request.hpp"

// class Location
// {
//     public:
//         Location(const LocationConfig& conf) : _config(conf) {}

//     private:
//         LocationConfig  _config;
// };

class Server
{
    public:
        Server(const ServerConfig& conf);

        void    addRequest(const Request& request);
        int     checkNames(const std::string& name);

    private:
        ServerConfig        _config;
        std::list<Request>  _requests;
};

class Listener
{
    public:
        Listener(const std::string& host, int port);

        int init();
        int accept();
        int process();

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