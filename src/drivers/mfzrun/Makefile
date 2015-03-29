BASE_DIR:=../../..
DRIVER_NAME:=mfzrun
BIN_DIR:=$(BASE_DIR)/bin
SRCS:=$(wildcard *.tmpl)
TARGETS:=$(patsubst %.tmpl, $(BIN_DIR)/%, $(SRCS))
ALLDEP+=./Makefile

all:	$(TARGETS) $(MOD_TARGETS)

$(BIN_DIR):	FORCE
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR)/%: ./%.tmpl *.inc FORCE | $(BIN_DIR)
	perl -e 'while(<>){if($$_=~/<<<INCLUDE:\s*(.+)\s*>>>/){system("cat $$1")} else {print}}' $< >$@
	chmod 755 $@

clean:	FORCE
	@rm -f *~
	@rm -f ${OBJS}

realclean:	clean
	@rm -f ${TARGET}

.PHONY:	FORCE
