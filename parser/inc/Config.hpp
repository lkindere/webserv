#pragma once

//INCLUDE

#include <map>
#include <string>
#include <vector>

#include "uMethod.hpp"

/**
 * @brief Global parameters that affect all servers
 * @param client_max_body_size: defines max request message size
 * @param error_root: starting path for error pages
 * @param error_pages: map<error_code, page>
 */
struct GlobalConfig {
    size_t client_max_body_size;
    std::string error_root;
    std::map< short, std::string > error_pages;
};

/**
 * @brief Location block configuration
 * @param uri: request path
 * @param root: starting path for this location
 * @param index: default page if request is directory
 * @param autoindex: enable directory listing
 * @param redirect: redirect path
 * @param uploads: upload path, if not set uploads disabled
 * @param authentication: authentication level required to access location
 * @param methods: allowed methods for this location
 * @param cgi_extensions: files with .extension will be executed as CGI
 * @param list_directories: if true list directories
 */
struct LocationConfig {
    LocationConfig() : autoindex(false), authentication(0) {}
    std::string uri;
    std::string root;
    std::string index;
    bool autoindex;
    std::string redirect;
    std::string uploads;
    int authentication;
    std::vector< e_method > methods;
    std::vector< std::string > cgi_extensions;
};

/**
 * @brief Server configuration
 * @param host: host    
 * @param port: port
 * @param root: starting path used if none are defined in further blocks
 * @param server_names: server names
 * @param locations: map<URL, LocationConfig struct>
 */
struct ServerConfig {
    std::string host;
    int port;
    std::string root;
    std::vector< std::string > server_names;
    std::vector< LocationConfig > locations;
};

/**
 * @brief Status of parsing
 * @param success: true if no errors
 * @param error_line: line where error occured if applicable
 * @param error_msg: error description
 */
struct ConfigStatus {
    bool success;
    int error_line;
    std::string error_msg;
};

/**
 * @brief All config data parsed from config file
 * @param status: ConfigStatus struct
 * @param global: GlobalConfig struct
 * @param servers: vector<ServerConfig struct> per server
 */
struct ConfigData {
    ConfigStatus status;
    GlobalConfig global;
    std::vector< ServerConfig > servers;
};

e_method toEmethod(const std::string &method);
ConfigData parseConfig(const std::string &path);