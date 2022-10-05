#include <algorithm> //std::find linux
#include <sstream>
#include <set>

#include "Webserv.hpp"

using namespace std;

//HELPERS

std::pair<std::string, short> getHost(int sock_fd){
    sockaddr_in addr = {0};
    socklen_t alen = sizeof(addr);
    getsockname(sock_fd, (sockaddr*)&addr, &alen);
    uint32_t ip = ntohl(addr.sin_addr.s_addr);
    std::stringstream ss;
    ss << ((ip >> 24) & 0xFF) << '.'
        << ((ip >> 16) & 0xFF) << '.'
        << ((ip >> 8) & 0xFF) << '.'
        << (ip & 0xFF);
    return std::make_pair(ss.str(), ntohs(addr.sin_port));
}

//WEBSERV

Webserv::Webserv(const ConfigData& conf) : _global(conf.global) {
    set<pair<string, int> > socks;
    for (vector<ServerConfig>::const_iterator srv = conf.servers.begin();
        srv != conf.servers.end(); ++srv){
        _servers.push_back(Server(&_global, *srv));
        if (socks.insert(make_pair(srv->host, srv->port)).second == true){
            _sockets.push_back(Socket(srv->host, srv->port));
            cout << "Socket: " << srv->host << ":" << srv->port << std::endl;       
        }
    }
}

int Webserv::init(){
    for (size_t i = 0; i < _sockets.size(); ++i){
        int ret = _sockets[i].init();
        if (ret != 0)
            return ret;
    }
    return 0;
}

int Webserv::accept(){
    for (size_t i = 0; i < _sockets.size(); ++i){
        int ret = _sockets[i].socket_accept();
        while (ret >= 0){
            pollfd pfd;
            pfd.fd = ret;
            pfd.events = POLLIN | POLLOUT;
            _connections.push_back(pfd);
            ret = _sockets[i].socket_accept();
        }
        if (ret == -1)
            return -1;
    }
    return 0;
}

int Webserv::process(){
    int ret = poll(_connections.data(), _connections.size(), 0);
    if (ret < 0)
        return error();
    if (ret == 0)
        return 0;
    for (vector<pollfd>::iterator it = _connections.begin();
        it != _connections.end(); ++it){
        if (it->revents & POLLERR || it->revents & POLLHUP || it->revents & POLLNVAL)
            it->fd = -1;
        if (it->revents & POLLIN)
            _requests[it->fd].push_back(Request(it->fd));
        if (it->revents & POLLOUT){
            if (serve(it->fd) != 0)
                return -1;
        }
    }
    //Rebuild vector without -1 connections?
    return 0;
}

int Webserv::serve(int fd){
    map<int, deque<Request> >::iterator it(_requests.find(fd));
    if (it == _requests.end() || it->second.size() == 0)
        return 0;
    Server*         server(getServer(it->second.front()));
    if (server == NULL){
        cout << "SHOULDN'T HAPPEN EITHER\n";
        return -1;
    }
    server->serve(it->second.front());
    it->second.pop_front();
    return 0;
}

Server* Webserv::getServer(const Request& request){
    Server* first = NULL;
    pair<string, short> host(getHost(request.fd()));
    for (size_t i = 0; i < _servers.size(); ++i){
        if (_servers[i].host() != host.first || _servers[i].port() != host.second)
            continue;
        if (_servers[i].checkNames(request.host()) == 0){
            if (first == NULL)
                first = &_servers[i];
            continue;
        }
        return &_servers[i];
    }
    if (first == NULL)
        cout << "THIS SHOULD NOT HAPPEN, SMTH SMTH ERROR\n";
    return first;
}


