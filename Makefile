NAME = webserv
SERVER_DIR = ./server

all: $(NAME)

run: all
	./webserv configs/basic.conf

$(NAME): $(shell find . -type f -name '*.cpp')
	@$(MAKE) -C $(SERVER_DIR) all
	@cp $(SERVER_DIR)/$(NAME) $(NAME)


clean:
	@$(MAKE) -C $(SERVER_DIR) clean

fclean:
	@$(MAKE) -C $(SERVER_DIR) fclean
	@$(RM) $(NAME)

re: fclean all

debug:
	@$(MAKE) -C $(SERVER_DIR) debug
	@cp $(SERVER_DIR)/$(NAME)_debug $(NAME)_debug

.PHONY: clean, fclean, re, debug