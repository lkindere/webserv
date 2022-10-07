#pragma once

#include <poll.h>

#include <list>
#include <vector>
#include <memory>

#include "Config.hpp"
#include "Request.hpp"
#include "Server.hpp"
#include "Socket.hpp"

class Webserv {
public:
    Webserv(const ConfigData &conf);

    int init();
    int accept();
    int process();

private:
    int serve(Request& request);
    int checkclose(pollfd& pfd);
    int rebuild();
    const Server *getServer(Request &request);

private:
    GlobalConfig _global;
    std::vector< Socket > _sockets;
    std::vector< Server > _servers;
    std::vector< pollfd > _connections;
    std::map< int, Request* > _requests;
};