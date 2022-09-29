#pragma once

#include <string>

struct PATHS
{
    std::string     pages;
    std::string     upload;
    std::string     CGI;
};

struct CGIstatus
{
    std::string             status;
    std::string             type;
    std::string             message;
    std::set<std::string>   uploads;
};
