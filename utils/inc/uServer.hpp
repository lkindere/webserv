#pragma once

#include <string>
#include <utility>

std::pair< std::string, short > getHost(int sock_fd);
int         isDirectory(const std::string &path);
std::string generateLocationURI(const std::string &root,
    const std::string &location, const std::string &request);