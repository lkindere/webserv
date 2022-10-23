#include <unistd.h>
#include <fstream>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef DEBUG
    #include <iostream>
#endif

#include "Server.hpp"
#include "uServer.hpp"

using namespace std;

string getFilename(const string& message) {
    size_t start = message.find("filename=\"");
    if (start == message.npos)
        return generateRandom(10);
    start += strlen("filename=\"");
    size_t end = message.find("\"", start);
    if (end == message.npos)
        return generateRandom(10);
    return message.substr(start, end - start);
}

void appendBeforeExtension(string& filename, const string& append) {
    size_t i = filename.rfind('.');
    if (i == filename.npos)
        filename += append;
    else
        filename.insert(i, append);
}

string generateUploadMessage(const string& filename, const string& uploadpath) {
    string message("<!DOCTYPE html><html><head><title>201 Created</title></head><body><h1>201 Created</h1>");
    message += "<p><a href=\"" + uploadpath + "/" + filename + "\">" + filename + "</a></p></body></html>";
    return message;
    // return string("<!DOCTYPE html><html><head><title>201 Created</title></head><body><h1>201 Created</h1></body></html>");
}

/**
 * @brief Uploads a multipart/form file to request URI
 * @param request 
 * @param path 
 * @return int 
 */
int Server::multipartUploader(Request& request, const Location& location) const {
    if (request.message().length() == 0 && request.contentlength() != 0){
        request.setStatus(READING);
        return 0;
    }
    static map<int, string> filenames;
    size_t start = 0;
    size_t end = request.message().find("\r\n--" + request.boundary() + "--");
    if (end == string::npos)
        end = request.message().length();
    if (request.postedlength() == 0){
        if (access((getCWD() + location.uploads()).data(), F_OK) != 0)
            mkdir((getCWD() + location.uploads()).data(), S_IRWXU);
        string file = getFilename(request.message());
        filenames.insert(make_pair(request.fd(), file));
        while (access((getCWD() + location.uploads() + "/" + filenames[request.fd()]).data(), F_OK) == 0)
            appendBeforeExtension(filenames[request.fd()], generateRandom(1));
        start = request.message().find("\r\n\r\n");
        if (start == string::npos)
            return serveError(request, 400);
        start += 4;
    }
    ofstream file((getCWD() + location.uploads() + "/" + filenames[request.fd()]).data(), ofstream::app);
    if (file.is_open() == false)
        return serveError(request, 500);
    file.write(&request.message()[start], end - start);
    request.setPosted(request.postedlength() + request.message().length());
#ifdef DEBUG
    cout << "CONTENT LEN: " << request.contentlength() << std::endl;
    cout << "READ LEN   : " << request.readlength() << std::endl;
    cout << "POSTED LEN: " << request.postedlength() << std::endl;
#endif
    if (request.postedlength() >= request.contentlength()){
        string message = generateUploadMessage(filenames[request.fd()], location.uploads());
        request.generateResponse("201 Created", "text/html", message);
        request.sendResponse();
        filenames.erase(request.fd());
        return 0;
    }
    request.setStatus(READING);
    return 0;
}
