NAME = webserv
SERVER_DIR = ./server

all: $(NAME)


$(NAME): $(shell find $(SERVER_DIR)/ -type f -name '*.cpp')
	@$(MAKE) -C $(SERVER_DIR) all
	@cp $(SERVER_DIR)/$(NAME) $(NAME)


clean:
	@$(MAKE) -C $(SERVER_DIR) clean

fclean:
	@$(MAKE) -C $(SERVER_DIR) fclean
	@$(RM) $(NAME)

re: fclean all

run: all
	./$(NAME)

debug:
	@$(MAKE) -C $(SERVER_DIR) debug
	@cp $(SERVER_DIR)/$(NAME)_debug $(NAME)_debug

.PHONY: clean, fclean, re, debug