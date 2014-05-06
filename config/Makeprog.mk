# This makefile to build programs from a bunch of libraries

BINDIR:=$(BASEDIR)/bin

# Our ultimate goal
program:	$(BINDIR)/$(COMPONENTNAME)

# Get rules to build our main.o
include $(BASEDIR)/config/Makelib.mk

# We don't have functioning interlibrary dependency tracking because
# we're losers.  For now say that if any .a files are touched, we need
# to relink

ARCHIVES := $(wildcard $(BASEDIR)/build/*/*.a)

$(BINDIR)/$(COMPONENTNAME):	$(BUILDDIR)/main.o $(ALLDEP) $(ARCHIVES)
	mkdir -p $(BINDIR)
	$(GPP) $(LDFLAGS) $(BUILDDIR)/main.o $(LIBS) -o $@
