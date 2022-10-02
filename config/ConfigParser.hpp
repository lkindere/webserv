#pragma once

#include <string>
#include <vector>
#include <map>

/**
 * @brief Global parameters that affect all servers
 * @param client_max_body_size: defines max request message size
 */
struct GlobalConfig
{
    size_t                          client_max_body_size;
    std::string                     error_root;
    std::map<short, std::string>    error_pages;
};

/**
 * @brief Location block configuration
 * @param uri: request path
 * @param root: starting path for this location
 */
struct LocationConfig
{
    std::string uri;
    std::string root;
};


/**
 * @brief Server configuration
 * @param host: host
 * @param port: port
 * @param root: starting path used if none are defined in further blocks
 * @param locations: map<URL, LocationConfig struct>
 */
struct ServerConfig
{
    std::string host;
    int         port;
    std::string root;
    std::vector<LocationConfig> locations;
};

/**
 * @brief Status of parsing
 * @param success: true if no errors
 * @param error_line: line where error occured if applicable
 * @param error_msg: error description
 */
struct ConfigStatus
{
    bool        success;
    int         error_line;
    std::string error_msg;
};

/**
 * @brief All config data parsed from config file
 * @param status: ConfigStatus struct
 * @param global: GlobalConfig struct
 * @param servers: vector<ServerConfig struct> per server
 */
struct ConfigData
{
    ConfigStatus                status;
    GlobalConfig                global;
    std::vector<ServerConfig>   servers;
};

ConfigData parseConfig(const std::string& path);