# Makeulamfile: Generate code for a single compiled ulam file (as .cpp)
#
# Given environmental variables:
#
#   ULAM_SRC_DIR   : Where the .cpp is located
#   ULAM_BLD_DIR   : Where to place compiled intermediates
#   ULAM_BIN_DIR   : Where to place linked programs
#   ULAM_CPP_FILE  : File to compile inside ULAM_SRC_DIR
#   MFM_ROOT_DIR   : The root of the MFM tree
#   USE_PATH       : Search path to set
#
# Targets:
#   ulam_cppcompile

COMPONENTNAME:=ulam
MFM_TARGET:=linux
COMMANDS:=1
ifdef MFM_ROOT_DIR
 export PATH=$(USE_PATH)
 BASEDIR:=$(MFM_ROOT_DIR)
 include $(BASEDIR)/config/Makecommon.mk
endif

INCLUDES += -I $(BASEDIR)/src/core/include -I $(ULAM_SRC_DIR)/../include

#CPPFLAGS += -fdiagnostics-color=never

PHONY_TARGETS:=ulam_cppcompile ulam_checkvar

ULAM_OBJ_FILE := $(patsubst %.cpp,$(ULAM_BLD_DIR)/%.o,$(ULAM_CPP_FILE))
#$(info "UCF($(ULAM_CPP_FILE))")
#$(info "UOF($(ULAM_OBJ_FILE))")
#$(info "USD($(ULAM_SRC_DIR))")
#$(info "MRD($(MFM_ROOT_DIR))")

ulam_cppcompile:	ulam_checkvar $(ULAM_OBJ_FILE)
	@if [ ! -r "$(ULAM_SRC_DIR)/$(ULAM_CPP_FILE)" ] ; then echo "Cannot read $(ULAM_SRC_DIR)/$(ULAM_CPP_FILE)"; exit 10 ; fi
	echo "COMPILED " $(ULAM_SRC_DIR)/$(ULAM_CPP_FILE)

# Pattern rule for cpp files
$(ULAM_BLD_DIR)/%.o:	$(ULAM_SRC_DIR)/%.cpp $(ALLDEP) #$(info DJDHD $(ULAM_BLD_DIR)/%.d)
	$(GPP) $(OPTS) $(DEBUGS) $(CPPFLAGS) $(DEFINES) -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -MT"$(@:%.o=%.d)" -o"$@" "$<"

ulam_checkvar:
	@if [ ! -d "$(ULAM_SRC_DIR)" ] ; then echo "ULAM_SRC_DIR='$(ULAM_SRC_DIR)' is not a dir"; exit 11 ; fi
	@if [ ! -d "$(ULAM_BLD_DIR)" ] ; then echo "ULAM_BLD_DIR='$(ULAM_BLD_DIR)' is not a dir"; exit 12 ; fi
	@if [ ! -d "$(ULAM_BIN_DIR)" ] ; then echo "ULAM_BIN_DIR='$(ULAM_BIN_DIR)' is not a dir"; exit 13 ; fi
	@if [ ! -d "$(MFM_ROOT_DIR)" ] ; then echo "MFM_ROOT_DIR='$(MFM_ROOT_DIR)' is not a dir"; exit 14 ; fi

LIBS += -L $(ULAM_DIR)
LIBS += -l mfmulam

.PHONY:	$(PHONY_TARGETS)
