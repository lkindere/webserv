#include "wrapRequest.hpp"

#include <sys/time.h>

using namespace std;

static size_t getTime(){
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (tv.tv_usec / 1000000);
}

wrapRequest::wrapRequest()
    : request(NULL), last_request(getTime()) {}

wrapRequest& wrapRequest::operator=(Request* req) {
    delete request;
    request = req;
    if (request != NULL)
        last_request = getTime();
    return *this;
}

bool wrapRequest::operator==(Request* req){
    return request == req;
}

bool wrapRequest::operator!=(Request* req){
    return request != req;
}

Request& wrapRequest::operator*(){
    return *request;
}

Request* wrapRequest::operator->(){
    return request;
}

bool wrapRequest::timeout(unsigned int timeout_limit){
    if (request != NULL)
        return false;
    if (getTime() - last_request >= timeout_limit)
        return true;
    return false;
}

wrapRequest::~wrapRequest() { delete request; }