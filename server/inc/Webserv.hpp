#pragma once

#include <poll.h>
#include <vector>

#include "Config.hpp"
#include "Server.hpp"
#include "Socket.hpp"
#include "wrapRequest.hpp"

#define TIMEOUT 10


/**
 * @brief _connections layout: 0:[STDIN], SOCKETS, CONNECTIONS
 */
class Webserv {
public:
    Webserv(const ConfigData &conf);
    ~Webserv();

    int init();
    int process();
    void terminate();

private:
    int accept();
    int readwrite();
    int serve(Request& request);
    int checkclose(pollfd& pfd);
    int rebuild();
    const Server *getServer(Request &request);
    //Terminal
    void terminalHandler(int fd);

private:
    GlobalConfig _global;
    std::vector< Socket > _sockets;
    std::vector< Server > _servers;
    std::vector< pollfd > _connections;
    std::map< int, wrapRequest > _requests;
};