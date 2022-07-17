# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lkindere <lkindere@student.42heilbronn.    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/07/17 15:05:47 by lkindere          #+#    #+#              #
#    Updated: 2022/07/17 15:13:07 by lkindere         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SERVER :=	./server
CLIENT :=	./client

S_SRCS :=	Socket.cpp		\
			server.cpp

C_SRCS :=	Socket.cpp		\
			client.cpp

all: client server

server: $(S_SRCS)
	c++ $(S_SRCS) -o $(SERVER)

client: $(C_SRCS)
	c++ $(C_SRCS) -o $(CLIENT)

clean:
	rm -f $(SERVER) $(CLIENT)