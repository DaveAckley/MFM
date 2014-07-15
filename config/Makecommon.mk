############ Nothing below here should need to be changed ############

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
INCLUDES+=-I include

### TOOL STUFF

ALLDEP+=$(wildcard $(BASEDIR)/config/*.mk) Makefile   # If config or local makefile changes, nuke it from orbit

ifeq ($(MFM_TARGET),linux)
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
CPPFLAGS+=$(INCLUDES)
CFLAGS+=$(INCLUDES)

### COMPONENT-SPECIFIC STUFF

BUILDDIR := $(BASEDIR)/build/$(COMPONENTNAME)
ARCHIVEBASENAME := mfm$(COMPONENTNAME)
ARCHIVENAME := lib$(ARCHIVEBASENAME).a
ARCHIVEPATH := $(BUILDDIR)/$(ARCHIVENAME)

### EXTRA DEFINES
DEFINES+=-DMFM_BUILD_DATE=$(shell date -u +0x%Y%m%d)
DEFINES+=-DMFM_BUILD_TIME=$(shell date -u +0x%H%M%S)
DEFINES+=-DMFM_BUILT_BY=$(shell whoami)
DEFINES+=-DMFM_BUILT_ON=$(shell hostname)
DEFINES+=-DMFM_VERSION_MAJOR=$(MFM_VERSION_MAJOR)
DEFINES+=-DMFM_VERSION_MINOR=$(MFM_VERSION_MINOR)
DEFINES+=-DMFM_VERSION_REV=$(MFM_VERSION_REV)

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


