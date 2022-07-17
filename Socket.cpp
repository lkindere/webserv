/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkindere <lkindere@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/17 01:46:49 by lkindere          #+#    #+#             */
/*   Updated: 2022/07/17 15:14:57 by lkindere         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"
#include <iostream>

Socket::Socket(int domain, int type, int protocol) {
	len = sizeof(address);
	fd = socket(domain, type, protocol);
	if (fd < 0)
		throw(std::runtime_error("Failed to create socket"));
	address.sin_family = domain;
	address.sin_addr.s_addr = IP;
	address.sin_port = htons(PORT);
	std::memset(address.sin_zero, 0, sizeof(address.sin_zero));
}

Socket::~Socket(){
	std::cout << "Socket closed\n";
	close(fd);
}

//Starts listening on the constructed socket
void Socket::socket_listen(){
	if (bind(fd, (struct sockaddr *)&address, sizeof(address)) < 0)
		throw(std::runtime_error("Failed to bind socket"));	
	if (listen(fd, BACKLOG) < 0)
		throw(std::runtime_error("Failed to listen to socket"));
}

//Accepts a connection to listening socket and returns new fd
int Socket::socket_accept(){
	int	accepted_fd = accept(fd, (struct sockaddr *)&address, &len);
	if (fd < 0)
		throw(std::runtime_error("Failed to accept connection"));
	return accepted_fd;
}

void Socket::set_ip(const char *ip){
	address.sin_addr.s_addr = inet_addr(ip);
	if (address.sin_addr.s_addr == -1)
		throw(std::runtime_error("Failed to set IP"));
}

//Connects to the socket
void Socket::socket_connect(){
	if (connect(fd, (struct sockaddr *)&address, sizeof(address)) < 0)
		throw(std::runtime_error("Failed to connect"));
}

//Sends data to socket returns bytes sent
size_t Socket::socket_send(const void *buf, size_t len, int flags){
	size_t ret = send(fd, buf, len, flags);
	if (ret < 0)
		throw(std::runtime_error("Failed to send data"));
	return ret;
}
