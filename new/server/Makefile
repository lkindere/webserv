NAME :=	./out

CXX = c++
CXXFLAGS := -std=c++98 -DDEBUG

SRCS :=		main.cpp		\
			Socket.cpp 		\
			Request.cpp		\
			Webserv.cpp		\

OBJ_DIR := ./_obj
OBJS := $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))

INC := -I ../_include
LDFLAGS := -L../_libs/
LDLIBS := -lconfig

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p _obj
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@


$(NAME): $(OBJS) $(LDLIBS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $(NAME)

$(LDLIBS): 
	@make -C ../config

all: $(NAME)

clean:
	rm -rf $(OBJ_DIR)	
	make clean -C ../config

fclean: clean
	rm -f $(NAME)
	make fclean -C ../config

re: fclean all

.PHONY: all clean fclean re
