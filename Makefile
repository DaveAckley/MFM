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
	rm -rf build/
	rm -rf doc/ref

TAR_EXCLUDES+=--exclude=tools --exclude=*~ --exclude=.git --exclude=doc/internal --exclude=spikes --exclude-backups
tar:	FORCE
	make realclean
	cd ..;tar cvzf MFMv2.tgz $(TAR_EXCLUDES) MFMv2

# Pass each entry in PLATFORMS down as a target
$(PLATFORMS):
	export MFM_TARGET=$@;$(MAKE) -C src $(MAKECMDGOALS)

.PHONY:	FORCE
