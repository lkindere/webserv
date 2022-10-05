CXX = c++
CXXFLAGS = -std=c++98 -Wall -Werror -Wextra -I $(INCDIR)

LIB = $(LIBDIR)/libconfigparser.a
SRCDIR = ./src
BUILDDIR = ./build
LIBDIR = $(BUILDDIR)/lib
OBJDIR = $(BUILDDIR)/obj
INCDIR = ./inc/
EXTENSION = .cpp

SOURCES := $(wildcard $(SRCDIR)/*$(EXTENSION))
OBJECTS := $(patsubst $(SRCDIR)/%$(EXTENSION),$(OBJDIR)/%.o,$(SOURCES))
DEPENDS := $(patsubst $(SRCDIR)/%$(EXTENSION),$(BUILDDIR)%.d,$(SOURCES))

all: $(LIB)
	

$(LIB): $(OBJECTS)
	@mkdir -p $(dir $(LIB))
	ar -r $(LIB) $(OBJECTS)
	cp $(LIB) ../server/libs/$(notdir $(LIB))

-include $(DEPENDS)

$(OBJDIR)/%.o: $(SRCDIR)/%$(EXTENSION) Makefile
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@
	@mv $(@:.o=.d) $(BUILDDIR)

# Cleaning Rules #

.PHONY: clean
clean:
	$(RM) $(OBJECTS) $(DEPENDS)
	$(RM) -r $(BUILDDIR)

.PHONY: fclean
fclean: clean
	$(RM) $(LIB)

.PHONY: re
re: fclean all	

.PHONY: get_lib_name
get_lib_name:
	@echo $(LIB)

