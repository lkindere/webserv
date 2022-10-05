CXX = c++
CXXFLAGS = -std=c++98 -Wall -Werror -Wextra -I $(INCDIR)

CGI_SRCS_DIR = ../cgi

CONFIG_PARSER_DIR = ../parser
CONFIG_PARSER_LIB = $(CONFIG_PARSER_DIR)/libconfigparser.a

BIN = webserv
SRCDIR = ./src
BUILDDIR = ./build
OBJDIR = $(BUILDDIR)/obj
INCDIR = ./inc/
EXTENSION = .cpp

SOURCES := $(wildcard $(SRCDIR)/*$(EXTENSION))
OBJECTS := $(patsubst $(SRCDIR)/%$(EXTENSION),$(OBJDIR)/%.o,$(SOURCES))
DEPENDS := $(patsubst $(SRCDIR)/%$(EXTENSION),$(BUILDDIR)%.d,$(SOURCES))
LIBS := $(wildcard ./libs/*)
CGI_BINS := $(wildcard ./cgi-bin/*)

all: compile_cgis $(CONFIG_PARSER_LIB) $(BIN)
	

compile_cgis:
	$(MAKE) -C $(CGI_SRCS_DIR)

$(CONFIG_PARSER_LIB):
	$(MAKE) -C $(dir $@)

$(BIN): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

-include $(DEPENDS)

$(OBJDIR)/%.o: $(SRCDIR)/%$(EXTENSION) Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@
	@mv $(@:.o=.d) $(BUILDDIR)

# Cleaning Rules #

.PHONY: clean
clean:
	$(MAKE) -C $(CGI_SRCS_DIR) clean
	$(MAKE) -C $(dir $(CONFIG_PARSER_LIB)) clean
	$(RM) $(OBJECTS) $(DEPENDS) $(LIBS) $(CGI_BINS)
	$(RM) -r $(BUILDDIR)

.PHONY: fclean
fclean: clean
	$(MAKE) -C $(CGI_SRCS_DIR) fclean
	$(MAKE) -C $(dir $(CONFIG_PARSER_LIB)) fclean
	$(RM) $(BIN)

.PHONY: re
re: fclean all	


.PHONY: run
run: all
	./$(BIN)


get_cgi_bin_dir:
	@echo $(CURDIR)/cgi-bin