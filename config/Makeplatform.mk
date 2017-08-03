# This needs to be reworked, but for now (heh), we're saying that
# the x86's are platform 'linux' and armv7l is platform 'tile'
MACHINE_TYPE:=$(shell uname -m)

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
PLATFORM:=tile
endif

ifeq ($(PLATFORM),)
$(error unrecognized platform '$(MACHINE_TYPE)')
endif
