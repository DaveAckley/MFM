# Who we are
COMPONENTNAME:=mfmdha

# Where's the top
BASEDIR:=../../..

# What we need to build
override INCLUDES += -I $(BASEDIR)/src/core/include -I $(BASEDIR)/src/elements/include -I $(BASEDIR)/src/sim/include -I $(BASEDIR)/src/gui/include
override INCLUDES += $(shell sdl-config --cflags)

# What we need to link
override LIBS += -L $(BASEDIR)/build/core/ -L $(BASEDIR)/build/sim/ -L $(BASEDIR)/build/gui/
override LIBS += -lmfmgui -lmfmsim -lmfmcore -lSDL_ttf -lSDL_image -lSDL -lpng -lm

# Do the program thing
include $(BASEDIR)/config/Makeprog.mk

#### TESTING: Build $(BINDIR)/qbar.so for dlopen

## Main executable needs -rdynamic to expose its symbols to dlopen'd stuff
LDFLAGS += -rdynamic

# Add dependency to make the plugin
$(BINDIR)/$(COMPONENTNAME):	$(BINDIR)/Element_MQBar-Plugin.so

$(BINDIR)/Element_MQBar-Plugin.so:	src/Element_MQBar.cpp include/Element_MQBar.h
	$(GPP) -shared -fPIC -DELEMENT_PLUG_IN $(OPTS) $(DEBUGS) $(CPPFLAGS) $(DEFINES) -o"$@" "$<"
