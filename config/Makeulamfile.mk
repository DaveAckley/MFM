# Makeulamfile: Generate code for a single compiled ulam file (as .cpp)
#
# Given environmental variables:
#
#   ULAM_CPPFLAGS  : Additional compilation flags supplied by ulam
#   ULAM_SRC_DIR   : Where the .cpp is located
#   ULAM_BLD_DIR   : Where to place compiled intermediates
#   ULAM_BIN_DIR   : Where to place linked programs
#   ULAM_CPP_FILE  : File to compile inside ULAM_SRC_DIR
#   MFM_ROOT_DIR   : The root of the MFM tree
#   USE_PATH       : Search path to set
#
# Targets:
#   ulam_cppcompile (ULAM_CPP_FILE is compiled ulam element)
#   ulam_program    (ULAM_CPP_FILE is NAMEOFPROGRAM.cpp)

COMPONENTNAME:=ulam

# Find our PLATFORM
include config/Makeplatform.mk
MFM_TARGET:=$(PLATFORM)

#COMMANDS:=1
#DEBUG:=1

dodo:=$(shell printenv)
ifdef MFM_ROOT_DIR
 export PATH=$(USE_PATH)
 BASEDIR:=$(MFM_ROOT_DIR)
 include $(BASEDIR)/config/Makecommon.mk
endif

# (See 'EXTRA DEFINES' in Makecommon.mk.  And weep for the humanity.)
override INCLUDES += -I $(BASEDIR)/src/core/include -I $(ULAM_SRC_DIR)/../include
override CPPFLAGS += $(ULAM_CPPFLAGS)
#$(info "ULAMICF($(CPPFLAGS))")

#DEBUGS += -g2

PHONY_TARGETS:=ulam_program ulam_cppcompile ulam_checkvar

ULAM_OBJ_FILE := $(patsubst %.cpp,$(ULAM_BLD_DIR)/%.o,$(ULAM_CPP_FILE))
ULAM_CUSTOM_OBJ_FILE := $(ULAM_BLD_DIR)/UlamCustomElements.o
ULAM_TEST_DRIVER_OBJ_FILE := $(ULAM_BLD_DIR)/testdriver.o
ULAM_EXE_FILE := $(patsubst %.cpp,$(ULAM_BIN_DIR)/%,$(ULAM_CPP_FILE))
#$(info "UXF($(ULAM_EXE_FILE))")
#$(info "UCF($(ULAM_CPP_FILE))")
#$(info "UOF($(ULAM_OBJ_FILE))")
#$(info "USD($(ULAM_SRC_DIR))")
#$(info "MRD($(MFM_ROOT_DIR))")

override ELEMENT_PIC_CPPFLAGS := -shared -fPIC -DELEMENT_PLUG_IN

ulam_program:	$(ULAM_EXE_FILE)

ulam_cppcompile:	ulam_checkvar $(ULAM_OBJ_FILE)
	@if [ ! -r "$(ULAM_SRC_DIR)/$(ULAM_CPP_FILE)" ] ; then \
	  echo "Cannot read $(ULAM_SRC_DIR)/$(ULAM_CPP_FILE)"; exit 10 ; fi
	echo "COMPILED " $(ULAM_SRC_DIR)/$(ULAM_CPP_FILE)

# Pattern rule for cpp files (matches $(ULAM_OBJ_FILE))
$(ULAM_BLD_DIR)/%.o:	$(ULAM_SRC_DIR)/%.cpp $(ALLDEP) #$(info DJDHD $(ULAM_BLD_DIR)/%.d)
	$(GPP) $(ELEMENT_PIC_CPPFLAGS) $(OPTS) $(DEBUGS) $(CPPFLAGS) $(DEFINES) -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -MT"$(@:%.o=%.d)" -o"$@" "$<"

# Rule for linking ulam programs
$(ULAM_EXE_FILE):	 $(ULAM_OBJ_FILE)
	$(GPP) $(LDFLAGS) $(ULAM_OBJ_FILE) $(ULAM_CUSTOM_OBJ_FILE) $(ULAM_TEST_DRIVER_OBJ_FILE) $(LIBS) -o $@

ulam_checkvar:
	@if [ ! -d "$(ULAM_SRC_DIR)" ] ; then echo "ULAM_SRC_DIR='$(ULAM_SRC_DIR)' is not a dir"; exit 11 ; fi
	@if [ ! -d "$(ULAM_BLD_DIR)" ] ; then echo "ULAM_BLD_DIR='$(ULAM_BLD_DIR)' is not a dir"; exit 12 ; fi
	@if [ ! -d "$(ULAM_BIN_DIR)" ] ; then echo "ULAM_BIN_DIR='$(ULAM_BIN_DIR)' is not a dir"; exit 13 ; fi
	@if [ ! -d "$(MFM_ROOT_DIR)" ] ; then echo "MFM_ROOT_DIR='$(MFM_ROOT_DIR)' is not a dir"; exit 14 ; fi

#LIBS := -L $(ULAM_BLD_DIR) -l culam $(LIBS)

override LIBS += -L $(BASEDIR)/build/core -l mfmcore

.PHONY:	$(PHONY_TARGETS)
