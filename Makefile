# Makefile of PDCurses wincon port for MSYS2/MinGW-w64 on Windows 10 JP

SRCDIR = wincon
DSTDIR = 0000_dist

all:
	cd $(SRCDIR); $(MAKE) -f Makefile WIN10_JP=Y DLL=Y
	cd $(SRCDIR); $(MAKE) -f Makefile WIN10_JP=Y DLL=Y demos
	mkdir -p $(DSTDIR)/include/pdcurses
	mkdir -p $(DSTDIR)/bin
	mkdir -p $(DSTDIR)/lib
	echo '#include "pdcurses/curses.h"' > $(DSTDIR)/include/pdcurses.h
	cp curses.h   $(DSTDIR)/include/pdcurses
	cp curspriv.h $(DSTDIR)/include/pdcurses
	cp panel.h    $(DSTDIR)/include/pdcurses
	cp $(SRCDIR)/pdcurses.dll $(DSTDIR)/bin
	cp $(SRCDIR)/pdcurses.a   $(DSTDIR)/lib/libpdcurses.dll.a

clean:
	cd $(SRCDIR); $(MAKE) clean
	rm -rf $(DSTDIR)/include
	rm -rf $(DSTDIR)/bin
	rm -rf $(DSTDIR)/lib

