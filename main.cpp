/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkindere <lkindere@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/17 02:17:55 by lkindere          #+#    #+#             */
/*   Updated: 2022/09/26 18:12:02 by lkindere         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"
#include "Server.hpp"
#include "Request.hpp"

#include <fcntl.h>

int main(void)
{
	Socket	socket;
	socket.socket_listen();

    std::ofstream out;
    out.open("output.txt");
    Server server("config.conf");
	while (1){
		std::cout << "Waiting" << std::endl;
		int fd = socket.socket_accept();
        Request request(fd);
        request.printRequest(std::cout);
        request.printRequest(out);

        server.sendResponse(request);
		close(fd);
	}
	return 0;
}