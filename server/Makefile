CXX = c++
CXXFLAGS = -std=c++98 -Wall -Werror -Wextra -I $(INCDIR) -I $(CONFIG_PARSER_INC)
LDFLAGS = -L./libs -lconfigparser

CGI_SRCS_DIR = ../cgi

CONFIG_PARSER_DIR = ../parser
CONFIG_PARSER_LIB = $(CONFIG_PARSER_DIR)/libconfigparser.a
CONFIG_PARSER_INC = $(CONFIG_PARSER_DIR)/inc/

BIN = webserv
SRCDIR = ./src
BUILDDIR = ./build
OBJDIR = $(BUILDDIR)/obj
INCDIR = ./inc/
EXTENSION = .cpp

SOURCES := $(shell find $(SRCDIR) -type f -name '*$(EXTENSION)')
OBJECTS := $(patsubst $(SRCDIR)/%$(EXTENSION),$(OBJDIR)/%.o,$(SOURCES))
DEPENDS := $(patsubst $(SRCDIR)/%$(EXTENSION),$(BUILDDIR)/%.d,$(SOURCES))
LIBS := $(wildcard ./libs/*)
CGI_BINS := $(wildcard ./cgi-bin/*)

all: compile_cgis $(CONFIG_PARSER_LIB) $(BIN)
	

compile_cgis:
	@printf "$(shell tput setaf 2)Compiling CGI's$(shell tput sgr0)\n"
	@$(MAKE) -C $(CGI_SRCS_DIR)

$(CONFIG_PARSER_LIB):
	@printf "$(shell tput setaf 2)Compiling Config Parser Library$(shell tput sgr0)\n"
	@$(MAKE) -C $(dir $@)

$(BIN): $(OBJECTS)
	@printf "$(shell tput setaf 2)Compiling [$(BIN)]$(shell tput sgr0)\n"
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

-include $(DEPENDS)

$(OBJDIR)/%.o: $(SRCDIR)/%$(EXTENSION) Makefile
	@printf "$(shell tput setaf 2)Compiling [$<]$(shell tput sgr0)\n"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@
	@mv $(@:.o=.d) $(BUILDDIR)

# Cleaning Rules #

.PHONY: clean
clean:
	@$(MAKE) -C $(CGI_SRCS_DIR) clean
	@$(MAKE) -C $(dir $(CONFIG_PARSER_LIB)) clean
	@printf "$(shell tput setaf 1)Removing $(OBJECTS) $(DEPENDS) $(LIBS) $(CGI_BINS)$(shell tput sgr0)\n"
	@$(RM) $(OBJECTS) $(DEPENDS) $(LIBS) $(CGI_BINS)
	@$(RM) -r $(BUILDDIR)

.PHONY: fclean
fclean: clean
	@$(MAKE) -C $(CGI_SRCS_DIR) fclean
	@$(MAKE) -C $(dir $(CONFIG_PARSER_LIB)) fclean
	@printf "$(shell tput setaf 1)Removing $(BIN)$(shell tput sgr0)\n"
	@$(RM) $(BIN)

.PHONY: re
re: fclean all	


.PHONY: run
run: all
	./$(BIN) ../configs/basic.conf


get_cgi_bin_dir:
	@echo $(CURDIR)/cgi-bin