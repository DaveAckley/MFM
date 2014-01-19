CC := g++ 
CFLAGS += -ansi -pedantic -Wall -Werror -O99
ALLDEP := Makefile
SRCDIR := src
BUILDDIR := build
OUTPUTDIR := bin
TESTTARGET:= $(OUTPUTDIR)/mfm_test
TARGET := $(OUTPUTDIR)/mfm

SIMMAIN := $(SRCDIR)/drivers/MFMSim.cpp

SRCEXT := cpp
OBJEXT := o
HDRPAT := -name *.h -o -name *.tcc
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
HEADERS := $(shell find $(SRCDIR) -type f $(HDRPAT))
ALLDEP += $(HEADERS)
OBJECTS := $(shell find $(BUILDDIR) -name *.$(OBJEXT))

LIB := -L lib -lm -lSDL -L build
INC := -I src/core/include -I src/gui/include -I src/sim/include

$(TARGET): dependencies
	$(CC) $(CFLAGS) -o $(TARGET) $(SIMMAIN) $(LIB) $(OBJECTS) $(INC)

dependencies:
	@mkdir -p $(OUTPUTDIR)
	@make -C src/core
	@make -C src/sim
	@make -C src/gui

clean:
	@echo "Cleaning..."
	@echo "$(RM) -r $(BUILDDIR) $(TARGET) $(OUTPUTDIR)"; $(RM) -r $(BUILDDIR) $(TARGET) $(OUTPUTDIR)

.PHONY: clean