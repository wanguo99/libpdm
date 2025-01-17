#Makefile
# SPDX-License-Identifier: GPL-2.0
VERSION = 1
PATCHLEVEL = 0
SUBLEVEL = 0
EXTRAVERSION =
NAME = PDM

# ---------------------------------------------------------------------------
# *DOCUMENTATION*
# To see a list of typical targets execute "make help"
# More info can be located in ./README
# Comments in this file are targeted only to the developer, do not
# expect to learn how to build the kernel reading this file.
# ---------------------------------------------------------------------------

ifeq ($(filter output-sync,$(.FEATURES)),)
$(error GNU Make >= 4.0 is required. Your Make version is $(MAKE_VERSION))
endif

$(if $(filter __%, $(MAKECMDGOALS)), \
	$(error targets prefixed with '__' are only for internal use))

# ---------------------------------------------------------------------------
# That's our default target when none is given on the command line
# ---------------------------------------------------------------------------
PHONY := __all
__all:

this-makefile := $(lastword $(MAKEFILE_LIST))
m_abs_srctree := $(realpath $(dir $(this-makefile)))
m_abs_objtree := $(CURDIR)


# ---------------------------------------------------------------------------
# Mbuild will save output files in the current working directory.
# This does not need to match to the root of the kernel source tree.
#
# For example, you can do this:
#
#  cd /dir/to/store/output/files; make -f /dir/to/kernel/source/Makefile
#
# If you want to save output files in a different location, there are
# two syntaxes to specify it.
#
# 1) O=
# Use "make O=dir/to/store/output/files/"
#
# 2) Set MBUILD_OUTPUT
# Set the environment variable MBUILD_OUTPUT to point to the output directory.
# export MBUILD_OUTPUT=dir/to/store/output/files/; make
#
# The O= assignment takes precedence over the MBUILD_OUTPUT environment
# variable.

# Do we want to change the working directory?
# ---------------------------------------------------------------------------
ifeq ("$(origin MO)", "command line")
  MBUILD_OUTPUT := $(MO)
endif

ifneq ($(MBUILD_OUTPUT),)
$(shell mkdir -p "$(MBUILD_OUTPUT)")
m_abs_objtree := $(realpath $(MBUILD_OUTPUT))
$(if $(m_abs_objtree),,$(error failed to create output directory "$(MBUILD_OUTPUT)"))
$(shell ln -sf $(m_abs_srctree)/Kbuild $(m_abs_objtree)/Kbuild)
# $(shell ln -sf $(m_abs_srctree)/src $(m_abs_objtree)/src)
endif # ifneq ($(MBUILD_OUTPUT),)

ifeq ($(m_abs_srctree),$(m_abs_objtree))
  # building in the source tree
  m_srctree := .
  VPATH		:=
else
  m_srctree := $(m_abs_srctree)
  VPATH		:= $(m_srctree)
endif

m_objtree	:= .

export m_srctree m_objtree VPATH


__all: all

PHONY += modules
modules:
	$(info m_abs_objtree: $(m_abs_objtree))
	$(info m_abs_srctree: $(m_abs_srctree))
	$(info m_objtree: $(m_objtree))
	$(info m_srctree: $(m_srctree))
	$(MAKE) -C $(KDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M="$(m_abs_objtree)" modules

PHONY += clean
clean:
	$(MAKE) -C $(KDIR) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) M="$(m_abs_objtree)" clean
ifneq ($(m_abs_srctree),$(m_abs_objtree))
	@rm -rf $(m_abs_objtree)
endif

PHONY += all
all: modules

# Declare the contents of the PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)
