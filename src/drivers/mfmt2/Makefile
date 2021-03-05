# Who we are
MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(notdir $(patsubst %/,%,$(dir $(MKFILE_PATH))))
COMPONENTNAME:=$(CURRENT_DIR)

#DEBUG:=1

# We only run on tile
MFM_TARGET:=tile

# Our aliases
#COMPONENTALIASES:= $(COMPONENTNAME)_s  $(COMPONENTNAME)_m $(COMPONENTNAME)_l

# Where's the top
BASEDIR:=../../..
T2LIB_BASE_DIR:=$(BASEDIR)/src/t2lib
T2LIB_INCLUDE_DIR:=$(T2LIB_BASE_DIR)/include

# T2 EXTERNAL FILES
#HOME_T2_DIR:=/data/ackley/PART4/code/D/
HOME_T2_DIR:=/home/t2
LKM_INCLUDE_DIR:=$(HOME_T2_DIR)/T2-12/low/lkms/include
LKM_ITC_INCLUDE_DIR:=$(HOME_T2_DIR)/T2-12/low/lkms/itc
ITC_PKT_INCLUDE_DIR:=$(HOME_T2_DIR)/T2-12/low/pru/itcio/module
T2ADC_INCLUDE_DIR:=$(HOME_T2_DIR)/T2-12/base/apps/tnv

# What we need to build
override INCLUDES += -I $(BASEDIR)/src/core/include -I $(BASEDIR)/src/elements/include -I $(BASEDIR)/src/sim/include -I $(BASEDIR)/src/gui/include
override INCLUDES += -I $(T2LIB_INCLUDE_DIR)
override INCLUDES += -I $(LKM_INCLUDE_DIR)
override INCLUDES += -I $(LKM_ITC_INCLUDE_DIR)
override INCLUDES += -I $(ITC_PKT_INCLUDE_DIR)
override INCLUDES += -I $(T2ADC_INCLUDE_DIR)
override INCLUDES += $(shell sdl-config --cflags)
override DEFINES += -D MFM_GUI_DRIVER

# What we need to link
override LIBS += -L $(BASEDIR)/build/$(COMPONENTNAME) -L $(BASEDIR)/build/core/ -L $(BASEDIR)/build/elements/ -L $(BASEDIR)/build/sim/ -L $(BASEDIR)/build/gui/ -L $(T2ADC_INCLUDE_DIR)
override LIBS += -L $(BASEDIR)/build/t2lib
override LIBS += -lmfm$(COMPONENTNAME) -lmfmt2lib -lt2adc -lmfmgui -lmfmsim -Wl,--whole-archive -lmfmcore -Wl,--no-whole-archive -lSDL -lSDL_ttf -lSDL_image -lpng -lm

# Grr need C99 stuff for LKM so can't have -pedantic here
# Grr although including Makevars now leads to duplicated flags

include $(BASEDIR)/config/Makevars.mk
COMMON_CPPFLAGS+=-std=c++11 
COMMON_CPPFLAGS+=-Wno-psabi
#202103050703 Next line imported from config/Makevars.mk
COMMON_CPPFLAGS+=-Wno-error=restrict -Wno-restrict
override COMMON_CPPFLAGS:=$(filter-out -pedantic,$(COMMON_CPPFLAGS))

# Add dependencies to make window config
WDWCFG_DIR:=$(BASEDIR)/res/mfmt2
WDWCFG_BIN:=$(WDWCFG_DIR)/wconfig.txt
WDWCFG_SRC:=./wconfig.pl

# Also build the trace file weaver
WEAVER_NAME:=Weaver
WEAVER_SRC:=src/$(WEAVER_NAME).cpp
WEAVER_BIN:=$(BASEDIR)/bin/$(WEAVER_NAME)

all:	$(WDWCFG_BIN)  $(WEAVER_BIN)

$(WDWCFG_BIN):	$(WDWCFG_SRC)
	@echo REBUILDING $@
	@mkdir -p $(WDWCFG_DIR)
	$(WDWCFG_SRC) > $@

# Do the program thing
include $(BASEDIR)/config/Makeprog.mk

$(BUILDDIR)/$(WEAVER_NAME).o:	$(WEAVER_SRC) $(ALLDEP) $(BUILDDIR)/%.d
	$(GPP) $(OPTS) $(DEBUGS) $(CPPFLAGS) $(DEFINES) -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -MT"$(@:%.o=%.d)" -o"$@" "$<"

$(WEAVER_BIN):	program $(BUILDDIR)/$(WEAVER_NAME).o $(ALLDEP) $(ARCHIVES)
	@mkdir -p $(BINDIR)
	@$(GPP) $(LDFLAGS) $(BUILDDIR)/$(WEAVER_NAME).o $(LIBS) -lpanel -lncurses -o $@
	@echo Made $@
