# Who we are
COMPONENTNAME:=mfmheadless

# Where's the top
BASEDIR:=../../..

# What we need to build
INCLUDES += -I $(BASEDIR)/src/core/include -I $(BASEDIR)/src/sim/include

# What we need to link
LIBS += -L $(BASEDIR)/build/core/ -L $(BASEDIR)/build/sim/
LIBS += -lmfmsim -lmfmcore -lSDL -lSDL_ttf -lpng -lm -lpthread

# Do the program thing
include $(BASEDIR)/config/Makeprog.mk
