# For now, we'll assume we're building on linux only
PLATFORMS=linux

.PHONY:	$(PLATFORMS) all clean realclean tar

sim:	PLATFORMS:=sim

all:	$(PLATFORMS)

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

include config/Makedebian.mk

# Pass each entry in PLATFORMS down as a target
$(PLATFORMS):
	export MFM_TARGET=$@;$(MAKE) -C src $(MAKECMDGOALS)

.PHONY:	FORCE
