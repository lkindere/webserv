
#include <unistd.h>
#include <iostream>

#include "Cgi.hpp"
#include "Request.hpp"

using namespace std;

class Pipe
{
    public:
        Pipe() : _error(false) {
            if (pipe(_pfd) == -1)
                _error = true;
        }
        // ~Pipe() {
        //     close(_pfd[0]);
        //     close(_pfd[1]);
        // }

        int in() const { return _pfd[0]; }
        int out() const { return _pfd[1]; }
        void closein() { close(_pfd[0]); }
        void closeout() { close(_pfd[1]); }
        bool error() const { return _error; }

    private:
        int _pfd[2];
        bool _error;
};

Cgi::Cgi(const vector<string>& env) : _env(env) {}


string Cgi::execute(Request& request, string path) {
    char** argv = { NULL };
    char* envp[_env.size() + 1];
    for (size_t i = 0; i < _env.size(); ++i)
        envp[i] = (char *)_env[i].data();
    envp[_env.size()] = NULL;
    Pipe p1;
    Pipe p2;
    pid_t pid = fork();
    if (pid < 0)
        return string();
    if (pid == 0){
        p1.closeout();
        dup2(p1.in(), STDIN_FILENO);
        // dup2(p2.out(), STDOUT_FILENO);
        execve(path.data(), argv, envp);
        cerr << "Execve error: " << strerror(errno) << endl;
        exit(1);
    }
    p1.closein();
    write(p1.out(), request.message().data(), request.message().length());
    p1.closeout();

    return string();
}