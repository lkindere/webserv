# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lkindere <lkindere@student.42heilbronn.    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/07/17 15:05:47 by lkindere          #+#    #+#              #
#    Updated: 2022/09/24 16:44:00 by lkindere         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SERVER :=	./server

S_SRCS :=	main.cpp		\
			Socket.cpp		\
			Request.cpp		\

all: server

server: $(S_SRCS) Socket.hpp Request.hpp
	c++ $(S_SRCS) -o $(SERVER)

debug: $(S_SRCS) Socket.hpp Request.hpp
	c++ $(S_SRCS) -o $(SERVER) -DDEBUG

clean:
	rm -f $(SERVER)

re: clean server
