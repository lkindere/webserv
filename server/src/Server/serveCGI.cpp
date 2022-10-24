#include "Server.hpp"
#include "serveCGI.hpp"
#include "uString.hpp"

using namespace std;

/**
 * @brief Saves space
 * @param filebuffers 
 * @param it 
 * @return int 
 */
void removeFilebuffer(map<int, pair<FILE*, FILE*> >& filebuffers, map<int, pair<FILE*, FILE*> >::iterator it) {
    fclose(it->second.first);
    fclose(it->second.second);
    filebuffers.erase(it);
}

/**
 * @brief Writes to filebuffer, rewinds when all is written
 * @param request 
 * @param filebuffer 
 * @return int 
 */
int bufferToFile(Request& request, FILE* filebuffer) {
    fwrite(request.message().data(), request.message().length(), 1, filebuffer);
    request.setPosted(request.postedlength() + request.message().length());
    if (request.postedlength() >= request.contentlength())
        rewind(filebuffer);
    return 0;
}

/**
 * @brief Grabs headers out of the CGI response, stores in headers&, stores message in message&
 * @param message 
 * @param headers 
 * @param filebuffer 
 * @return int 
 */
int parseResponse(string& message, map<string, string>& headers, FILE* filebuffer) {
    message.resize(BUFFER_SIZE);
    size_t bytes_read = fread((void*)message.data(), 1, BUFFER_SIZE, filebuffer);
    if (bytes_read < BUFFER_SIZE)
        message = message.substr(0, bytes_read);
    while (bytes_read == BUFFER_SIZE) {
        string tmp(BUFFER_SIZE, 0);
        bytes_read = fread((void*)tmp.data(), 1, BUFFER_SIZE, filebuffer);
        message += tmp.substr(0, bytes_read);
    }
    size_t start = 0;
    size_t end = 0;
    while (end == 0){
        if (message.substr(start, 2) == "\n\n")
            end = start + 2;
        else if (start + 3 < message.size()){
            if (message.substr(start, 4) == "\r\n\r\n")
                end = start + 4;
        }
        if (++start + 1 >= message.size())
            return 0;
    }
    deque<string> lines = split(message.substr(0, start), "\n", true);
    for (size_t i = 0; i < lines.size(); ++i){
        deque<string> current = split(lines[i], ": ", true);
        if (current.size() != 2)
            return 1;
        headers.insert(make_pair(current[0], current[1]));
    }
    message = message.substr(end);
    return 0;
}
