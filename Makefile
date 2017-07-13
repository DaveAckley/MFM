ifndef DEBIAN_PACKAGE_NAME
export DEBIAN_PACKAGE_NAME:=ulam
export MAGIC_DEBIAN_PACKAGE_VERSION:=
endif
$(info Building MFM for Debian package name: $(DEBIAN_PACKAGE_NAME))

# Find our PLATFORM
include config/Makeplatform.mk
PLATFORMS:=$(PLATFORM)

####
# Variables exported to submakes

export ULAM_CUSTOM_ELEMENTS

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
