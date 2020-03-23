# Who we are
MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(notdir $(patsubst %/,%,$(dir $(MKFILE_PATH))))
COMPONENTNAME:=$(CURRENT_DIR)

DEBUG:=1

# We only run on tile
MFM_TARGET:=tile

# Our aliases
#COMPONENTALIASES:= $(COMPONENTNAME)_s  $(COMPONENTNAME)_m $(COMPONENTNAME)_l

# Where's the top
BASEDIR:=../../..

# T2 EXTERNAL FILES
LKM_INCLUDE_DIR:=/home/t2/T2-12/lkms/include
LKM_ITC_INCLUDE_DIR:=/home/t2/T2-12/lkms/itc
ITC_PKT_INCLUDE_DIR:=/home/t2/T2-12/pru/itcio/module


# What we need to build
override INCLUDES += -I $(BASEDIR)/src/core/include -I $(BASEDIR)/src/elements/include -I $(BASEDIR)/src/sim/include -I $(BASEDIR)/src/gui/include
override INCLUDES += -I $(LKM_INCLUDE_DIR)
override INCLUDES += -I $(LKM_ITC_INCLUDE_DIR)
override INCLUDES += -I $(ITC_PKT_INCLUDE_DIR)
override INCLUDES += $(shell sdl-config --cflags)
override DEFINES += -D MFM_GUI_DRIVER

# What we need to link
override LIBS += -L $(BASEDIR)/build/$(COMPONENTNAME) -L $(BASEDIR)/build/core/ -L $(BASEDIR)/build/elements/ -L $(BASEDIR)/build/sim/ -L $(BASEDIR)/build/gui/
override LIBS += -lmfm$(COMPONENTNAME) -lmfmgui -lmfmsim -Wl,--whole-archive -lmfmcore -Wl,--no-whole-archive -lSDL -lSDL_ttf -lSDL_image -lpng -lm

# Grr need C99 stuff for LKM so can't have -pedantic here
# Grr although including Makevars now leads to duplicated flags

include $(BASEDIR)/config/Makevars.mk
COMMON_CPPFLAGS+=-std=c++11
override COMMON_CPPFLAGS:=$(filter-out -pedantic,$(COMMON_CPPFLAGS))

# Do the program thing
include $(BASEDIR)/config/Makeprog.mk
