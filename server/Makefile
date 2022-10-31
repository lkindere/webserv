CXX = c++
CXXFLAGS = -std=c++98 -Wall -Werror -Wextra -I $(INCDIR) -I $(CONFIG_PARSER_INC) -I $(UTILS_INC) -I $(REQUEST_INC) -I $(CGI_INC)
LDFLAGS = -L./libs -lconfigparser -lutils -lrequest -lcgi

# CGI_SRCS_DIR = ../cgi

UTILS_LIB_DIR = ../utils
UTILS_LIB = $(UTILS_LIB_DIR)/libutils.a
UTILS_LIB_INC = $(UTILS_LIB_DIR)/inc/

CONFIG_PARSER_DIR = ../parser
CONFIG_PARSER_LIB = $(CONFIG_PARSER_DIR)/libconfigparser.a
CONFIG_PARSER_INC = $(CONFIG_PARSER_DIR)/inc/

UTILS_DIR = ../utils
UTILS_LIB = $(UTILS_DIR)/libutils.a
UTILS_INC = $(UTILS_DIR)/inc/

REQUEST_DIR = ../request
REQUEST_LIB = $(REQUEST_DIR)/librequest.a
REQUEST_INC = $(REQUEST_DIR)/inc/

CGI_DIR = ../cgi
CGI_LIB = $(CGI_DIR)/libcgi.a
CGI_INC = $(CGI_DIR)/inc/

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

all: $(CONFIG_PARSER_LIB) $(UTILS_LIB) $(REQUEST_LIB) $(CGI_LIB) $(BIN)
	

$(CONFIG_PARSER_LIB): $(shell find $(CONFIG_PARSER_DIR)/ -type f -name '*.cpp')
	@printf "$(shell tput setaf 2)Compiling Config Parser Library$(shell tput sgr0)\n"
	@$(MAKE) --no-print-directory -C $(dir $@)

$(UTILS_LIB): $(shell find $(dir $(UTILS_LIB))/ -type f -name '*.cpp')
	@printf "$(shell tput setaf 2)Compiling Utils Library$(shell tput sgr0)\n"
	@$(MAKE) -C $(dir $@)

$(REQUEST_LIB): $(shell find $(dir $(REQUEST_LIB))/ -type f -name '*.cpp')
	@printf "$(shell tput setaf 2)Compiling Request Library$(shell tput sgr0)\n"
	@$(MAKE) -C $(dir $@)

$(CGI_LIB): $(shell find $(dir $(CGI_LIB))/ -type f -name '*.cpp')
	@printf "$(shell tput setaf 2)Compiling CGI Library$(shell tput sgr0)\n"
	@$(MAKE) -C $(dir $@)

$(BIN): $(OBJECTS)
	@printf "$(shell tput setaf 2)Compiling [$(BIN)]$(shell tput sgr0)\n"
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

-include $(DEPENDS)

$(OBJDIR)/%.o: $(SRCDIR)/%$(EXTENSION)
	@printf "$(shell tput setaf 2)Compiling [$<]$(shell tput sgr0)\n"
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@
	@mv $(@:.o=.d) $(BUILDDIR)

# Cleaning Rules #

.PHONY: clean
clean:
# @$(MAKE) -C $(CGI_SRCS_DIR) clean
	@$(MAKE) -C $(dir $(CONFIG_PARSER_LIB)) clean
	@$(MAKE) -C $(dir $(UTILS_LIB)) clean
	@$(MAKE) -C $(dir $(REQUEST_LIB)) clean
	@$(MAKE) -C $(dir $(CGI_LIB)) clean
	@printf "$(shell tput setaf 1)Removing $(OBJECTS) $(DEPENDS) $(LIBS) $(CGI_BINS)$(shell tput sgr0)\n"
	@$(RM) $(OBJECTS) $(DEPENDS) $(LIBS) $(CGI_BINS)
	@$(RM) -r $(BUILDDIR)

.PHONY: fclean
fclean: clean
# @$(MAKE) -C $(CGI_SRCS_DIR) fclean
	@$(MAKE) -C $(dir $(CONFIG_PARSER_LIB)) fclean
	@$(MAKE) -C $(dir $(UTILS_LIB)) fclean
	@$(MAKE) -C $(dir $(REQUEST_LIB)) fclean
	@$(MAKE) -C $(dir $(CGI_LIB)) fclean
	@printf "$(shell tput setaf 1)Removing $(BIN)$(shell tput sgr0)\n"
	@$(RM) $(BIN)

.PHONY: re
re: fclean all	

.PHONY: run
run: all
	./$(BIN) ../configs/basic.conf

.PHONY: debug
debug:
	$(CXX) -g -DDEBUG $(CXXFLAGS) $(SOURCES) $(LDFLAGS) -o $(BIN)_debug

get_cgi_bin_dir:
	@echo $(CGI_BIN_DIR)
