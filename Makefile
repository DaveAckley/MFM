CC := g++ -std=c++0x
SRCDIR := src
BUILDDIR := build
OUTPUTDIR := bin
TARGET := $(OUTPUTDIR)/mfm

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
LIB := -L lib -lm -lSDL
INC := -I include

$(TARGET): $(OBJECTS)
	@mkdir -p $(OUTPUTDIR)
	@echo "$(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo "$(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo "Cleaning..."
	@echo "$(RM) -r $(BUILDDIR) $(TARGET) $(OUTPUTDIR)"; $(RM) -r $(BUILDDIR) $(TARGET) $(OUTPUTDIR)

.PHONY: clean
