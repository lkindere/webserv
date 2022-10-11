CXX = c++
CXXFLAGS = -std=c++98 -Wall -Werror -Wextra -I $(INCDIR)

BIN = hello.cgi
SRCDIR = ./src
BUILDDIR = ./build
OBJDIR = $(BUILDDIR)/obj
INCDIR = $(SRCDIR)/inc/
EXTENSION = .cpp

SOURCES := $(wildcard $(SRCDIR)/*$(EXTENSION))
OBJECTS := $(patsubst $(SRCDIR)/%$(EXTENSION),$(OBJDIR)/%.o,$(SOURCES))
DEPENDS := $(patsubst $(SRCDIR)/%$(EXTENSION),$(BUILDDIR)/%.d,$(SOURCES))

.NOTPARALLEL:

all: $(BIN)


$(BIN): $(OBJECTS)
	@printf "$(shell tput setaf 2)Compiling [$(BIN)]$(shell tput sgr0)\n"
	@$(CXX) $(CXXFLAGS) $^ -o $@

-include $(DEPENDS)

$(OBJDIR)/%.o: $(SRCDIR)/%$(EXTENSION) Makefile
	@printf "$(shell tput setaf 2)Compiling [$<]$(shell tput sgr0)\n"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@
	@mv $(@:.o=.d) $(BUILDDIR)

# Cleaning Rules #

.PHONY: clean
clean:
	@printf "$(shell tput setaf 1)Removing $(OBJECTS) $(DEPENDS) $(BUILDDIR)$(shell tput sgr0)\n"
	@$(RM) $(OBJECTS) $(DEPENDS)
	@$(RM) -r $(BUILDDIR)

.PHONY: fclean
fclean: clean
	@printf "$(shell tput setaf 1)Removing $(BIN)$(shell tput sgr0)\n"
	@$(RM) $(BIN)

.PHONY: re
re: fclean all


.PHONY: run
run: all
	./$(BIN)

get_bin:
	@echo $(CURDIR)/$(BIN)