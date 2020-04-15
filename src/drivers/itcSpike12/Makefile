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
T2ADC_INCLUDE_DIR:=/home/t2/T2-12/apps/tnv

# What we need to build
override INCLUDES += -I $(BASEDIR)/src/core/include -I $(BASEDIR)/src/elements/include -I $(BASEDIR)/src/sim/include -I $(BASEDIR)/src/gui/include
override INCLUDES += -I $(LKM_INCLUDE_DIR)
override INCLUDES += -I $(LKM_ITC_INCLUDE_DIR)
override INCLUDES += -I $(ITC_PKT_INCLUDE_DIR)
override INCLUDES += -I $(T2ADC_INCLUDE_DIR)
override INCLUDES += $(shell sdl-config --cflags)
override DEFINES += -D MFM_GUI_DRIVER

# What we need to link
override LIBS += -L $(BASEDIR)/build/$(COMPONENTNAME) -L $(BASEDIR)/build/core/ -L $(BASEDIR)/build/elements/ -L $(BASEDIR)/build/sim/ -L $(BASEDIR)/build/gui/ -L $(T2ADC_INCLUDE_DIR)
override LIBS += -lmfm$(COMPONENTNAME) -lt2adc -lmfmgui -lmfmsim -Wl,--whole-archive -lmfmcore -Wl,--no-whole-archive -lSDL -lSDL_ttf -lSDL_image -lpng -lm

# Grr need C99 stuff for LKM so can't have -pedantic here
# Grr although including Makevars now leads to duplicated flags

include $(BASEDIR)/config/Makevars.mk
COMMON_CPPFLAGS+=-std=c++11
override COMMON_CPPFLAGS:=$(filter-out -pedantic,$(COMMON_CPPFLAGS))

# Add dependencies to make window config
WDWCFG_BIN:=wconfig.txt
WDWCFG_SRC:=./wconfig.pl

all:	$(WDWCFG_BIN)

$(WDWCFG_BIN):	$(WDWCFG_SRC)
	@echo REBUILDING $@
	$(WDWCFG_SRC) > $@

# Do the program thing
include $(BASEDIR)/config/Makeprog.mk
