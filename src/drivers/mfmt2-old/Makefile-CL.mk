# Who we are
COMPONENTNAME:=mfmcl

# Our aliases
COMPONENTALIASES:= $(COMPONENTNAME)_s  $(COMPONENTNAME)_m $(COMPONENTNAME)_l

# Where's the top
BASEDIR:=../../..

# Depend on us too
ALLDEP += $(BASEDIR)/src/drivers/mfmc/Makefile-CL.mk

# What we need to build
override INCLUDES += -I $(BASEDIR)/src/core/include -I $(BASEDIR)/src/elements/include -I $(BASEDIR)/src/sim/include

# What we need to link
override LIBS += -L $(BASEDIR)/build/core/ -L $(BASEDIR)/build/elements/ -L $(BASEDIR)/build/sim/
override LIBS += -lmfmsim -lmfmelements -Wl,--whole-archive -lmfmcore -Wl,--no-whole-archive -lm

# Do the program thing
include $(BASEDIR)/config/Makeprog.mk
