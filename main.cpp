/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lkindere <lkindere@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/17 02:17:55 by lkindere          #+#    #+#             */
/*   Updated: 2022/09/29 20:37:22 by lkindere         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <signal.h>

#include "Socket.hpp"
#include "Server.hpp"
#include "Request.hpp"

#include <fcntl.h>

Socket *tired_of_waiting_for_socket_to_close;

void close_fd_on_ctrl_c_temp(int){
    tired_of_waiting_for_socket_to_close->socket_close();
    exit(0);
}

int main(void)
{
	Socket	socket;
	socket.socket_listen();

    std::ofstream out("output.txt");
    Server server("config.conf");
	while (1){
		std::cout << "Waiting" << std::endl;
		int fd = socket.socket_accept();
        Request request(fd);
#ifdef DEBUG
        request.printRequest(std::cout);
#endif
        // request.printRequest(out);

        server.sendResponse(request);
		close(fd);
	}
	return 0;
}