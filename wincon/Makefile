# GNU Makefile for PDCurses - Windows console
#
# Usage: make [-f path\Makefile] [DEBUG=Y] [DLL=Y] [WIDE=Y] [UTF8=Y] [WIN10_JP=Y]
#        [INFOEX=N] [tgt]
#
# where tgt can be any of:
# [all|demos|pdcurses.a|testcurs.exe...]

O = o
E = .exe

ifeq ($(OS),Windows_NT)
	RM = cmd /c del
else
	RM = rm -f
endif

# for MSYS2/MinGW-w64
ifneq ($(MSYSTEM),)
	RM = rm -f
endif

ifndef PDCURSES_SRCDIR
	PDCURSES_SRCDIR = ..
endif

osdir		= $(PDCURSES_SRCDIR)/wincon
common		= $(PDCURSES_SRCDIR)/common

include $(common)/libobjs.mif

PDCURSES_WIN_H	= $(osdir)/pdcwin.h

CC		= gcc
AR		= ar
STRIP		= strip
LINK		= gcc
WINDRES		= windres
WINDRES_FLAGS	=

ifeq ($(DEBUG),Y)
	CFLAGS  = -g -Wall -Wextra -Wno-unused-parameter -Wno-cast-function-type -DPDCDEBUG
	LDFLAGS = -g
else
	CFLAGS  = -O2 -Wall -Wextra -Wno-unused-parameter -Wno-cast-function-type
	LDFLAGS =
endif

CFLAGS += -I$(PDCURSES_SRCDIR)

# for windows 10 jp
ifeq ($(WIN10_JP),Y)
	CFLAGS += -DPDC_WIN10_JP  -DPDC_WIDE  -DPDC_FORCE_UTF8            \
	          -DPDC_FORCE_UPDATE          -DPDC_SKIP_ZERO_WIDTH_SPACE \
	          -DPDC_NO_CHECK_ON_RESIZE    -DPDC_CLEAR_ON_RESIZE       \
	          -DPDC_CURSOR_HOME_ON_RESIZE -DPDC_ADDITIONAL_KEYS       \
	          -DPDC_VT_MOUSE_INPUT        -DPDC_DISABLE_CLICK_EVENT   \
	          -DPDC_PASTE_ON_RIGHT_CLICK
	PDCOBJS += pdcdisp_sub.o pdckbd_sub.o
	WINDRES_FLAGS += -DPDC_WIN10_JP
endif

ifeq ($(WIDE),Y)
	CFLAGS += -DPDC_WIDE
endif

ifeq ($(UTF8),Y)
	CFLAGS += -DPDC_FORCE_UTF8
endif

ifeq ($(DLL),Y)
	CFLAGS += -DPDC_DLL_BUILD
	LIBEXE = $(CC)
	LIBFLAGS = -Wl,--out-implib,pdcurses.a -shared -o
	LIBCURSES = pdcurses.dll
	CLEAN = $(LIBCURSES) *.a
	RESOURCE = pdcurses.o
else
	LIBEXE = $(AR)
	LIBFLAGS = rcv
	LIBCURSES = pdcurses.a
	CLEAN = *.a
endif

ifeq ($(INFOEX),N)
	CFLAGS += -DHAVE_NO_INFOEX
endif

.PHONY: all libs clean demos dist

all:	libs

libs:	$(LIBCURSES)

clean:
	-$(RM) *.o
	-$(RM) *.exe
	-$(RM) $(CLEAN)

demos:	$(DEMOS)
ifneq ($(DEBUG),Y)
	$(STRIP) *.exe
endif

$(LIBCURSES) : $(LIBOBJS) $(PDCOBJS) $(RESOURCE)
	#$(LIBEXE) $(LIBFLAGS) $@ $?
	$(LIBEXE) $(LIBFLAGS) $@ $^

pdcurses.o: $(common)/pdcurses.rc
	$(WINDRES) $(WINDRES_FLAGS) -i $(common)/pdcurses.rc pdcurses.o

$(LIBOBJS) $(PDCOBJS) : $(PDCURSES_HEADERS)
$(PDCOBJS) : $(PDCURSES_WIN_H)
$(DEMOS) : $(PDCURSES_CURSES_H) $(LIBCURSES)
panel.o : $(PANEL_HEADER)

$(LIBOBJS) : %.o: $(srcdir)/%.c
	$(CC) -c $(CFLAGS) $<

$(PDCOBJS) : %.o: $(osdir)/%.c
	$(CC) -c $(CFLAGS) $<

firework.exe ozdemo.exe rain.exe testcurs.exe worm.exe xmas.exe \
ptest.exe: %.exe: $(demodir)/%.c
	$(CC) $(CFLAGS) -o$@ $< $(LIBCURSES)

tuidemo.exe: tuidemo.o tui.o
	$(LINK) $(LDFLAGS) -o$@ tuidemo.o tui.o $(LIBCURSES)

tui.o: $(demodir)/tui.c $(demodir)/tui.h $(PDCURSES_CURSES_H)
	$(CC) -c $(CFLAGS) -I$(demodir) -o$@ $<

tuidemo.o: $(demodir)/tuidemo.c $(PDCURSES_CURSES_H)
	$(CC) -c $(CFLAGS) -I$(demodir) -o$@ $<
