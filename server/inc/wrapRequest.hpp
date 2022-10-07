#pragma once

#include "Request.hpp"

class wrapRequest {
public:
    wrapRequest();
    wrapRequest& operator=(Request* req);

    bool operator==(Request* req);
    bool operator!=(Request* req);

    Request& operator*();
    Request* operator->();

    bool timeout(unsigned int timeout_limit);

    ~wrapRequest();

private:
    Request* request;
    size_t last_request;
};