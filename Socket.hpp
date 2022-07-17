/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkindere <lkindere@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/17 01:04:33 by lkindere          #+#    #+#             */
/*   Updated: 2022/07/17 15:14:23 by lkindere         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#define IP INADDR_ANY
#define PORT 8080
#define BACKLOG	10

#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class Socket
{
	// private:
	public:
		int					fd;
		socklen_t			len;
		struct sockaddr_in	address;
		
	public:
		Socket(int domain = AF_INET, int type = SOCK_STREAM, int protocol = 0);
		~Socket();

		//Server side
		void	socket_listen();
		int		socket_accept();
		
		//Client side
		void	set_ip(const char *ip);
		void	socket_connect();
		size_t	socket_send(const void *buf, size_t len, int flags = 0);

};
