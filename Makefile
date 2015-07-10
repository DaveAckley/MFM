# For now, we'll assume we're building on linux only
PLATFORMS=linux

####
# Variables exported to submakes

export ULAM_CUSTOM_ELEMENTS

# Try to make a build timestamp just once at the top, so everybody
# coming out of this build agrees on it.
TOP_MFM_BUILD_DATE:=$(shell date -u +0x%Y%m%d)
TOP_MFM_BUILD_TIME:=$(shell date -u +0x%H%M%S)
export TOP_MFM_BUILD_DATE
export TOP_MFM_BUILD_TIME

export BASEDIR=$(CURDIR)

.PHONY:	$(PLATFORMS) all clean realclean tar

sim:	PLATFORMS:=sim

all:	$(PLATFORMS)

export MFM_VERSION_NUMBER
doc:	FORCE
	mkdir -p doc/ref
	doxygen

clean:  $(PLATFORMS)

realclean:  $(PLATFORMS)
	rm -f bin/*
	rm -f res/elements/*.so
	rm -rf build/
	rm -rf doc/ref

include config/Makeversion.mk
TAR_EXCLUDES+=--exclude=tools --exclude=*~ --exclude=.git --exclude=doc/internal --exclude=spikes --exclude-backups
tar:	FORCE
	make realclean
	PWD=`pwd`;BASE=`basename $$PWD`;cd ..;tar cvzf mfm-$(MFM_VERSION_NUMBER).tgz $(TAR_EXCLUDES) $$BASE

identify:	FORCE
	@echo "MFMsim $(MFM_VERSION_NUMBER)"

include config/Makedebian.mk

include config/Makeulamfile.mk

# Pass each entry in PLATFORMS down as a target
$(PLATFORMS):
	export MFM_TARGET=$@;$(MAKE) -C src $(MAKECMDGOALS)

.PHONY:	FORCE
