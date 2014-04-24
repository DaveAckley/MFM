# For now, we'll assume we're building on linux only
PLATFORMS=linux

.PHONY:	$(PLATFORMS) all clean realclean

sim:	PLATFORMS:=sim

all:	$(PLATFORMS)

doc:	FORCE
	mkdir -p doc/ref
	doxygen

commondeps: FORCE
	mkdir -p ~/.mfm/res/
	cp res/* ~/.mfm/res/
	chmod -R 777 ~/.mfm/

clean:  $(PLATFORMS)

realclean:  $(PLATFORMS)

# Pass each entry in PLATFORMS down as a target
$(PLATFORMS):
	export MFM_TARGET=$@;$(MAKE) -C src $(MAKECMDGOALS)

.PHONY:	FORCE
