# This makefile to build programs from a bunch of libraries

BINDIR:=$(BASEDIR)/bin

all:	program

# Our ultimate goal
program:	$(BINDIR)/$(COMPONENTNAME)
	@cd $(BINDIR); for i in $(COMPONENTALIASES); do ln -f -s $(COMPONENTNAME) $$i; done

# Get rules to build our main.o
include $(BASEDIR)/config/Makelib.mk

# We don't have functioning interlibrary dependency tracking because
# we're losers.  For now say that if any .a files are touched, we need
# to relink

ARCHIVES := $(wildcard $(BASEDIR)/build/*/*.a)

# Aaand don't forget to make our own archive before trying to build
# our program.  Given the above, after a 'realclean' $(ARCHIVES) is
# empty.

ARCHIVES += $(ARCHIVEPATH)

$(BUILDDIR)/$(COMPONENTNAME).o:	src/main.cpp $(ALLDEP) $(BUILDDIR)/%.d
	@$(GPP) $(OPTS) $(DEBUGS) $(CPPFLAGS) $(DEFINES) -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -MT"$(@:%.o=%.d)" -o"$@" "$<"

$(BINDIR)/$(COMPONENTNAME):	$(BUILDDIR)/$(COMPONENTNAME).o $(ALLDEP) $(ARCHIVES)
	@mkdir -p $(BINDIR)
	@$(GPP) $(LDFLAGS) $(BUILDDIR)/$(COMPONENTNAME).o $(LIBS) -o $@
	@echo Made $@
