
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>

#include "Cgi.hpp"
#include "Request.hpp"

using namespace std;

Cgi::Cgi(const vector<string>& env) : _env(env) {}


int Cgi::execute(string path, FILE* in, FILE* out) {
    char** argv = { NULL };
    char* envp[_env.size() + 1];
    for (size_t i = 0; i < _env.size(); ++i)
        envp[i] = (char *)_env[i].data();
    envp[_env.size()] = NULL;
    pid_t pid = fork();
    if (pid < 0)
        return 1;
    if (pid == 0){
        dup2(fileno(in), STDIN_FILENO);
        dup2(fileno(out), STDOUT_FILENO);
        execve(path.data(), argv, envp);
        cerr << "Execve error: " << strerror(errno) << endl;
        exit(1);
    }
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        return 1;
    rewind(out);
    return 0;
}