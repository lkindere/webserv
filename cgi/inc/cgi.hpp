#pragma once

#include <string>
#include <vector>

class Request;

class Cgi {
	public:
		Cgi(const std::vector<std::string>& env);

		std::string execute(Request& request, std::string path);

	private:
		std::vector<std::string> _env;
        std::string              _output;
};
