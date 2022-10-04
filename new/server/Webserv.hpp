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


        const Location&         getLocation(const std::string& uri) const{
            const Location& best_match = _locations[0];
            for (size_t i = 0; i < _locations.size(); ++i){
                const LocationConfig& loc = _locations[i].config();
                //Remove from str end to first / from the end
                //If no match remove further until another /
                //Eventually should be left with / which would return the base / location
            }
        }

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