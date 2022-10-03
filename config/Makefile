CONF :=	./conf

S_SRCS :=	main.cpp				\
			Config.cpp				\
			ConfigParser.cpp		\

all: $(S_SRCS) Config.hpp ConfigParser.hpp
	c++ $(S_SRCS) -o $(CONF)

debug: $(S_SRCS) Config.hpp ConfigParser.hpp
	c++ $(S_SRCS) -o $(CONF) -DDEBUG

clean:
	rm -f $(CONF)

re: clean all
