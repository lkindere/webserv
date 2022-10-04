#pragma once

#include "Request.hpp"

#include <dirent.h>

class Response
{
    public:
        Response() {}

        int init(const Server& server, const Request& request){
            if (request.method() == INVALID)
                return errorPage(400);
            const Location& location = server.getLocation(request.uri());

            //Continue this bs

            return 0;
        }

        int errorPage(int error) {
            //Will return error pages based one error.... some day
            return 0;
        }

    private:
        std::string _response;
};