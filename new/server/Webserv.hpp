#pragma once

#include <vector>
#include <list>
#include <poll.h>

#include "Config.hpp"
#include "Server.hpp"
#include "Socket.hpp"
#include "Request.hpp"

class Webserv
{
    public:
        Webserv(const ConfigData& conf);

        int init();
        int accept();
        int process();

    private:
        int                     serve(int fd);
        Server*                 getServer(const Request& request);


    private:
        GlobalConfig                            _global;
        std::vector<Socket>                     _sockets;
        std::vector<Server>                     _servers;
        std::vector<pollfd>                     _connections;
        std::map<int, std::deque<Request> >     _requests;
};