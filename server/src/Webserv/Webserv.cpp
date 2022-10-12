
#include <unistd.h>
#include <cstring> //memset
#include <set>
#include <sstream>

// #ifdef DEBUG
    #include <iostream>
// #endif

#include "Webserv.hpp"
#include "uServer.hpp"

using namespace std;

/**
 * @brief Contructs Webserv and all needed sockets + servers
 * @param conf 
 */
Webserv::Webserv(const ConfigData &conf) : _global(conf.global) {
    set< pair< string, int > > socks;
    for (vector< ServerConfig >::const_iterator srv = conf.servers.begin();
         srv != conf.servers.end(); ++srv) {
        _servers.push_back(Server(&_global, *srv));
        if (socks.insert(make_pair(srv->host, srv->port)).second == true){
            _sockets.push_back(Socket(srv->host, srv->port));
            cout << "Listening on: " << srv->host << ':' << srv->port << endl;
        }
    }
}

/**
 * @brief Initializes all sockets, pushes back STDIN + base sockets to _connections
 * @return int 0 on success 1 on error
 */
int Webserv::init() {
    pollfd pfd;
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;
    _connections.push_back(pfd);
    for (size_t i = 0; i < _sockets.size(); ++i) {
        int ret = _sockets[i].init();
        if (ret != 0)
            return ret;
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
        const short& revents = _connections[i + 1].revents;
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

/**
 * @brief Handles terminal 
 * 
 * @param fd 
 */
void Webserv::terminalHandler(int fd){
    cout << "FD: " << fd << std::endl;
    string buffer(255, 0);
    size_t bytes_read = read(fd, (void*)buffer.data(), 255);
    buffer.resize(bytes_read);
    if (buffer == "exit\n" || buffer == "EXIT\n"){
        terminate();
        exit(0);
    }
    if (buffer == "ls" || buffer == "LS"){

    }
}

int Webserv::readwrite() {
    if (_sockets.size() + 1 > _connections.size())
        throw("FATAL ERROR: LESS CONNECTIONS THAN SOCKETS");
    if (_connections[0].revents & POLLIN)
        terminalHandler(_connections[0].fd);
    for (vector< pollfd >::iterator it = _connections.begin() + _sockets.size() + 1;
        it != _connections.end(); ++it) {
        if (checkclose(*it) == 1)
            continue;
        wrapRequest& request = _requests[it->fd];
        if (it->revents & POLLIN){
            if (request == NULL)
                request = new Request(it->fd, _global.client_max_body_size);
            else
                request->readMessage();
            if (request->status() == POSTING || request->status() == RESPONDING) 
                it->events = POLLOUT;
        }
        else if (it->revents & POLLOUT && request != NULL){
            if (request->status() == POSTING){
                if (serve(*request) != 0)
                    return -1;
            }
            else if (request->status() == RESPONDING)
                request->sendResponse();
            if (request->status() == COMPLETED || request->status() == READING)
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
    cout << "POLL RET: " << ret << endl;
    if (ret < 0)
        return error();
    if (ret == 0)
        return rebuild();
    if (accept() != 0)
        return -1;
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
        if (rit->second != NULL && rit->second->status() == COMPLETED)
            rit->second = NULL;
    }
    cout << "NEXT\n";
    vector<pollfd>::iterator cit(_connections.begin() + _sockets.size() + 1);
    while (cit != _connections.end()){
        if (cit->fd != -1){
            map<int, wrapRequest>::iterator rit = _requests.find(cit->fd);
            if (rit != _requests.end() && rit->second.timeout(TIMEOUT) == true){
                _requests.erase(rit);
                shutdown(cit->fd, SHUT_RDWR);
                close(cit->fd);
                cit->fd = -1;
            }
        }
        if (cit->fd == -1)
            cit = _connections.erase(cit);
        else
            ++cit;
    }
#ifdef DEBUG
    cout << "\nConnections size: " << _connections.size() << '\n';
    cout << "Requests size:    " << _requests.size() << "\n\n";
#endif
    return 0;
}

/**~
 * @brief Serves a writeable fd if there are pending requests
 * @param fd 
 * @return int 0 on success -1 on error
 */
int Webserv::serve(Request& request) {
    const Server *server(getServer(request));
    if (server == NULL)
        throw("SHOULDN'T HAPPEN");
    return server->serve(request);
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

void Webserv::terminate() {
    cout << "TERMINATING\n";
    for (size_t i = 1; i < _connections.size(); ++i){
        shutdown(_connections[i].fd, O_RDWR);
        close(_connections[i].fd);
    }
}

Webserv::~Webserv() {
    terminate();
}