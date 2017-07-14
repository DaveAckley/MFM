# Platform-specific definitions needed by builds of all components,
# not just us

override INCLUDES += -I $(BASEDIR)/src/platform-tile/include

# Gah, relink core after ourselves -- we need to get MFMFailCodeReason
# from libcore, and usually nobody but us uses that.

override LIBS += -L $(BASEDIR)/build/platform-tile -l mfmplatform-tile -l mfmcore
