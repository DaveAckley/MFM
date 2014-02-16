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
  OPTFLAGS += -O99
else
  OPTFLAGS += -g2
  # Default to commands if debugginb
  ifndef COMMANDS
    COMMANDS := 1
  endif	
endif


ifndef COMMANDS
  MAKEFLAGS += --quiet
endif

# Common flags
COMMON_CFLAGS:=-Wall -pedantic -Werror -Wundef     # Let's help them help us
COMMON_CPPFLAGS:=-ansi -pedantic -Wall -Werror     #   "
COMMON_LDFLAGS:=--fatal-warnings -L$(LIBDIR)       #   "  

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
