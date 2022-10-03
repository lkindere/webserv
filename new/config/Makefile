NAME :=	../_libs/libconfig.a

CXX = c++
CXXFLAGS := -std=c++98 -DDEBUG

SRCS :=		Config.cpp				\
			ConfigParser.cpp		\

OBJ_DIR := ./_obj
OBJS := $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))

INC := -I ../_include

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p _obj
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

$(NAME): $(OBJS)
	@mkdir -p ../_libs
	ar -r $(NAME) $(OBJS)

all: $(NAME)

clean:
	rm -rf $(OBJ_DIR)	

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
