#pragma once

#include <set>
#include <string>
#include <unistd.h>
#include <cstdlib>

#include "Structs.hpp"
#include "Request.hpp"
#include "Response.hpp"

#include <poll.h>
#include <fcntl.h>

class Pipe
{
    public:
        Pipe() { pipe(pfd); }

        int in() { return pfd[0]; }
        int out() { return pfd[1]; }

        void close_in() { close(pfd[0]); }
        void close_out() { close(pfd[1]); }

    private:
        int pfd[2];
};

class CGIthingy
{
    public:
        CGIstatus generateResponse(const Request& request, const PATHS& paths, const std::string& CGI){
            std::vector<const char*> ARGV;
            ARGV.push_back((paths.upload + "/").data());
            ARGV.push_back(NULL);
            std::string response = runUploader(request, CGI, ARGV.data());

            CGIstatus ret;
            // size_t line = response.find("\r\n");
            // ret.status = response.substr(0, line);
            // ret.type = "text/html";
            // ret.message = response.substr(line);
            return ret;
        }

        void readWrite(int fd_in, int fd_out){
            pollfd fds[2];
            std::memset(fds, 0, sizeof(fds));
            fds[0].fd = fd_in;
            fds[0].events = POLLIN;
            fds[1].fd = fd_out;
            fds[1].events = POLLOUT;

            char buffer[BUFFER_SIZE];
            ssize_t bytes_read = 0;
            ssize_t bytes_wrote = 0;
            ssize_t offset = 0;
            std::ofstream OUT("SERVEROUT");
            while (1){
                if (poll(fds, 2, 50) <= 0
                    || fds[0].revents & POLLHUP || fds[1].revents & POLLHUP)
                    break;
                // std::cout << "IN: " << (fds[0].revents) << std::endl;
                // std::cout << "OUT: " << (fds[1].revents) << std::endl;
                if (bytes_read == 0 && (fds[0].revents &= POLLIN)){
                    bytes_read = read(fds[0].fd, buffer, BUFFER_SIZE);
                    offset = 0;
                }
                if (bytes_read > 0 && (fds[1].revents &= POLLOUT)){
                    bytes_wrote = write(fds[1].fd, buffer + offset, bytes_read);
                    std::cout << "\nSERVER BYTES WROTE: " << bytes_wrote << std::endl;
                    OUT.write(buffer + offset, bytes_wrote);
                    OUT.flush();
                    bytes_read -= bytes_wrote;
                    if (bytes_read != 0)
                        offset += bytes_wrote;
                }
            }
        }

        std::string runUploader(const Request& request, const std::string& CGI, const char** argv){
            Pipe p1;
            Pipe p2;
            fcntl(p1.in(), F_SETFL, O_NONBLOCK);
            fcntl(p1.out(), F_SETFL, O_NONBLOCK);
            fcntl(p2.in(), F_SETFL, O_NONBLOCK);
            fcntl(p2.out(), F_SETFL, O_NONBLOCK);
            int PID = fork();
            if (PID == 0){
                dup2(p1.in(), STDIN_FILENO);
                dup2(p2.out(), STDOUT_FILENO);
                p1.close_out();
                p2.close_in();
                execv(CGI.data(), (char* const*)argv);
            }
            p1.close_in();
            p2.close_out();
            readWrite(request.fd(), p1.out());
            // std::cout << "WRITING LENGTH: " << message.length();
            // write(p1.out(), message.data(), message.length());
            // std::cout << "PREPARING TO READ\n";
            // std::string response;
            // response.resize(10000);
            // int bytes_read = read(p2.in(), (void*)response.data(), 10000);
            // std::cout << "READ: " << bytes_read << std::endl;
            // p1.close_out();
            // p2.close_in();
            // return std::string(response.substr(0, bytes_read));
            return std::string();
        }

};