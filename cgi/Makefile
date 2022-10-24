CXX = c++
CXXFLAGS = -std=c++98 -Wall -Werror -Wextra -I $(INCDIR) -I $(UTILS_INC) -I $(REQUEST_INC)

LIB = libcgi.a
SRCDIR = ./src
BUILDDIR = ./build
OBJDIR = $(BUILDDIR)/obj
INCDIR = ./inc/
EXTENSION = .cpp

UTILS_DIR = ../utils
UTILS_LIB = $(UTILS_DIR)/libcgi.a
UTILS_INC = $(UTILS_DIR)/inc/

REQUEST_DIR = ../request
REQUEST_LIB = $(REQUEST_DIR)/librequest.a
REQUEST_INC = $(REQUEST_DIR)/inc/

SOURCES := $(wildcard $(SRCDIR)/*$(EXTENSION))
OBJECTS := $(patsubst $(SRCDIR)/%$(EXTENSION),$(OBJDIR)/%.o,$(SOURCES))
DEPENDS := $(patsubst $(SRCDIR)/%$(EXTENSION),$(BUILDDIR)/%.d,$(SOURCES))

all: $(UTILS_LIB) $(REQUEST_LIB) $(LIB)
	
$(UTILS_LIB):
	@printf "$(shell tput setaf 2)Compiling Utils Library$(shell tput sgr0)\n"
	@$(MAKE) -C $(dir $@)

$(REQUEST_LIB):
	@printf "$(shell tput setaf 2)Compiling Request Library$(shell tput sgr0)\n"
	@$(MAKE) -C $(dir $@)


$(LIB): $(OBJECTS)
	@mkdir -p $(dir $(LIB))
	@printf "$(shell tput setaf 2)Compiling [$(LIB)]$(shell tput sgr0)\n"
	@ar -r $(LIB) $(OBJECTS)
	@mkdir -p ../server/libs/
	@cp $(LIB) ../server/libs/$(notdir $(LIB))

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
	@printf "$(shell tput setaf 1)Removing $(LIB)$(shell tput sgr0)\n"
	@$(RM) $(LIB)

.PHONY: re
re: fclean all	

.PHONY: get_lib_name
get_lib_name:
	@echo $(LIB)