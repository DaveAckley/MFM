# This needs to be reworked, but for now (heh), we're saying that
# the x86's are platform 'linux' and armv7l (+ nodename beaglebone)
# is platform 'tile' (need nodename else generic armhf build fails
# on launchpad)

ifdef FORCE_MACHINE_TYPE
MACHINE_TYPE:=$(FORCE_MACHINE_TYPE)
else
MACHINE_TYPE:=$(shell uname -m)
endif

PLATFORM:=
ifeq ($(MACHINE_TYPE), x86_64)
PLATFORM:=linux
endif
ifeq ($(MACHINE_TYPE), x86)
PLATFORM:=linux
endif
ifeq ($(MACHINE_TYPE), i386)
PLATFORM:=linux
endif
ifeq ($(MACHINE_TYPE), i586)
PLATFORM:=linux
endif
ifeq ($(MACHINE_TYPE), i686)
PLATFORM:=linux
endif
ifeq ($(MACHINE_TYPE), armv7l)
NODE_NAME:=$(shell uname -n)
ifeq ($(NODE_NAME), beaglebone)
PLATFORM:=tile
else
PLATFORM:=linux
endif
endif

$(info MACHINE TYPE: $(MACHINE_TYPE) / PLATFORM: $(PLATFORM))

ifeq ($(PLATFORM),)
$(error unrecognized platform '$(MACHINE_TYPE)')
endif
