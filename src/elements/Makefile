# Who we are
COMPONENTNAME:=elements

# Where's the top
BASEDIR:=../..

# What we need from outside
INCLUDES += -I $(BASEDIR)/src/core/include

# Magic def to put newlines in patsubst results
define \n


endef

ELEMENTS:=$(wildcard src/Element_*.cpp)
ELEMENT_HEADER_INCLUDES:=$(patsubst src/%.cpp,\#include "%.h"\n,$(ELEMENTS))
#${warning EHI $(ELEMENT_HEADER_INCLUDES) iha}
ELEMENT_MACRO_INVOKES:=$(patsubst src/%.cpp,XX(%);,$(ELEMENTS))
UNUSED1:=${shell echo '$(ELEMENT_HEADER_INCLUDES)' > include/StdElementsHeaders.inc}
UNUSED2:=${shell echo "$(ELEMENT_MACRO_INVOKES)" > include/StdElements.inc}

# Do the library thing
include $(BASEDIR)/config/Makelib.mk

# Extra dependencies for StdElements
$(BASEDIR)/build/$(COMPONENTNAME)/StdElements.o:	include/StdElements.inc
$(BASEDIR)/build/$(COMPONENTNAME)/StdElements.o:	include/StdElements.tcc
$(BASEDIR)/build/$(COMPONENTNAME)/StdElements.o:	include/StdElementsHeaders.inc
$(BASEDIR)/build/$(COMPONENTNAME)/StdElements.o:	src/StdElements.cpp

