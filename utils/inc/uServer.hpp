#pragma once

#include <string>
#include <vector>
#include <utility>


int         isCGI(const std::vector<std::string>& cgi_extensions, const std::string& path);
int         isDirectory(const std::string &path);
std::string                     getCWD();
std::string                     getPeer(int sock_fd);
std::pair< std::string, short > getHost(int sock_fd);
std::string getScriptname(const std::string& fullpath);
std::string generateLocationURI(const std::string &root,
    const std::string &location, const std::string &request);