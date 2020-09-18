# Who we are
MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(notdir $(patsubst %/,%,$(dir $(MKFILE_PATH))))
COMPONENTNAME:=$(CURRENT_DIR)

# We only run on tile
MFM_TARGET:=tile

# Where's the top
BASEDIR:=../..

# What we need from outside
override INCLUDES += -I $(BASEDIR)/src/core/include -I $(BASEDIR)/src/sim/include

HOME_T2_DIR:=/home/t2
LKM_INCLUDE_DIR:=$(HOME_T2_DIR)/T2-12/lkms/include
ITC_PKT_INCLUDE_DIR:=$(HOME_T2_DIR)/T2-12/pru/itcio/module
override INCLUDES += -I $(LKM_INCLUDE_DIR)
override INCLUDES += -I $(ITC_PKT_INCLUDE_DIR)

include $(BASEDIR)/config/Makevars.mk
COMMON_CPPFLAGS+=-std=c++11 
COMMON_CPPFLAGS+=-Wno-psabi
override COMMON_CPPFLAGS:=$(filter-out -pedantic,$(COMMON_CPPFLAGS))

# Do the library thing
include $(BASEDIR)/config/Makelib.mk
