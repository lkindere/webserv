#include <unistd.h>
#include <fcntl.h>
#include <string.h>//strerror linux
#include <cstring>//memset linux
#include <cerrno>
#include <iostream>

#include "Socket.hpp"

using namespace std;

int error() {
    cerr << strerror(errno) << '\n';
    return -1;
};

Socket::Socket(const std::string &host, int port)
    : _fd(-1), _len(sizeof(_address)), _host(host), _port(port) {
    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = inet_addr(host.data());
    _address.sin_port = htons(port);
    memset(_address.sin_zero, 0, sizeof(_address.sin_zero));
}

/**
 * @brief Initializes a socket and starts listening
 * @return int 0 on success 1 + writes to cerr on error
 */
int Socket::init() {
    if (_address.sin_addr.s_addr == INADDR_NONE)//Same as 255.255.255.255 don't allow this IP before config?
        return error();
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0)
        return error();
    int i = 1;
    setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int));
    fcntl(_fd, F_SETFL, O_NONBLOCK);
    if (bind(_fd, ( sockaddr * ) &_address, sizeof(_address)) != 0)
        return error();
    if (listen(_fd, BACKLOG) != 0)
        return error();
    return 0;
}

/**
 * @brief accepts a connection if there is one, sets fd to nonblock
 * @return -1337 if would block but no error, -1 on error, fd on success 
 */
int Socket::socket_accept() {
    int accepted_fd = accept(_fd, ( sockaddr * ) &_address, &_len);
    if (accepted_fd == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            return error();
        return -1337;
    }
    int i = 1;
    setsockopt(accepted_fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &i, sizeof(int));
    fcntl(accepted_fd, F_SETFL, O_NONBLOCK);
    return accepted_fd;
}

Socket::~Socket() {
    shutdown(_fd, SHUT_RDWR);
    close(_fd);
}