NAME = webserv
SERVER_DIR = ./server

all: $(NAME)

run: all
	./webserv configs/basic.conf

$(NAME): $(shell find $(SERVER_DIR) -type f -name *.cpp)
	@$(MAKE) -C $(SERVER_DIR) all
	@cp $(SERVER_DIR)/$(NAME) $(NAME)


clean:
	@$(MAKE) -C $(SERVER_DIR) clean

fclean:
	@$(MAKE) -C $(SERVER_DIR) fclean
	@$(RM) $(NAME)

re: fclean all

get_cgi_bin_dir:
	@echo $(CURDIR)/cgi-bin

.PHONY: clean, fclean, re, get_cgi_bin_dir
