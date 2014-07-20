# This makefile to build an archive file from a single directory of source
include $(BASEDIR)/config/Makecommon.mk

CSOURCES := $(wildcard src/*.c)
COBJECTS := $(patsubst src/%.c,$(BUILDDIR)/%.o,$(CSOURCES))

CPPSOURCES := $(wildcard src/*.cpp)
CPPOBJECTS := $(patsubst src/%.cpp,$(BUILDDIR)/%.o,$(CPPSOURCES))

SSOURCES := $(wildcard src/*.S)
SOBJECTS := $(patsubst src/%.S,$(BUILDDIR)/%.o,$(SSOURCES))

SOURCES := $(CSOURCES) $(CPPSOURCES) $(SSOURCES)
OBJECTS := $(COBJECTS) $(CPPOBJECTS) $(SOBJECTS)

HEADERS := $(wildcard include/*)

ALLDEP += $(HEADERS)

all:	$(ARCHIVEPATH) commondeps


commondeps:	FORCE
	mkdir -p $(BASEDIR)/res/elements

$(ARCHIVEPATH):	$(BUILDDIR) $(OBJECTS)
	$(AR) ruc "$@" $(OBJECTS)

clean:
	rm -f $(OBJECTS)

include $(BASEDIR)/config/Makedeps.mk

realclean:	clean
	rm -f $(AUTODEPS) # AUTODEPS
	rm -f $(ARCHIVEPATH)
	rm -f src/*~ include/*~ *~
	rm -f $(BUILDDIR)/*

.PHONY:	clean realclean FORCE

