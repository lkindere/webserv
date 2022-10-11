#include "cgi.hpp"

cgi::cgi(Request &request) {
	_env.insert(std::pair("SERVER_SOFTWARE", "webserv/1.0"));
	_env.insert(std::pair("SERVER_NAME", ""));
	_env.insert(std::pair("GATEWAY_INTERFACE", "CGI/1.1"));
	_env.insert(std::pair("SERVER_PROTOCOL", ""));
	_env.insert(std::pair("REQUEST_METHOD", ""));
	_env.insert(std::pair("PATH_INFO", ""));
	_env.insert(std::pair("PATH_TRANSLATED", ""));
	_env.insert(std::pair("SCRIPT_NAME", ""));
	_env.insert(std::pair("QUERY_STRING", ""));
	_env.insert(std::pair("REMOTE_HOST", ""));
	_env.insert(std::pair("REMOTE_ADDR", ""));
	_env.insert(std::pair("AUTH_TYPE", ""));
	_env.insert(std::pair("REMOTE_USER", ""));
	_env.insert(std::pair("REMOTE_IDENT", ""));
	_env.insert(std::pair("CONTENT_TYPE", ""));
	_env.insert(std::pair("CONTENT_LENGTH", ""));
	_env.insert(std::pair("HTTP_ACCEPT", ""));
	_env.insert(std::pair("HTTP_USER_AGENT", ""));
}

cgi::~cgi() {

}

std::string cgi::execute() {
 char **env;
 char **argv;
 pid_t pid;
 int std_in = dup(STDIN_FILENO);
 int std_out = dup(STDOUT_FILENO);
 std::string return_string;

 FILE *file_in = fopen("tmpIn", "w+");
 FILE *file_out = fopen("tmpOut", "w+");
 int fdIn = fileno(file_in);
 int fdOut = fileno(file_out);
 int status;

//  pid = fork();
//  if (pid == -1) {
// 	std::cerr << "CGI: fork crashed" << std::endl;
// 	return ("<html><body>Error: CGI Process</body></html>");
//  }
//  if (pid == 0) {

//  }
}

void cgi::getCgiEnv(Request &request) {
}