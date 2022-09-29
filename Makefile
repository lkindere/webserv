# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: lkindere <lkindere@student.42heilbronn.    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/07/17 15:05:47 by lkindere          #+#    #+#              #
#    Updated: 2022/09/26 23:22:57 by lkindere         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SERVER :=	./server

S_SRCS :=	main.cpp		\
			Socket.cpp		\
			Request.cpp		\

all: server CGI

server: $(S_SRCS) Socket.hpp Request.hpp
	c++ $(S_SRCS) -o $(SERVER)
	c++ CGI/Upload.cpp -o CGI/UPLOAD.cgi

debug: $(S_SRCS) Socket.hpp Request.hpp
	c++ $(S_SRCS) -o $(SERVER) -DDEBUG

clean:
	rm -f $(SERVER)

re: clean all
