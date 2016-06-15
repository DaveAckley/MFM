# SUBDIRS are listed in 'likely precedence' order..  If we get that
# right, we ought to be able to build everything in a single pass from
# a realclean start..
#
# (Note that images/ is not built automatically.  We're going to
# commit the results in res/images instead, since the image build
# stuff is rickety and depends on more tools.)
SUBDIRS=platform-$(MFM_TARGET) core elements sim gui test drivers

.PHONY:	$(SUBDIRS) all clean realclean

all:	$(SUBDIRS)

clean:	$(SUBDIRS)
	@rm -f *~

realclean:	clean $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
