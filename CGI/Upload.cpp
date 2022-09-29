#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sstream>

// ARGV LAYOUT
#define UPLOAD_PATH 0

#define BUFFER_SIZE 65536

struct Upload
{
    std::string     buffer;
    std::string     path;
    std::string     filename;
    std::string     boundary;
};

void uploadFailed() {
    std::cerr << "\nul FAILED\n";

    std::cout << "400 Bad Request\r\n";

    std::cout << "<!DOCTYPE html>\n"
        << "<html>\n"
        << "<body>\n"
        << "<h2>ul FAILED</h2>\n"
        << "</body>\n"
        << "</html>\n";

    exit(1);
}

void uploadSuccess(const std::string& URL) {
    std::cerr << "\nul SUCCESS\n";

    std::cout << "201 Created\r\n";

    std::cout << "<!DOCTYPE html>\n"
            << "<html>\n"
            << "<body>\n"
            << "<h2>ul SUCCESSFUL</h2>\n"
            << "<a href=\"" << URL << "\">THIS IS YOUR FILE</a>\n"
            << "</body>\n"
            << "</html>\n";

    exit(0);
}

std::string randomNumberThingy(){
    srand(time(NULL));
    std::stringstream ss;
    ss << rand();
    return std::string(ss.str());
}

void getFilename(Upload& ul){
    size_t start = ul.buffer.find("filename=");
    if (start == std::string::npos)
        uploadFailed();
    start += 10;
    size_t end = ul.buffer.find("\"", start);
    if (end == std::string::npos)
        uploadFailed();
    ul.filename = ul.buffer.substr(start, end - start);
    size_t endinfo = ul.buffer.find("\r\n\r\n");
    if (endinfo == std::string::npos)
        uploadFailed();
    ul.buffer = ul.buffer.substr(endinfo + 4);
}

void getBoundary(Upload& ul){
    size_t start = ul.buffer.find("------WebKitFormBoundary");
    if (start == std::string::npos)
        uploadFailed();
    size_t end = ul.buffer.find("\r\n", start);
    if (end == std::string::npos)
        uploadFailed();
    ul.boundary = ul.buffer.substr(start, end - start);
    ul.buffer = ul.buffer.substr(end + 2);
}

int fileExists(const std::string& path){
    std::ifstream file(path);
    if (file.is_open())
        return true;
    return false;
}

int checkBoundary(Upload& ul){
    size_t bound = ul.buffer.find("\r\n" + ul.boundary);
    if (bound == std::string::npos)
        return 0;
    ul.buffer = ul.buffer.substr(0, bound);
    std::cerr << "found bound\n";
    return 1;
}

void uploadFile(Upload& ul) {
    while (fileExists(ul.path + ul.filename))
        ul.filename = randomNumberThingy() + ul.filename;
    std::ofstream file(ul.path + ul.filename);
    if (file.is_open() == false)
        uploadFailed();
    ssize_t bytes_read = ul.buffer.length();
    ul.buffer.resize(BUFFER_SIZE);
    while (checkBoundary(ul) == 0){
        if (bytes_read > 0){
            file.write(ul.buffer.data(), bytes_read);
            file.flush();
        }
        bytes_read = read(STDIN_FILENO, (void*)ul.buffer.data(), BUFFER_SIZE);
    }
    file.write(ul.buffer.data(), ul.buffer.length());
    file.flush();
    uploadSuccess(ul.path + ul.filename);
}

int main(int argc, char** argv) {
    Upload ul;
    ul.buffer.resize(BUFFER_SIZE);
    ssize_t bytes_read = read(STDIN_FILENO, (void*)ul.buffer.data(), BUFFER_SIZE);
    ul.buffer.resize(bytes_read);
    ul.path = argv[UPLOAD_PATH];
    getBoundary(ul);
    getFilename(ul);
    uploadFile(ul);
    exit(0);
}