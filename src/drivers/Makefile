# Here to dish the TARGET out to the various drivers
ifndef TARGET
  $(error TARGET not defined)
endif
export TARGET

# SUBDIRS here are expected to be independent of each other
SUBDIRS=mfmtest mfmsim

.PHONY:	$(SUBDIRS) all clean realclean

all:	$(SUBDIRS)

clean:	$(SUBDIRS)
	@rm -f *~	

realclean:	clean $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
