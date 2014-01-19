CC := g++
CFLAGS := -ansi -pedantic -Wall -Werror -O99
ALLDEP := Makefile
SRCDIR := src
BUILDDIR := ../../build

SRCEXT := cpp
HDRPAT := -name *.h -o -name *.tcc
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
HEADERS := $(shell find $(SRCDIR) -type f $(HDRPAT))
ALLDEP += $(HEADERS)
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
LIB := -lm -lSDL
INC := -I include -I ../sim/include -I ../core/include

all: $(OBJECTS)

$(BUILDDIR)/%.o:	$(SRCDIR)/%.$(SRCEXT) $(ALLDEP)
	@mkdir -p $(BUILDDIR)
	@echo "$(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<