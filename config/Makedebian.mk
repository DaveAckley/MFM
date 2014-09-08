# Extra stuff for debian, only on 'make install'
DEB_PROGRAMS_TO_INSTALL += mfmcl mfmcl_s mfmcl_m mfmcl_l
DEB_PROGRAMS_TO_INSTALL += mfms mfms_s mfms_m mfms_l
DEB_PROGRAMS_TO_INSTALL += mfmtest
DEB_PROGRAMS_PATHS_TO_INSTALL := $(DEB_PROGRAMS_TO_INSTALL:%=bin/%)
DEB_MFM_BINDIR := $(DESTDIR)/usr/bin
DEB_MFM_RESDIR := $(DESTDIR)/usr/share/mfm/res

# We're recursing rather than depending on 'all' so that the
# $(PLATFORMS) mechanism doesn't need to know about install.
install:	FORCE
	COMMANDS=1 make -k all
	mkdir -p $(DEB_MFM_BINDIR)
	cp -a $(DEB_PROGRAMS_PATHS_TO_INSTALL) $(DEB_MFM_BINDIR)
	mkdir -p $(DEB_MFM_RESDIR)
	cp -r res/fonts $(DEB_MFM_RESDIR)
	cp -r res/images $(DEB_MFM_RESDIR)
	cp -r res/elements $(DEB_MFM_RESDIR)
	# MAN AND DOC?

version:	FORCE
	@echo $(MFM_VERSION_NUMBER)
