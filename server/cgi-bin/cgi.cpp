#include "cgi.hpp"

cgi::cgi(Request &request) {
    _env.insert(std::pair("SERVER_SOFTWARE", "webserv/1.0"));
    _env.insert(std::pair("SERVER_NAME", "localhost/127.0.0.1"));
    _env.insert(std::pair("GATEWAY_INTERFACE", "CGI/1.1"));
    _env.insert(std::pair("SERVER_PROTOCOL", "HTTP/1.0"));
    _env.insert(std::pair("REQUEST_METHOD", "port"));
    _env.insert(std::pair("PATH_INFO", ""));
    _env.insert(std::pair("PATH_TRANSLATED", "real path to file"));
    // _env.insert(std::pair("SCRIPT_NAME", "cgi-bin" + programname));
    _env.insert(std::pair("QUERY_STRING", "fullpath with vars"));
    _env.insert(std::pair("REMOTE_HOST", request.host()));
    _env.insert(std::pair("REMOTE_ADDR", ""));
    _env.insert(std::pair("AUTH_TYPE", ""));
    _env.insert(std::pair("REMOTE_USER", ""));
    _env.insert(std::pair("REMOTE_IDENT", ""));
    _env.insert(std::pair("CONTENT_TYPE", ""));
    _env.insert(std::pair("CONTENT_LENGTH", request.message()));
    _env.insert(std::pair("HTTP_ACCEPT", ""));
    _env.insert(std::pair("HTTP_USER_AGENT", ""));
}

cgi::~cgi() {
}

std::string cgi::execute(std::string cgiPath) {
    char **env;
    char **argv;
    pid_t pid;
    int std_in = dup(STDIN_FILENO);
    int std_out = dup(STDOUT_FILENO);
    std::string new_output;

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
        std::cerr << "CGI: fork crashed" << std::endl;
        return ("<html><body>Error: CGI Process</body></html>");
    } else if (pid == 0) {
        dup2(fdIn, STDIN_FILENO);
        dup2(fdOut, STDOUT_FILENO);
        execve(cgiPath.c_str(), argv, env);
        std::cerr << "ERROR: CGI child" << std::endl;
        write(STDOUT_FILENO, "Status: 500\r\n", 14);
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