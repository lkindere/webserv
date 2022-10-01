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
    size_t      client_max_body_size;
};

/**
 * @brief Location block configuration
 * @param root: starting path for this location
 */
struct LocationConfig
{
    std::string root;
};

/**
 * @brief Error page configurations
 * @param root: starting path for all error pages
 * @param pages: map<error_code, page.html>
 */
struct ErrorConfig
{
    std::string root;
    std::map<short, std::string>  pages;
};

/**
 * @brief Server configuration
 * @param host: host
 * @param port: port
 * @param root: starting path used if none are defined in further blocks
 * @param locations: map<URL, LocationConfig struct>
 * @param error: ErrorConfig struct
 */
struct ServerConfig
{
    std::string host;
    int         port;
    std::string root;
    std::map<std::string, LocationConfig> locations;
    ErrorConfig error;

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

/**
 * @brief Parses config file
 * That's it for now idk what to write here tbh
 */
class ConfigParser
{
    public:
        ConfigParser(const std::string& path);

        ConfigData parse() const;

    private:
        ConfigData invalid(int line, const std::string& msg) const;
    
    private:
        std::string _path;
};