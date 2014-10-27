# Who we are
COMPONENTNAME:=mfmsim

# Where's the top
BASEDIR:=../../..

# What we need to build
INCLUDES += -I $(BASEDIR)/src/core/include -I $(BASEDIR)/src/elements/include -I $(BASEDIR)/src/sim/include -I $(BASEDIR)/src/gui/include
INCLUDES += $(shell sdl-config --cflags)

# What we need to link
LIBS += -L $(BASEDIR)/build/core/ -L $(BASEDIR)/build/sim/ -L $(BASEDIR)/build/gui/
LIBS += -lmfmgui -lmfmsim -lmfmcore -lSDL -lSDL_ttf -lSDL_image -lpng -lm

# Do the program thing
include $(BASEDIR)/config/Makeprog.mk
