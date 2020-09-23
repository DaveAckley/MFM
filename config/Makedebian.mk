# Extra stuff for debian, only on 'make install'
#DEB_PROGRAMS_TO_INSTALL += mfmcl mfmcl_s mfmcl_m mfmcl_l
#DEB_PROGRAMS_TO_INSTALL += mfms mfms_s mfms_m mfms_l
ifeq ($(PLATFORM), linux)
	DEB_PROGRAMS_TO_INSTALL += mfms
endif
ifeq ($(PLATFORM), tile)
	DEB_PROGRAMS_TO_INSTALL += mfmt2
endif
#DEB_PROGRAMS_TO_INSTALL += mfmtest
DEB_PROGRAMS_TO_INSTALL += mfzmake mfzrun
DEB_PROGRAMS_TO_INSTALL += mfm-config
DEB_PROGRAMS_PATHS_TO_INSTALL := $(DEB_PROGRAMS_TO_INSTALL:%=bin/%)
DEB_MFM_BINDIR := $(DESTDIR)/usr/bin
DEB_MFM_RESDIR := $(DESTDIR)/usr/share/mfm/res

# For mfm-config
export DEB_MFM_BINDIR
export DEB_MFM_RESDIR

# We're recursing rather than depending on 'all' so that the
# $(PLATFORMS) mechanism doesn't need to know about install.
install:	FORCE
	@#COMMANDS=1 make -k all
	COMMANDS=1 make -k -C src/drivers/mfzrun all
	mkdir -p $(DEB_MFM_BINDIR)
	cp -a $(DEB_PROGRAMS_PATHS_TO_INSTALL) $(DEB_MFM_BINDIR)
	mkdir -p $(DEB_MFM_RESDIR)
	cp -r res/* $(DEB_MFM_RESDIR)
	@# MAN AND DOC?

version:	FORCE
	@echo $(MFM_VERSION_NUMBER)
