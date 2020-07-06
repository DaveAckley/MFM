SHELL = /bin/bash
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

export PLATFORM

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
TAR_SWITCHES+=--exclude=tools --exclude=*~ --exclude=.git --exclude=doc/internal --exclude=spikes --exclude-backups
TAR_SWITCHES+=--mtime="2008-01-02 12:34:56"
TAR_SWITCHES+=--owner=0 --group=0 --numeric-owner 

tar:	FORCE
	make realclean
	PWD=`pwd`;BASE=`basename $$PWD`;cd ..;tar cvzf mfm-$(MFM_VERSION_NUMBER).tgz $(TAR_SWITCHES) $$BASE

ifeq ($(PLATFORM),tile)
cdmd:	FORCE
	echo make
	MPWD=`pwd`;BASE=`basename $$MPWD`;echo $$MPWD for $$BASE;pushd ..;tar cvzf $$BASE-built.tgz $(TAR_SWITCHES) $$BASE;cp -f $$BASE-built.tgz /home/debian/CDMSAVE/TGZS/;/home/t2/MFM/bin/mfzmake make - cdmd-$$BASE.mfz $$BASE-built.tgz;cp -f cdmd-$$BASE.mfz /home/debian/CDMSAVE/CDMDS/;popd
endif

identify:	FORCE
	@echo "MFMsim $(MFM_VERSION_NUMBER)"

include config/Makedebian.mk

include config/Makeulamfile.mk

# Pass each entry in PLATFORMS down as a target
$(PLATFORMS):
	export MFM_TARGET=$@;$(MAKE) -C src $(MAKECMDGOALS)

.PHONY:	FORCE
