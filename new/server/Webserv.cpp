#include <algorithm> //std::find linux

#include "Webserv.hpp"

#include "Response.hpp"

using namespace std;

/**
 * @brief Splits lines by delim
 * @param str input str
 * @param delim delimiter
 * @param noempty don't return empty strings
 * @return deque<string> split lines
 */
static deque<string> split(const string& str, const string& delim, bool noempty = false) {
    deque<string> split;
    size_t start = 0;
    size_t end = str.find(delim);
    size_t index = 1;
    while (end != str.npos) {
        string segment(str.substr(start, end - start));
        if (noempty == false || segment.empty() == false)
            split.push_back(segment);
        start = end + delim.length();
        end = str.find(delim, start);
    }
    string segment(str.substr(start));
    if (noempty == false || segment.empty() == false)
        split.push_back(segment);
    return split;
}

//LOCATION

Location::Location(const LocationConfig& conf)
    : _config(conf){}

//SERVER

Server::Server(GlobalConfig* global, const ServerConfig& conf)
    : _global(global), _config(conf){
    for (size_t i = 0; i < conf.locations.size(); ++i)
        _locations.push_back(Location(conf.locations[i]));
}

int     Server::process(){
    list<Request>::iterator it(_requests.begin());
    while (it != _requests.end()){
        pollfd pfd = {0};
        pfd.fd = it->fd();
        pfd.events = POLLOUT;
        int ret = poll(&pfd, 1, 0);
        if (ret == 0)
            continue;
        if (ret == -1)
            return error();
        if (pfd.revents & POLLERR || pfd.revents & POLLHUP || pfd.revents & POLLNVAL)
            _requests.erase(it++);
        else if (pfd.revents & POLLOUT){
            Response response;
            response.init(*this, *it);
            _requests.erase(it++);
        }
        else
            ++it;
    }
    return 0;
}

void    Server::addRequest(const Request& request){
     _requests.push_back(request);
}

int     Server::checkNames(const std::string& name) const{
    if (find(_config.server_names.begin(), _config.server_names.end(), name)
        != _config.server_names.end())
        return 1;
    return 0;
}

size_t getMatches(deque<string>& path, deque<string>& uri){
    size_t matches = 0;
    for (size_t i = 0; i < path.size() && i < uri.size(); ++i){
        if (path[i] == uri[i])
            ++matches;
        else
            break;
    }
    if (matches == path.size() && matches == uri.size())
        return -1;
    return matches;
}

const Location* Server::getLocation(const std::string& uri) const{
    if (_locations.size() == 0)
        return NULL;
    deque<string>   path(split(uri, "/", true));
    const Location* ptr = NULL;
    size_t          current = 0;
    for (size_t i = 0; i < _locations.size(); ++i){
        deque<string> loc(split(_locations[i].config().uri, "/", true));
        size_t match = getMatches(path, loc);
        if (match == -1)
            return &_locations[i];
        if (match > current){
            ptr = &_locations[i];
            current = match;
        }
    }
    return ptr;
}


//LISTENER

Listener::Listener(const string& host, int port)
    : _host(host), _port(port), _socket(host, port){}

int Listener::init(){
#ifdef DEBUG
    cout << "\nStarting to listen: " << _host << ":" << _port << std::endl;
    cout << "Number of severs:   " << _servers.size() << '\n' << std::endl;
#endif
    return _socket.init();
}

int Listener::accept(){
    int ret = _socket.socket_accept();
    while (ret >= 0){
        pollfd connection = {0};
        connection.fd = ret;
        connection.events = POLLIN;
        _connections.push_back(connection);
        ret = _socket.socket_accept();
    }
    if (ret == -1)
        return error();
    return 0;
}

int Listener::process(){
    list<pollfd>::iterator it(_connections.begin());
    while (it != _connections.end()){
        int ret = poll(&(*it), 1, 0);
        if (ret == 0)
            continue;
        if (ret == -1)
            return error();
        if (it->revents & POLLERR || it->revents & POLLHUP || it->revents & POLLNVAL)
            _connections.erase(it++);
        else if (it->revents & POLLIN)
            matchServer(Request(it++->fd));
        else
            ++it;
    }
    for (size_t i = 0; i < _servers.size(); ++i)
        _servers[i].process();
    return 0;
}

void Listener::matchServer(const Request& request){
    if (_servers.size() == 1)
        _servers[0].addRequest(request);
    for (size_t i = 0; i < _servers.size() + 1; ++i){
        if (i == _servers.size())
            _servers[0].addRequest(request);
        else if (_servers[i].checkNames(request.host()) != 0){
            _servers[i].addRequest(request);
            break;
        }
    }
}


//WEBSERV

Webserv::Webserv(const ConfigData& conf) : _global(conf.global) {
    for (vector<ServerConfig>::const_iterator srv = conf.servers.begin();
        srv != conf.servers.end(); ++srv){
        for (size_t i = 0; i < _listeners.size() + 1; ++i){
            if (i == _listeners.size()){
                _listeners.push_back(Listener(srv->host, srv->port));
                _listeners.back().addServer(Server(&_global, *srv));
                break;
            }
            else if (_listeners[i].host() == srv->host && _listeners[i].port() == srv->port){
                _listeners[i].addServer(Server(&_global, *srv));
                break;
            }
        }
    }
}

int Webserv::init(){
    for (size_t i = 0; i < _listeners.size(); ++i){
        int ret = _listeners[i].init();
        if (ret != 0)
            return ret;
    }
    return 0;
}

int Webserv::accept(){
    for (size_t i = 0; i < _listeners.size(); ++i){
        int ret = _listeners[i].accept();
        if (ret != 0)
            return ret;
    }
    return 0;
}

int Webserv::process(){
    for (size_t i = 0; i < _listeners.size(); ++i){
        int ret = _listeners[i].process();
        if (ret != 0)
            return ret;
    }
    return 0;
}
