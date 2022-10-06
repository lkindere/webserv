NAME = webserv
SERVER_DIR = ./server

all: $(NAME)


$(NAME): $(shell find $(SERVER_DIR)/ -type f)
	@$(MAKE) -C $(SERVER_DIR) all
	@cp $(SERVER_DIR)/$(NAME) $(NAME)


clean:
	@$(MAKE) -C $(SERVER_DIR) clean

fclean:
	@$(MAKE) -C $(SERVER_DIR) fclean
	@$(RM) $(NAME)

re: fclean all

.PHONY: clean, fclean, re