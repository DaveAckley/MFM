# Who we are
COMPONENTNAME:=mfmtest

# Where's the top
BASEDIR:=../../..

# What we need to build
INCLUDES += -I $(BASEDIR)/src/core/include -I $(BASEDIR)/src/sim/include -I $(BASEDIR)/src/test/include

# What we need to link
LIBS += -L $(BASEDIR)/build/core/ -L $(BASEDIR)/build/sim/ -L $(BASEDIR)/build/test/
LIBS += -lmfmtest -lmfmsim -lmfmcore

# Do the program thing
include $(BASEDIR)/config/Makeprog.mk
