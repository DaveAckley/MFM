# Who we are
COMPONENTNAME:=mfmdha

# Where's the top
BASEDIR:=../../..

# What we need to build
INCLUDES += -I $(BASEDIR)/src/core/include -I $(BASEDIR)/src/elements/include -I $(BASEDIR)/src/sim/include -I $(BASEDIR)/src/gui/include
INCLUDES += $(shell sdl-config --cflags)

# What we need to link
LIBS += -L $(BASEDIR)/build/core/ -L $(BASEDIR)/build/sim/ -L $(BASEDIR)/build/gui/
LIBS += -lmfmgui -lmfmsim -lmfmcore -lSDL_ttf -lSDL_image -lSDL -lpng -lm -ldl

# Do the program thing
include $(BASEDIR)/config/Makeprog.mk

#### TESTING: Build $(BINDIR)/qbar.so for dlopen

## Main executable needs -rdynamic to expose its symbols to dlopen'd stuff
LDFLAGS += -rdynamic

# Add dependency to make the plugin
$(BINDIR)/$(COMPONENTNAME):	$(BINDIR)/Element_MQBar-Plugin.so

$(BINDIR)/Element_MQBar-Plugin.so:	src/Element_MQBar.cpp include/Element_MQBar.h include/LocalConfig.h
	$(GPP) -shared -fPIC -DELEMENT_PLUG_IN $(OPTS) $(DEBUGS) $(CPPFLAGS) $(DEFINES) -o"$@" "$<"
