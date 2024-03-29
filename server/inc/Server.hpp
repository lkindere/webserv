#pragma once

#include <string>
#include <vector>

#include "Config.hpp"
#include "Request.hpp"
#include "Sessions.hpp"

class Location {
public:
    Location(const LocationConfig &conf)
        : _config(conf) {}

    const std::string &uri() const { return _config.uri; }
    const std::string &root() const { return _config.root; }
    const std::string &index() const { return _config.index; }
    bool autoindex() const { return _config.autoindex; }
    const std::string &redirect() const { return _config.redirect; }
    const std::string &uploads() const { return _config.uploads; }
    int authentication() const { return _config.authentication; }
    const std::vector< e_method > &methods() const { return _config.methods; }
    const std::vector<std::string>& cgi_extensions() const { return _config.cgi_extensions; }

    //Cgi extension getter/checker
private:
    LocationConfig _config;
};

class Server {
public:
    Server(GlobalConfig *global, const ServerConfig &conf);

    int serve(Request &request);

private:
    //Server
    int serveRoot(Request &request);
    int serveLocation(Request &request, const Location &location);
    int serveDirectory(Request &request, const Location &location);
    int serveRedirect(Request& request, const Location& location) const;
    int serveAutoindex(Request& request, const Location& location, const std::string& path) const;
    int serveCGI(Request& request, const std::string& path) const;
    int serveError(Request &request, short error) const;
    int serveDefaultError(Request &request, const std::string &status) const;
    int serveCustom(Request& request, const std::string& status, const std::string& message) const;

    //ServerMethods
    int mget(Request &request, const std::string &path) const;
    int mpost(Request &request, const Location& location);
    int mdelete(Request &request, const std::string &path) const;

    //ServerUpload
    int multipartUploader(Request& request, const Location& location) const;

    //ServerSessions
    int login(Request& request, const Location& location);
    int registration(Request& request, const Location& location);

public:
    //ServerMisc
    int checkNames(const std::string &name) const;
    const Location *getLocation(const std::string &uri) const;
    std::vector<std::string> generateENV(const Request& request, const std::string& path) const;

    //Getters
    const std::string &host() const { return _config.host; }
    int port() const { return _config.port; }
    const std::vector< std::string > &names() { return _config.server_names; }
    const std::string &root() const { return _config.root; }
    const std::string &errorRoot() const { return _global->error_root; }
    const std::map< short, std::string > &errorPages() const { return _global->error_pages; }

private:
    GlobalConfig *_global;
    ServerConfig _config;
    Sessions _sessions;
    std::vector< Location > _locations;
};
