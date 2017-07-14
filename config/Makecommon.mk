############ Nothing below here should need to be changed ############

# Make a build timestamp just once at the top, so everybody in this
# build agrees on it (so we avoid spurious element UUID mismatches)
ifndef TOP_MFM_BUILD_DATE
 export TOP_MFM_BUILD_DATE:=$(shell date -u +0x%Y%m%d)
 export TOP_MFM_BUILD_TIME:=$(shell date -u +0x%H%M%S)
endif

include $(BASEDIR)/config/Makeversion.mk
include $(BASEDIR)/config/Makevars.mk

#####
# PLATFORM-SPECIFIC MAKE CONFIGURATION
include $(BASEDIR)/src/platform-$(MFM_TARGET)/MakePlatform.mk

#####
# COMPONENT DIRECTORY LAYOUT
#
# For a COMPONENTNAME foo, we expect to find the following structure:
# foo/Makefile        Makefile to build component foo
# foo/src/            All foo-specific source codes (which other components can't see)
# foo/include/        All foo-specific header files (which other components can see)
#

# (And now the INCLUDES are dying -- but only on the launchpad build,
# not when I build locally! :( Gah!  See 'EXTRA DEFINES', below.  Drop
# the override bomb!)
override INCLUDES+=$(EXTERNAL_INCLUDES)
override INCLUDES+=-I include

### EXTRA DEFINES
# Argh, DEFINES is seen as user-set and make is ignoring our changes.
# We're completely lost in the weeds!  Drop the bomb!  Drop the
# override bomb!
override DEFINES+= $(EXTERNAL_DEFINES)
override DEFINES+=-DMFM_BUILD_DATE=$(TOP_MFM_BUILD_DATE)
override DEFINES+=-DMFM_BUILD_TIME=$(TOP_MFM_BUILD_TIME)
override DEFINES+=-DMFM_BUILT_BY=$(shell whoami)
override DEFINES+=-DMFM_BUILT_ON=$(shell hostname)
override DEFINES+=-DMFM_VERSION_MAJOR=$(MFM_VERSION_MAJOR)
override DEFINES+=-DMFM_VERSION_MINOR=$(MFM_VERSION_MINOR)
override DEFINES+=-DMFM_VERSION_REV=$(MFM_VERSION_REV)
override DEFINES+=-DMFM_TREE_VERSION="$(MFM_TREE_VERSION)"
override DEFINES+=-DDEBIAN_PACKAGE_NAME="$(DEBIAN_PACKAGE_NAME)"
override DEFINES+=-DMAGIC_DEBIAN_PACKAGE_VERSION="$(MAGIC_DEBIAN_PACKAGE_VERSION)"

# (AAAaand, we are now adopting a preemptive first strike policy!  See
# 'EXTRA DEFINES', above.  Drop the override bomb!)
override LIBS+=$(EXTERNAL_LIBS)

### TOOL STUFF

ALLDEP+=$(wildcard $(BASEDIR)/config/*.mk) Makefile   # If config or local makefile changes, nuke it from orbit

IS_NATIVE=no
ifeq ($(MFM_TARGET),linux)
  IS_NATIVE:=yes
endif
ifeq ($(MFM_TARGET),tile)
  IS_NATIVE:=yes
endif
ifeq ($(IS_NATIVE),yes)
  CFLAGS+=$(NATIVE_GCC_CFLAGS)
  CPPFLAGS+=$(NATIVE_GCC_CPPFLAGS)
  LDFLAGS+=$(NATIVE_GCC_LDFLAGS)
  DEFINES+=$(NATIVE_GCC_DEFINES)
  OPTS+=$(NATIVE_GCC_OPTS)
  DEBUGS+=$(NATIVE_GCC_DEBUGS)
  GCC:=$(NATIVE_GCC)
  GPP:=$(NATIVE_GPP)
  LD:=$(NATIVE_LD)
  OBJCOPY:=$(NATIVE_OBJCOPY)
  ARCH_CODE:=$(NATIVE_ARCH_CODE)
else
  CFLAGS+=$(CROSS_GCC_CFLAGS)
  CPPFLAGS+=$(CROSS_GCC_CPPFLAGS)
  LDFLAGS+=$(CROSS_GCC_LDFLAGS)
  DEFINES+=$(CROSS_GCC_DEFINES)
  OPTS+=$(CROSS_GCC_OPTS)
  DEBUGS+=$(CROSS_GCC_DEBUGS)
  GCC:=$(CROSS_GCC)
  GPP:=$(CROSS_GPP)
  LD:=$(CROSS_LD)
  OBJCOPY:=$(CROSS_OBJCOPY)
  ARCH_CODE:=$(CROSS_ARCH_CODE)
endif
override CPPFLAGS+=$(INCLUDES)
override CFLAGS+=$(INCLUDES)

### DYNAMIC LOADING
LDFLAGS+=-rdynamic

### COMPONENT-SPECIFIC STUFF

ifndef BUILDDIR
	BUILDDIR := $(BASEDIR)/build/$(COMPONENTNAME)
endif
ARCHIVEBASENAME := mfm$(COMPONENTNAME)
ARCHIVENAME := lib$(ARCHIVEBASENAME).a
ARCHIVEPATH := $(BUILDDIR)/$(ARCHIVENAME)

### PATTERN RULES

# Pattern rule for c files
$(BUILDDIR)/%.o:	src/%.c $(ALLDEP) $(BUILDDIR)/%.d
	$(GCC) $(OPTS) $(DEBUGS) $(CFLAGS) $(DEFINES) -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -MT"$(@:%.o=%.d)" -o"$@" "$<"

# Pattern rule for cpp files
$(BUILDDIR)/%.o:	src/%.cpp $(ALLDEP) $(BUILDDIR)/%.d
	$(GPP) $(OPTS) $(DEBUGS) $(CPPFLAGS) $(DEFINES) -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -MT"$(@:%.o=%.d)" -o"$@" "$<"

# Pattern rule for S files
$(BUILDDIR)/%.o:	src/%.S $(ALLDEP) $(BUILDDIR)/%.d
	$(GCC) $(OPTS) $(DEBUGS) $(CFLAGS) $(DEFINES) -c -o"$@" "$<"
