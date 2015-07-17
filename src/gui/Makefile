# Who we are
COMPONENTNAME:=gui

# Where's the top
BASEDIR:=../..

# What we need from outside
override INCLUDES += -I $(BASEDIR)/src/core/include -I $(BASEDIR)/src/elements/include -I $(BASEDIR)/src/sim/include
override INCLUDES += $(shell sdl-config --cflags)

# Do the library thing
include $(BASEDIR)/config/Makelib.mk
