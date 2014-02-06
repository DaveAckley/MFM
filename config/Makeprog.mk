# This makefile to build programs from a bunch of libraries

BINDIR:=$(BASEDIR)/bin

# Our ultimate goal
program:	$(BINDIR)/$(COMPONENTNAME)

# Get rules to build our main.o
include $(BASEDIR)/config/Makelib.mk

$(BINDIR)/$(COMPONENTNAME):	$(BUILDDIR)/main.o $(ALLDEP)
	$(GPP) $(BUILDDIR)/main.o $(LIBS) -o $@
