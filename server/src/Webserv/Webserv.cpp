#include "Webserv.hpp"

#include <unistd.h>
#include <cstring>  //memset cuz linux doesn't like the same initializers
#include <algorithm>//std::find linux
#include <set>
#include <sstream>

#include <iostream>

using namespace std;

//HELPERS

/**
 * @brief Returns a IP:port pair from socket fd
 * @param sock_fd 
 * @return std::pair< std::string, short > 
 */
std::pair< std::string, short > getHost(int sock_fd) {
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t alen = sizeof(addr);
    getsockname(sock_fd, ( sockaddr * ) &addr, &alen);
    uint32_t ip = ntohl(addr.sin_addr.s_addr);
    std::stringstream ss;
    ss << ((ip >> 24) & 0xFF) << '.'
       << ((ip >> 16) & 0xFF) << '.'
       << ((ip >> 8) & 0xFF) << '.'
       << (ip & 0xFF);
    return std::make_pair(ss.str(), ntohs(addr.sin_port));
}

//WEBSERV

/**
 * @brief Contructs Webserv and all needed sockets + servers
 * @param conf 
 */
Webserv::Webserv(const ConfigData &conf) : _global(conf.global) {
    set< pair< string, int > > socks;
    for (vector< ServerConfig >::const_iterator srv = conf.servers.begin();
         srv != conf.servers.end(); ++srv) {
        _servers.push_back(Server(&_global, *srv));
        if (socks.insert(make_pair(srv->host, srv->port)).second == true) {
            _sockets.push_back(Socket(srv->host, srv->port));
            cout << "Socket: " << srv->host << ":" << srv->port << std::endl;
        }
    }
}

/**
 * @brief Initializes all sockets
 * @return int 0 on success 1 on error
 */
int Webserv::init() {
    for (size_t i = 0; i < _sockets.size(); ++i) {
        int ret = _sockets[i].init();
        if (ret != 0)
            return ret;
    }
    return 0;
}

/**
 * @brief Accepts all pending connection requests on all sockets
 * @return int 0 on success 1 on error
 */
int Webserv::accept() {
    for (size_t i = 0; i < _sockets.size(); ++i) {
        int ret = _sockets[i].socket_accept();
        while (ret >= 0) {
            pollfd pfd;
            pfd.fd = ret;
            pfd.events = POLLIN | POLLOUT | POLLRDHUP;
            _connections.push_back(pfd);
            ret = _sockets[i].socket_accept();
        }
        if (ret == -1)
            return -1;
    }
    return 0;
}

/**
 * @brief Checks if a connection needs to be closed and closes it
 * @param pfd pollfd struct
 * @return int 1 if closed 0 if not
 */
int Webserv::checkclose(pollfd& pfd){
    if (pfd.revents & POLLERR || pfd.revents & POLLHUP 
        || pfd.revents & POLLRDHUP || pfd.revents & POLLNVAL){
        close(pfd.fd);
        pfd.fd = -1;
        cout << "FD SET TO -1\n";
        return 1;
    }
    // if timeout
    //  close
}

/**
 * @brief Loops through all connections, closes invalid, creates Requests, serves to servers
 * @return int 0 on success 1 on error
 */
int Webserv::process() {
    if (_connections.size() == 0)
        return 0;
    int ret = poll(_connections.data(), _connections.size(), -1);
    if (ret < 0)
        return error();
    if (ret == 0)
        return 0;
    for (vector< pollfd >::iterator it = _connections.begin(); it != _connections.end(); ++it) {
        if (checkclose(*it) == 1)
            continue;
        else if (it->revents & POLLIN && _requests[it->fd] == NULL)
            _requests[it->fd] = new Request(it->fd, _global.client_max_body_size);
        else if (it->revents & POLLOUT && _requests[it->fd] != NULL)
            if (serve(*(_requests[it->fd])) != 0)
                return -1;
    }
    return rebuild();
}

int Webserv::rebuild() {
    vector<pollfd>::iterator cit(_connections.begin());
    while (cit != _connections.end()){
        if (cit->fd == -1){
            cit = _connections.erase(cit);
            cout << "CONNECTION ERASED\n";
            exit(0);
        }
        else
            ++cit;
    }
    map<int, Request*>::iterator rit(_requests.begin());
    while (rit != _requests.end()){
        if (rit->second != NULL && rit->second->status() == COMPLETED){
            cout << "\nREQUEST DELETED\n";
            delete rit->second;
            _requests.erase(rit++);
        }
        else
            ++rit;
    }
    return 0;
}

/**~
 * @brief Serves a writeable fd if there are pending requests
 * @param fd 
 * @return int 0 on success -1 on error
 */
int Webserv::serve(Request& request) {
    const Server *server(getServer(request));
    if (server == NULL){
        throw("SHOULDN'T HAPPEN");
        return -1;
    }
    server->serve(request);
    return 0;
}

/**
 * @brief Finds the appropriate server for a request
 * @param request 
 * @return Server* server or NULL if not found
 */
const Server *Webserv::getServer(Request &request) {
    const Server *first = NULL;
    pair< string, short > host(getHost(request.fd()));
    for (size_t i = 0; i < _servers.size(); ++i) {
        if (_servers[i].host() != host.first || _servers[i].port() != host.second)
            continue;
        if (_servers[i].checkNames(request.host()) == 0) {
            if (first == NULL)
                first = &_servers[i];
            continue;
        }
        return &_servers[i];
    }
    if (first == NULL)
        throw("THIS SHOULD NOT HAPPEN, SMTH SMTH ERROR");
    return first;
}
