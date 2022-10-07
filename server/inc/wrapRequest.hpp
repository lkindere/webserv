#pragma once

#include "Request.hpp"

/**
 * @brief Acts as a Request* for assignment, access and comparisons.
 * @brief Allows storing additional data and functions like last_request timer
 */
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