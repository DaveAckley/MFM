# SUBDIRS are listed in 'likely precedence' order..  If we get that
# right, we ought to be able to build everything in a single pass from
# a realclean start..
SUBDIRS=platform-$(MFM_TARGET) core elements sim gui test drivers

.PHONY:	$(SUBDIRS) all clean realclean

all:	$(SUBDIRS)

clean:	$(SUBDIRS)
	@rm -f *~

realclean:	clean $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
