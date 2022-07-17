/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkindere <lkindere@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/17 02:38:01 by lkindere          #+#    #+#             */
/*   Updated: 2022/07/17 15:00:21 by lkindere         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <unistd.h>

#include "Socket.hpp"

#define CIP "127.0.0.2"

int main(int argc, char **argv)
{
	Socket socket;

	socket.set_ip(CIP);
	std::cout << socket.address.sin_addr.s_addr << std::endl;
	socket.socket_connect();
	if (argv[1])
		socket.socket_send(argv[1], sizeof(argv[1]));
	else
		socket.socket_send("Hello", sizeof("Hello"));
	return 0;
}