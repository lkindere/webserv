#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <utility>
#include "../inc/Request.hpp"

class cgi {
	public:
		cgi(Request &request);
		~cgi();

		std::string execute(std::string cgiPath);
		void getCgiEnv(Request &request);
	private:
		std::map<std::string, std::string> _env;
        std::string                        _output;
};
#endif