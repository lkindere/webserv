#pragma once

#include <map>
#include <string>
#include <stdio.h>

class Request;

void removeFilebuffer(std::map<int, std::pair<FILE*, FILE*> >& filebuffers,
    std::map<int, std::pair<FILE*, FILE*> >::iterator it);
int parseResponse(std::string& message,
    std::map<std::string, std::string>& headers, FILE* filebuffer);
int bufferToFile(Request& request, FILE* filebuffer);
int cgiExec(const std::vector<std::string>& env, const std::string& path, FILE* in, FILE* out);