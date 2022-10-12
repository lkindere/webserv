#pragma once

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <string>

#define BACKLOG 100

int error();

class Socket {
public:
    Socket(const std::string &host, int port);
    ~Socket();

    int init();
    int socket_accept();
    
    int fd() const { return _fd; }

private:
    int _fd;
    socklen_t _len;
    struct sockaddr_in _address;
};