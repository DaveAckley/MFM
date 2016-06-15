DESTDIR:=../../../res/images
ZSHEET_FILE:=mfms-icons-ZSHEET.png
ZSHEET_PATH:=$(DESTDIR)/$(ZSHEET_FILE)

$(ZSHEET_PATH):	generateZSheetFromMasterRowSVG.pl mfms-icons.svg Makefile
	./generateZSheetFromMasterRowSVG.pl ./mfms-icons.svg
	mv -f $(ZSHEET_FILE) $(ZSHEET_PATH)

