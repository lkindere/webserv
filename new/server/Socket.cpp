#include "Socket.hpp"

#include <fcntl.h>

#include <iostream>
#include <cerrno>

int error(){
    std::cerr << strerror(errno) << '\n';
    return -1;
};

Socket::Socket(const std::string& host, int port)
    : _fd(-1), _len(sizeof(_address)){
    _address.sin_family = AF_INET;
    _address.sin_addr.s_addr = inet_addr(host.data());
    _address.sin_port = htons(port);
    std::memset(_address.sin_zero, 0, sizeof(_address.sin_zero));
}

int Socket::init() {
    if (_address.sin_addr.s_addr == INADDR_NONE) //Same as 255.255.255.255 don't allow this IP before config?
        return error();
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0)
        return error();
    if (fcntl(_fd, F_SETFL, O_NONBLOCK) != 0)
        return error();
    if (bind(_fd, (sockaddr*)&_address, sizeof(_address)) != 0)
        return error();
    if (listen(_fd, BACKLOG) != 0)
        return error();
    std::cout << "INIT SUCCESS\n";
    return 0;
}

int Socket::socket_accept(){
    int accepted_fd = accept(_fd, (sockaddr*)&_address, &_len);
    if (accepted_fd == -1){
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            return error();
        return -1337;
    }
    fcntl(accepted_fd, F_SETFL, O_NONBLOCK);
    return accepted_fd;
}