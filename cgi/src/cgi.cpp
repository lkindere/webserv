
#include <unistd.h>
#include <iostream>

#include "Cgi.hpp"
#include "Request.hpp"

using namespace std;

Cgi::Cgi(const map<string, string>& env) : _env(env) {}

string Cgi::execute(string path) {
    cout << "CGI THINGY\n";
    char **env = { NULL };
    char **argv = { NULL };
    pid_t pid;
    int std_in = dup(STDIN_FILENO);
    int std_out = dup(STDOUT_FILENO);
    string new_output;

//         env = envToString();
//         argv = keyMapConvert(_env[“PATH_TRANSLATED”]);
    FILE *file_in = fopen("tmpIn", "w+");
    FILE *file_out = fopen("tmpOut", "w+");
    int fdIn = fileno(file_in);
    int fdOut = fileno(file_out);
    int status;

    write(fdIn, _output.c_str(), _output.size());
    lseek(fdIn, 0, SEEK_SET);
    pid = fork();
    if (pid == -1) {
        cerr << "CGI: fork crashed" << endl;
        return ("<html><body>Error: CGI Process</body></html>");
    } else if (pid == 0) {
        dup2(fdIn, STDIN_FILENO);
        dup2(fdOut, STDOUT_FILENO);
        execve(path.data(), argv, env);
        cerr << "ERROR: CGI child: " << strerror(errno) << endl;
        write(STDOUT_FILENO, "Status: 500\r\n", 14);
        exit(1);
    } else {
        char buffer[4096];
        waitpid(-1, &status, 0);
        lseek(fdOut, 0, SEEK_SET);
        int ret = 1;
        while (ret > 0) {
            memset(buffer, 0, 4096);
            ret = read(fdOut, buffer, 4096 - 1);
            new_output += buffer;
        }
    }
    dup2(std_in, STDIN_FILENO);
    dup2(std_out, STDOUT_FILENO);
    fclose(file_in);
    fclose(file_out);
    close(fdIn);
    close(fdOut);
    remove("tmpIn");
    remove("tmpOut");
    // delete env
    return new_output;
}