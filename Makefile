# Makefile of PDCurses wincon port for MSYS2/MinGW-w64 on Windows 10 JP

SRC_DIR  = wincon
DIST_DIR = 0000_dist

all:
	cd $(SRC_DIR); $(MAKE) -f Makefile WIN10_JP=Y DLL=Y
	cd $(SRC_DIR); $(MAKE) -f Makefile WIN10_JP=Y DLL=Y demos
	mkdir -p $(DIST_DIR)/include/pdcurses
	mkdir -p $(DIST_DIR)/bin
	mkdir -p $(DIST_DIR)/lib
	echo '#include "pdcurses/curses.h"' > $(DIST_DIR)/include/pdcurses.h
	cp curses.h   $(DIST_DIR)/include/pdcurses
	cp curspriv.h $(DIST_DIR)/include/pdcurses
	cp panel.h    $(DIST_DIR)/include/pdcurses
	cp $(SRC_DIR)/pdcurses.dll $(DIST_DIR)/bin
	cp $(SRC_DIR)/pdcurses.a   $(DIST_DIR)/lib/libpdcurses.dll.a

clean:
	cd $(SRC_DIR); $(MAKE) clean
	rm -rf $(DIST_DIR)/include
	rm -rf $(DIST_DIR)/bin
	rm -rf $(DIST_DIR)/lib

