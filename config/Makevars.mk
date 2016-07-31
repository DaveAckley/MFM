# All directories mentioned here are relative to the project root
ifndef BASEDIR
  $(error BASEDIR should be defined before Makevars.mk is included)
endif
ifndef COMPONENTNAME
  $(error COMPONENTNAME should be defined before Makevars.mk is included)
endif

# Customizing/standardizing the behavior of make
SHELL:=/bin/bash

ifndef DEBUG
  # Default to commands if optimizing too
  ifndef QUIET
    COMMANDS := 1
  endif
  OPTFLAGS += -O99
# Can't just plop in whizzo sse instruction set flags given we're
# going to be building cross-platform on launchpad..
#  OPTFLAGS += -O99 -msse4.2
#  ${info NO DEBUG SO OPTFLAGS=$(OPTFLAGS)}
else
  OPTFLAGS += -g2
  # Default to commands if debugging
  ifndef COMMANDS
    COMMANDS := 1
  endif
#  ${info DEBUG=$(DEBUG) SO OPTFLAGS=$(OPTFLAGS)}
endif

ifndef PROFILE
else
  OPTFLAGS := -pg -O99
  COMMON_LDFLAGS += -pg
  # Default to commands if profiling
  ifndef COMMANDS
    COMMANDS := 1
  endif
endif

ifndef COMMANDS
  MAKEFLAGS += --quiet
endif

ifndef SHARED_DIR
  SHARED_DIR := $(shell readlink -e $(BASEDIR))
endif

ifdef FIND_DEAD_CODE
  OPTFLAGS += -Wunreachable-code
endif

ifdef MAKE_GUI
  COMMON_CFLAGS += -DMFM_GUI_DRIVER
  COMMON_CPPFLAGS += -DMFM_GUI_DRIVER
  COMMON_LDFLAGS += -DMFM_GUI_DRIVER
endif

# Common flags: All about errors -- let's help them help us
# Also: We need pthread!
COMMON_CFLAGS+=-Wall -pedantic -Werror -Wundef -D SHARED_DIR=\"$(SHARED_DIR)\" -pthread
# not reliable enough: COMMON_CPPFLAGS+=-Wmissing-noreturn -ansi -pedantic -Wall -Werror -D SHARED_DIR=\"$(SHARED_DIR)\" -pthread
COMMON_CPPFLAGS+=-ansi -pedantic -Wall -Werror -D SHARED_DIR=\"$(SHARED_DIR)\" -pthread
COMMON_LDFLAGS+=-Wl,--fatal-warnings -pthread

# Ubuntu 12.04 needs this for clock_gettime
override LIBS+=-lrt

#### DYNAMIC LOADING
override LIBS+=-ldl

# Native tool chain
NATIVE_GCC:=gcc
NATIVE_GPP:=g++
NATIVE_GCC_CFLAGS:=$(COMMON_CFLAGS)
NATIVE_GCC_CPPFLAGS:=$(COMMON_CPPFLAGS)
NATIVE_GCC_LDFLAGS:=$(COMMON_LDFLAGS)
NATIVE_GCC_DEFINES:=
NATIVE_GCC_OPTS:=$(OPTFLAGS)
NATIVE_GCC_DEBUGS:=
NATIVE_LD:=ld

# Cross tool chain
## (FUTURE EXPANSION OPTION WE NEED TO PRESERVE)
