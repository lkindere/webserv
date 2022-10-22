#pragma once

#include <string>
#include <vector>
#include <fstream>

class Request;

class Cgi {
	public:
		Cgi(const std::vector<std::string>& env);

		int execute(std::string path, FILE* in, FILE* out);

	private:
		std::vector<std::string> _env;
};
