DESTDIR:=../../../res/images
ZSHEET_FILE:=mfms-icons-ZSHEET.png
ZSHEET_PATH:=$(DESTDIR)/$(ZSHEET_FILE)
ZSHEET_FILE_T2:=t2viz-icons-ZSHEET.png
ZSHEET_PATH_T2:=$(DESTDIR)/$(ZSHEET_FILE_T2)

all:	$(ZSHEET_PATH) $(ZSHEET_PATH_T2)

$(ZSHEET_PATH):	generateZSheetFromMasterRowSVG.pl mfms-icons.svg Makefile
	./generateZSheetFromMasterRowSVG.pl ./mfms-icons.svg
	mv -f $(ZSHEET_FILE) $(ZSHEET_PATH)

$(ZSHEET_PATH_T2):	generateT2VizZSheetFromMasterRowSVG.pl t2viz-icons.svg Makefile
	./generateT2VizZSheetFromMasterRowSVG.pl ./t2viz-icons.svg
	mv -f $(ZSHEET_FILE_T2) $(ZSHEET_PATH_T2)

