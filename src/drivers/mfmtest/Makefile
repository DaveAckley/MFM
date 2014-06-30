# Who we are
COMPONENTNAME:=mfmtest

# Where's the top
BASEDIR:=../../..

# What we need to build
INCLUDES += -I $(BASEDIR)/src/core/include -I $(BASEDIR)/src/elements/include -I $(BASEDIR)/src/sim/include -I $(BASEDIR)/src/gui/include -I $(BASEDIR)/src/test/include

# What we need to link
LIBS += -L $(BASEDIR)/build/core/ -L $(BASEDIR)/build/test/ -L $(BASEDIR)/build/sim/ -L $(BASEDIR)/build/gui/
LIBS += -lmfmtest -lmfmsim -lmfmcore -lmfmgui

# Do the program thing
include $(BASEDIR)/config/Makeprog.mk
