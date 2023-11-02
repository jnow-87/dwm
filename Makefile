#
# Copyright (C) 2014 Jan Nowotsch
# Author Jan Nowotsch	<jan.nowotsch@gmail.com>
#
# Released under the terms of the GNU GPL v2.0
#



####
## init
####

# build system variables
scripts_dir := scripts/build
project_type := c
config := .config
config_tree := scripts/config
use_config_sys := y
config_ftype := Pconfig
use_coverage_sys = n
tool_deps :=

# include config
-include $(config)

# init source and build tree
default_build_tree := build/
src_dirs := core/ dmenu/

# include build system Makefile
include $(scripts_dir)/main.make


####
## flags
####

warnflags := \
	-Wall \
	-Wno-deprecated-declarations \
	-Wno-unused-function

cflags := \
	$(CFLAGS) \
	$(CONFIG_CFLAGS) \
	$(warnflags) \
	-std=c2x \
	-Os

cppflags := \
	$(CPPFLAGS) \
	$(CONFIG_CPPFLAGS) \
	-Iinclude \
	-I$(build_tree)

ldflags := \
	$(LDFLAGS) \
	$(CONFIG_LDFLAGS)

ldlibs := \
	$(LDLIOBSFLAGS) \
	$(CONFIG_LDLIBS)


####
## targets
####

## build
.PHONY: all
ifeq ($(CONFIG_BUILD_DEBUG),y)
all: cflags += -g
all: asflags += -g
all: hostcflags += -g
all: hostcxxflags += -g
all: hostasflags += -g
endif

all: $(lib) $(bin) $(hostlib) $(hostbin)

## cleanup
.PHONY: clean
clean:
	$(rm) $(filter-out $(build_tree)/$(scripts_dir),$(wildcard $(build_tree)/*))

.PHONY: distclean
distclean:
	$(rm) $(config) $(build_tree)

## install
include $(scripts_dir)/install.make

.PHONY: install
install: all
	$(call install,$(build_tree)/core/dwm,/usr/bin/)
	$(call install,$(build_tree)/dmenu/dmenu,/usr/bin/)
	$(call install,$(build_tree)/dmenu/stest,/usr/bin/)
	$(call install,dmenu/dmenurun,/usr/bin/)
	$(call install,dmenu/dmenupath,/usr/bin/)
	$(call install,man/dwm.1,/usr/local/man/man1)
	$(call install,man/dmenu.1,/usr/local/man/man1)
	$(call install,man/stest.1,/usr/local/man/man1)
	update-alternatives --install /usr/bin/x-window-manager x-window-manager /usr/bin/dwm 1
	update-alternatives --config x-window-manager

.PHONY: uninstall
uninstall:
	$(call uninstall,/usr/bin/dwm)
	$(call uninstall,/usr/bin/dmenu)
	$(call uninstall,/usr/bin/dmenurun)
	$(call uninstall,/usr/bin/dmenupath)
	$(call uninstall,/usr/bin/stest)
	$(call uninstall,/usr/local/man/man1/dwm.1)
	$(call uninstall,/usr/local/man/man1/dmenu.1)
	$(call uninstall,/usr/local/man/man1/stest.1)
