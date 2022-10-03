#pragma once

#include <vector>
#include <list>
#include <poll.h>

#include "Config.hpp"
#include "Socket.hpp"

class Location
{
    public:
        Location(const LocationConfig& conf) : _config(conf) {}

    private:
        LocationConfig  _config;
};

class Server
{
    public:
        Server(const ServerConfig& conf);
    
        int init();
        int accept();
        int process();

    private:
        ServerConfig          _config;
        Socket                _socket;
        std::list<pollfd>     _connections;
};

class Webserv
{
    public:
        Webserv(const ConfigData& conf);

        int init();
        int accept();
        int process();

    private:
        GlobalConfig        _global;
        std::vector<Server> _servers;
};