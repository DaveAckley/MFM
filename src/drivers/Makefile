# SUBDIRS here are expected to be independent of each other
SUBDIRS= mfmc mfmtest # mfmc mfmsim mfmdha mfmbigtile mfmcity #mfmheadless

.PHONY:	$(SUBDIRS) all clean realclean

all:	$(SUBDIRS)

clean:	$(SUBDIRS)
	@rm -f *~

realclean:	clean $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
