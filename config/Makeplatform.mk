# This needs to be reworked, but for now (heh), we're saying that
# X86_64 and X86 is platform 'linux' and armv7l is platform 'tile'
MACHINE_TYPE:=$(shell uname -m)

PLATFORM:=
ifeq ($(MACHINE_TYPE), x86_64)
PLATFORM:=linux
endif
ifeq ($(MACHINE_TYPE), x86)
PLATFORM:=linux
endif
ifeq ($(MACHINE_TYPE), armv7l)
PLATFORM:=tile
endif

ifeq ($(PLATFORM), "")
$(error unrecognized platform '$(MACHINE_TYPE)')
endif
