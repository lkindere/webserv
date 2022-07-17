/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkindere <lkindere@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/17 02:17:55 by lkindere          #+#    #+#             */
/*   Updated: 2022/07/17 16:11:56 by lkindere         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <unistd.h>
#include <sstream>
#include <string>

#include "Socket.hpp"

#define HTTP "HTTP/1.1"
#define STATUS "200 OK"
#define TYPE "text/plain"

void send_response(int fd, const char* message){
	std::stringstream ss;
	ss << HTTP << ' ' << STATUS << '\n' << "Content-Type: " << TYPE << '\n'
			<< "Content-Length: " << strlen(message) << "\n\n" << message;
	std::string response(ss.str());
	write (fd, response.c_str(), response.length());
}

int main(void)
{
	char	buffer[1024] = {0};
	Socket	socket;

	socket.socket_listen();
	while (1){
		std::cout << "Waiting" << std::endl;
		int fd = socket.socket_accept();
		int bytes_read = read(fd, buffer, 1024);
		std::cout << buffer << std::endl;
		send_response(fd, "Hello hello there");
		close(fd);
		if (std::string(buffer) == "exit")
			break ;
	}
	
	return 0;
}