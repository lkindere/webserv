
#include <unistd.h>
#include <iostream>

#include "cgi.hpp"
#include "Request.hpp"

using namespace std;

cgi::cgi(Request &request) {
    _env.insert(make_pair("SERVER_SOFTWARE", "webserv/1.0"));
    _env.insert(make_pair("SERVER_NAME", "localhost/127.0.0.1"));
    _env.insert(make_pair("GATEWAY_INTERFACE", "CGI/1.1"));
    _env.insert(make_pair("SERVER_PROTOCOL", "HTTP/1.0"));
    _env.insert(make_pair("REQUEST_METHOD", "port"));
    _env.insert(make_pair("PATH_INFO", ""));
    _env.insert(make_pair("PATH_TRANSLATED", "real path to file"));
    // _env.insert(make_pair("SCRIPT_NAME", "cgi-bin" + programname));
    _env.insert(make_pair("QUERY_STRING", "fullpath with vars"));
    _env.insert(make_pair("REMOTE_HOST", request.host()));
    _env.insert(make_pair("REMOTE_ADDR", ""));
    _env.insert(make_pair("AUTH_TYPE", ""));
    _env.insert(make_pair("REMOTE_USER", ""));
    _env.insert(make_pair("REMOTE_IDENT", ""));
    _env.insert(make_pair("CONTENT_TYPE", ""));
    _env.insert(make_pair("CONTENT_LENGTH", request.message()));
    _env.insert(make_pair("HTTP_ACCEPT", ""));
    _env.insert(make_pair("HTTP_USER_AGENT", ""));
}

cgi::~cgi() {
}

string cgi::execute(string cgiPath) {
    cout << "CGI THINGY\n";
    char **env;
    char **argv;
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
        execve(cgiPath.c_str(), argv, env);
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