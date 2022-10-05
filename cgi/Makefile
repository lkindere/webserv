# Targets that can be executed by this Makefile
TOPTARGETS := all clean fclean re

# Gets all Subdirectorys of cgi
SUBDIRS = $(wildcard */.)

$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS):

# Gets the Name of the binary of the current Subdir Makefile
	$(eval BIN = $(shell make -C $@ get_bin))
# Gets the cgi-bin Directory (of the server)
	$(eval CGI_BIN_DIR = $(shell make -C ../server get_cgi_bin_dir))

# Executes Makefile in current Subdirectory
	@$(MAKE) -C $@ $(MAKECMDGOALS)

# Only Copy the binary if rule all or re is called
	@if [ -z $(MAKECMDGOALS) ] || [ $(MAKECMDGOALS) = all ] || [ $(MAKECMDGOALS) = re ]; then \
		cp $(BIN) $(CGI_BIN_DIR)/$(notdir $(BIN)); \
	fi; \

.PHONY: all $(SUBDIRS)