# Who we are
COMPONENTNAME:=t2viz

# Our aliases
#COMPONENTALIASES:= $(COMPONENTNAME)_s  $(COMPONENTNAME)_m $(COMPONENTNAME)_l

# Where's the top
BASEDIR:=../../..

# What we need to build
override INCLUDES += -I $(BASEDIR)/src/core/include -I $(BASEDIR)/src/sim/include -I $(BASEDIR)/src/gui/include
override INCLUDES += $(shell sdl-config --cflags)
override DEFINES += -D MFM_GUI_DRIVER

# What we need to link
override LIBS += -L $(BASEDIR)/build/core/ -L $(BASEDIR)/build/sim/ -L $(BASEDIR)/build/gui/
override LIBS += -L $(BASEDIR)/build/$(COMPONENTNAME)
override LIBS += -lmfm$(COMPONENTNAME)
override LIBS += -lmfmgui -lmfmsim  -Wl,--whole-archive -lmfmcore -Wl,--no-whole-archive -lSDL -lSDL_ttf -lSDL_image -lpng -lm

# Do the program thing
include $(BASEDIR)/config/Makeprog.mk
