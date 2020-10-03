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
TAR_SWITCHES+=--exclude=*.o --exclude=*.d
TAR_SWITCHES+=--mtime="2008-01-02 12:34:56"
TAR_SWITCHES+=--owner=0 --group=0 --numeric-owner 

tar:	FORCE
	make realclean
	PWD=`pwd`;BASE=`basename $$PWD`;cd ..;tar cvzf mfm-$(MFM_VERSION_NUMBER).tgz $(TAR_SWITCHES) $$BASE

ifeq ($(PLATFORM),tile)
REGNUM:=0
SLOTNUM:=03
cdmd:	FORCE
	MPWD=`pwd`;BASE=`basename $$MPWD`; \
	echo $$MPWD for $$BASE; \
	pushd ..;tar cvzf $$BASE-built.tgz $(TAR_SWITCHES) $$BASE; \
	cp -f $$BASE-built.tgz /home/debian/CDMSAVE/TGZS/; \
	FN=`/home/t2/MFM/bin/mfzmake cdmake $(REGNUM) $(SLOTNUM) $$BASE $$BASE-built.tgz | \
            perl -e "while(<>) {/'([^']+)'/ && print "'$$1}'`; \
	if [ "x$$FN" = "x" ] ; then echo "Build failed" ; else  \
	echo -n "Got $$FN for $$BASE, tag = "; \
	perl -e '"'$$FN'" =~ /[^-]+-[^-]+-([[:xdigit:]]+)[.]/; print $$1' > /cdm/tags/slot$(SLOTNUM)-install-tag.dat; \
	cat /cdm/tags/slot$(SLOTNUM)-install-tag.dat; echo ;\
	cp -f $$FN /home/debian/CDMSAVE/CDMDS/; \
	fi; \
	popd

restart:	FORCE
	pkill mfmt2

endif

identify:	FORCE
	@echo "MFMsim $(MFM_VERSION_NUMBER)"

include config/Makedebian.mk

include config/Makeulamfile.mk

# Pass each entry in PLATFORMS down as a target
$(PLATFORMS):
	export MFM_TARGET=$@;$(MAKE) -C src $(MAKECMDGOALS)

.PHONY:	FORCE
