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
 * @brief Initializes all sockets, pushes back to _connections
 * @return int 0 on success 1 on error
 */
int Webserv::init() {
    for (size_t i = 0; i < _sockets.size(); ++i) {
        int ret = _sockets[i].init();
        if (ret != 0)
            return ret;
        pollfd pfd;
        pfd.fd = _sockets[i].fd();
        pfd.events = POLLIN;
        _connections.push_back(pfd);
    }
    return 0;
}

/**
 * @brief Accepts all pending connection requests on all sockets
 * @return int 0 on success 1 on error
 */
int Webserv::accept() {
    for (size_t i = 0; i < _sockets.size(); ++i){
        const short& revents = _connections[i].revents;
        if (revents & POLLIN){
            int ret = _sockets[i].socket_accept();
            if (ret == -1)
                return ret;
            if (ret >= 0){
                pollfd pfd;
                pfd.fd = ret;
                pfd.events = POLLIN;
                _connections.push_back(pfd);
            }
        }
    }
    return 0;
}

int Webserv::readwrite() {
    if (_sockets.size() > _connections.size())
        throw("FATAL ERROR: LESS CONNECTIONS THAN SOCKETS");
    for (vector< pollfd >::iterator it = _connections.begin() + _sockets.size();
        it != _connections.end(); ++it) {
        if (checkclose(*it) == 1)
            continue;
        wrapRequest& request = _requests[it->fd];
        if (it->revents & POLLIN){
            if (request == NULL){
                cout << "\nREQUEST IS NULL, READING TO NEW\n";
                request = new Request(it->fd, _global.client_max_body_size);
            }
            else {
                cout << "\nREQUEST NOT NULL, READING TO SAME\n";
                request->readMessage();
            }
            if (request->status() == WRITING) 
                it->events = POLLOUT;
        }
        else if (it->revents & POLLOUT && request != NULL){
            if (serve(*request) != 0)
                return -1;
            it->events = POLLIN;
        }
    }
    return 0;
}

/**
 * @brief Closes connections if needed, setting fd to -1, removes matching request map
 * @param pfd pollfd struct
 * @return int 1 if closed 0 if not
 */
int Webserv::checkclose(pollfd& pfd){
    if (pfd.revents & POLLERR || pfd.revents & POLLHUP || pfd.revents & POLLNVAL){
        map<int, wrapRequest>::iterator it = _requests.find(pfd.fd);
        if (it != _requests.end())
            _requests.erase(it);
        shutdown(pfd.fd, SHUT_RDWR);
        close(pfd.fd);
        pfd.fd = -1;
        cout << "REQUEST HUP\n";
        return 1;
    }
    return 0;
}

/**
 * @brief Loops through all connections, closes invalid, creates Requests, serves to servers
 * @return int 0 on success 1 on error
 */
int Webserv::process() {
    int ret = poll(_connections.data(), _connections.size(), TIMEOUT * 1000);
    cout << "POLL RET: " << ret << std::endl;
    if (ret < 0)
        return error();
    if (ret == 0)
        return rebuild();
    // cout << "Accepting\n";
    if (accept() != 0)
        return -1;
    // cout << "Readwriting\n";
    if (readwrite() != 0)
        return -1;
    return rebuild();
}

/**
 * @brief NULLs completed requests, removes -1 fd connections
 * @return int always 0
 */
int Webserv::rebuild() {
    for (map<int, wrapRequest>::iterator rit = _requests.begin();
        rit != _requests.end(); ++rit){
        if (rit->second != NULL && rit->second->status() == COMPLETED){
            cout << "STATUS COMPLETED, SETTING NULL\n";
            rit->second = NULL;
        }
    }
    vector<pollfd>::iterator cit(_connections.begin() + _sockets.size());
    while (cit != _connections.end()){
        if (cit->fd != -1){
            map<int, wrapRequest>::iterator rit = _requests.find(cit->fd);
            if (rit != _requests.end() && rit->second.timeout(TIMEOUT) == true){
                _requests.erase(rit);
                shutdown(cit->fd, SHUT_RDWR);
                close(cit->fd);
                cit->fd = -1;
                cout << "REQUEST TIMEOUT\n";
            }
        }
        if (cit->fd == -1){
            cit = _connections.erase(cit);
            cout << "CONNECTION ERASED\n";
        }
        else
            ++cit;
    }
    cout << "\nConnections size: " << _connections.size() << '\n';
    cout << "Requests size:    " << _requests.size() << "\n\n";
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
