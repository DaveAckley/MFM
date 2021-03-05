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

# What we need to build
override INCLUDES += -I $(BASEDIR)/src/core/include -I $(BASEDIR)/src/elements/include -I $(BASEDIR)/src/sim/include -I $(BASEDIR)/src/gui/include
override INCLUDES += -I $(T2LIB_INCLUDE_DIR)
override INCLUDES += -I $(LKM_INCLUDE_DIR)
override INCLUDES += -I $(LKM_ITC_INCLUDE_DIR)
override INCLUDES += -I $(ITC_PKT_INCLUDE_DIR)

# What we need to link
override LIBS += -L $(BASEDIR)/build/$(COMPONENTNAME) -L $(BASEDIR)/build/core/ -L $(BASEDIR)/build/elements/ -L $(BASEDIR)/build/sim/
override LIBS += -L $(BASEDIR)/build/t2lib
override LIBS += -lmfm$(COMPONENTNAME) -lmfmt2lib -lmfmsim -Wl,--whole-archive -lmfmcore -Wl,--no-whole-archive -lm

# Grr need C99 stuff for LKM so can't have -pedantic here
# Grr although including Makevars now leads to duplicated flags

include $(BASEDIR)/config/Makevars.mk
COMMON_CPPFLAGS+=-std=c++11 
COMMON_CPPFLAGS+=-Wno-psabi
override COMMON_CPPFLAGS:=$(filter-out -pedantic,$(COMMON_CPPFLAGS))

# Do the program thing
include $(BASEDIR)/config/Makeprog.mk

