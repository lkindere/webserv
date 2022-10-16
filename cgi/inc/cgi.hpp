#pragma once

#include <string>
#include <map>

class Request;

class Cgi {
	public:
		Cgi(const std::map< std::string, std::string >& env);

		std::string execute(std::string path);

	private:
		std::map<std::string, std::string> _env;
        std::string                        _output;
};
