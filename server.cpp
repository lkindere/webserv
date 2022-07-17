/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkindere <lkindere@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/17 02:17:55 by lkindere          #+#    #+#             */
/*   Updated: 2022/07/17 15:01:36 by lkindere         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <unistd.h>

#include "Socket.hpp"

int main(void)
{
	char	buffer[1024] = {0};
	Socket	socket;

	socket.socket_listen();
	// std::cout << socket.address.sin_addr.s_addr << std::endl;
	while (1){
		std::cout << "Waiting" << std::endl;
		int fd = socket.socket_accept();
		int bytes_read = read(fd, buffer, 1024);
		std::cout << buffer << std::endl;
		close(fd);
		if (std::string(buffer) == "exit")
			break ;
	}
	
	return 0;
}