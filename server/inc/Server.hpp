#pragma once

#include <string>
#include <vector>

#include "Config.hpp"
#include "Request.hpp"

class Location {
public:
    Location(const LocationConfig &conf);

    const std::string &uri() const { return _config.uri; }
    const std::string &root() const { return _config.root; }
    const std::string &index() const { return _config.index; }
    bool autoindex() const { return _config.autoindex; }
    const std::string &redirect() const { return _config.redirect; }
    const std::string &uploads() const { return _config.uploads; }
    const std::vector< e_method > &methods() const { return _config.methods; }

    //Cgi extension getter/checker
private:
    LocationConfig _config;
};

class Server {
public:
    Server(GlobalConfig *global, const ServerConfig &conf);

    int serve(Request &request) const;

private:
    int serveRoot(Request &request) const;
    int serveLocation(Request &request, const Location &location) const;
    int serveDirectory(Request &request, const Location &location) const;
    int serveError(Request &request, short error) const;
    int serveDefaultError(Request &request, const std::string &status) const;
    
    int mget(Request &request, const std::string &path) const;
    int mpost(Request &request, const std::string &path) const;
    int mdelete(Request &request, const std::string &path) const;

    int multipartUploader(Request& request, const std::string& path) const;

public:
    int checkNames(const std::string &name) const;
    const Location *getLocation(const std::string &uri) const;
    const std::string &host() const { return _config.host; }
    int port() const { return _config.port; }
    const std::vector< std::string > &names() { return _config.server_names; }
    const std::string &root() const { return _config.root; }
    const std::string &errorRoot() const { return _global->error_root; }
    const std::map< short, std::string > &errorPages() const { return _global->error_pages; }

private:
    GlobalConfig *_global;
    ServerConfig _config;
    std::vector< Location > _locations;
};