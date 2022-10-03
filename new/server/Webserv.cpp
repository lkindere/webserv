#include "Webserv.hpp"
#include "Request.hpp"

Server::Server(const ServerConfig& conf)
    : _config(conf), _socket(conf.host, conf.port) {}

int Server::init(){
    std::cout << "Starting server at " << _config.host << ":" << _config.port << '\n';
    return _socket.init();
}

int Server::accept(){
    int ret = _socket.socket_accept();
    while (ret >= 0){
        pollfd connection = {0};
        connection.fd = ret;
        connection.events = POLLIN | POLLOUT;
        _connections.push_back(connection);
        std::cout << "Connections: " << _connections.size() << std::endl;
        ret = _socket.socket_accept();
    }
    if (ret == -1)
        return error();
    return 0;
}

int Server::process(){
    for (std::list<pollfd>::iterator it = _connections.begin();
        it != _connections.end(); ++it){
        int ret = poll(&(*it), 1, 0);
        if (ret == -1)
            return error();
        if (ret == 0)
            continue;
        if (it->revents & POLLERR || it->revents & POLLHUP || it->revents & POLLNVAL){
            if (it->revents & POLLHUP)
                std::cout << "HANG UP\n";
            if (it->revents & POLLERR)
                std::cout << "ERROR\n";
            if (it->revents & POLLNVAL)
                std::cout << "INVAL\n";
            _connections.erase(it);
        }
        if (it->revents & POLLIN){
            Request request(it->fd);
            if (it->revents & POLLOUT)
                std::cout << "Can also write\n";
        }
    }
    return 0;
}

Webserv::Webserv(const ConfigData& conf) : _global(conf.global) {
    _servers.reserve(conf.servers.size());
    for (size_t i = 0; i < conf.servers.size(); ++i)
        _servers.push_back(Server(conf.servers[i]));
}

int Webserv::init(){
    for (size_t i = 0; i < _servers.size(); ++i){
        int ret = _servers[i].init();
        if (ret != 0)
            return ret;
    }
    return 0;
}

int Webserv::accept(){
    for (size_t i = 0; i < _servers.size(); ++i){
        int ret = _servers[i].accept();
        if (ret != 0)
            return ret;
    }
    return 0;
}

int Webserv::process(){
    for (size_t i = 0; i < _servers.size(); ++i){
        int ret = _servers[i].process();
        if (ret != 0)
            return ret;
    }
    return 0;
}
