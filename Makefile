# For now, we'll assume we're building the simulator only
PLATFORMS=sim

.PHONY:	$(PLATFORMS) all clean realclean

sim:	PLATFORMS:=sim

all:	$(PLATFORMS)

clean:  $(PLATFORMS)

realclean:  $(PLATFORMS)

# Pass each entry in PLATFORMS down as a target
$(PLATFORMS):
	export TARGET=$@;$(MAKE) -C src $(MAKECMDGOALS)
