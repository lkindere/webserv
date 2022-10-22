#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sstream>
#include <cstring>
#include <algorithm>

#include "uServer.hpp"

using namespace std;

/**
 * @brief Generates n amount of alphanumeric chars
 * @param n 
 * @return string 
 */
string generateRandom(int n) {
    srand(time(NULL));
    char options[62];
    for (short i = 0; i < 10; ++i)
        options[i] = i + 48;
    for (short i = 10; i < 36; ++i)
        options[i] = i + 55;
    for (short i = 36; i < 62; ++i)
        options[i] = i + 61;
    string rnd(n, 0);
    for (size_t i = 0; i < rnd.size(); ++i)
        rnd[i] = options[rand() % 62];
    return rnd;
}

/**
 * @brief Returns 1 if path.extension matches cgi extensions
 * @param cgi_extensions 
 * @param path 
 * @return int 1 on true 0 on false
 */
int isCGI(const vector<string>& cgi_extensions, const string& path){
    size_t i = path.rfind('.');
    if (i == path.npos)
        return 0;
    if (find(cgi_extensions.begin(), cgi_extensions.end(), path.substr(i))
        != cgi_extensions.end())
        return 1;
    return 0;
}

/**
 * @brief Checks if path is a directory
 * @param path 
 * @return int 1 on directory 0 if not
 */
int isDirectory(const string &path) {
    struct stat sb;
    stat(path.data(), &sb);
    return (S_ISDIR(sb.st_mode));
}

/**
 * @brief Returns current working dir
 * @return string 
 */
string getCWD() {
    char* cwd = getcwd(NULL, 0);
    string ret(cwd);
    free(cwd);
    return ret;
}

/**
 * @brief Returns a IP from socket fd (CLIENT)
 * @param sock_fd 
 * @return string 
 */
string getPeer(int sock_fd) {
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t alen = sizeof(addr);
    getpeername(sock_fd, ( sockaddr * ) &addr, &alen);
    uint32_t ip = ntohl(addr.sin_addr.s_addr);
    stringstream ss;
    ss << ((ip >> 24) & 0xFF) << '.'
       << ((ip >> 16) & 0xFF) << '.'
       << ((ip >> 8) & 0xFF) << '.'
       << (ip & 0xFF);
    return ss.str();
}

/**
 * @brief Returns a IP:port pair from socket fd (SERVER)
 * @param sock_fd 
 * @return std::pair< std::string, short > 
 */
pair< string, short > getHost(int sock_fd) {
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t alen = sizeof(addr);
    getsockname(sock_fd, ( sockaddr * ) &addr, &alen);
    uint32_t ip = ntohl(addr.sin_addr.s_addr);
    stringstream ss;
    ss << ((ip >> 24) & 0xFF) << '.'
       << ((ip >> 16) & 0xFF) << '.'
       << ((ip >> 8) & 0xFF) << '.'
       << (ip & 0xFF);
    return make_pair(ss.str(), ntohs(addr.sin_port));
}

/**
 * @brief Generates new URI on matching location
 * @param root location root
 * @param location location uri
 * @param request request uri
 * @return string uri
 */
string generateLocationURI(const string &root, const string &location, const string &request) {
    size_t i = 0;
    while (i < location.size() && location[i] == request[i])
        ++i;
    string req(request.substr(i));
    if (req.size() == 0)
        req = "/";
    if (req[0] != '/')
        req = "/" + req;
    return string(getCWD() + root + req);
}

/**
 * @brief Basically just cuts off the CWD part
 * @param fullpath 
 * @return string 
 */
string getScriptname(const string& fullpath) {
    string cwd = getCWD();
    if (fullpath.find(cwd) == fullpath.npos)
        return fullpath;
    return fullpath.substr(cwd.length());
}
