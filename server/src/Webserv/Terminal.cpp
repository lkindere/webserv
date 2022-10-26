#include <iostream>
#include <unistd.h>
#include <sstream>

#include "Webserv.hpp"
#include "uServer.hpp"

using namespace std;

/**
 * @brief Handles terminal 
 * @param fd 
 */
void Webserv::terminalHandler(int fd) {
    string buffer(255, 0);
    size_t bytes_read = read(fd, (void*)buffer.data(), 255);
    buffer.resize(bytes_read);
    if (buffer == "help\n" || buffer == "HELP\n")
        cout << "Available commands:\nEXIT: terminates the server\nLS: lists current connections\n";
    if (buffer == "exit\n" || buffer == "EXIT\n"){
        terminate();
        exit(0);
    }
    if (buffer == "ls\n" || buffer == "LS\n"){
        for (size_t i = 0; i < _sockets.size(); ++i) {
            stringstream ss;
            ss << "Socket: " << _sockets[i].host() << ":" << _sockets[i].port()
                << " fd: " << _sockets[i].fd() << '\n';
            cout << ss.rdbuf();
        }
        for (size_t i = 1 + _sockets.size(); i < _connections.size(); ++i){
            stringstream ss;
            pair<string, short> host(getHost(_connections[i].fd));
            ss << "Connection: " << host.first << ":" << host.second
                << " fd: " << _connections[i].fd << "\n";
            cout << ss.rdbuf();
        }
    }
}