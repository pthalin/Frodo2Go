# Makefile.in for Frodo (generic Unix/X11)
FRODO2GO_VERSION=1.0.1

## System specific configuration

SHELL = /bin/sh

prefix = /usr/local
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
datarootdir = ${prefix}/share
datadir = ${datarootdir}

DESTDIR =

CC = gcc
CXX = g++

SYSROOT     := $(shell $(CC) --print-sysroot)


LIBS   = -lSDL_image -lpng -lm $(shell /usr/bin/sdl-config --cflags)
LDFLAGS= $(shell /usr/bin/sdl-config --static-libs) 

CFLAGS = -g  -I./ -DKBD_LANG=0 -DHAVE_SDL -Wno-narrowing -DFRODO2GO_VERSION=\"$(FRODO2GO_VERSION)\"


DEFS = -DHAVE_CONFIG_H -DDATADIR=\"$(datadir)/frodo/\" -DBINDIR=\"$(bindir)/\"

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

## Files
GUIOBJS = font.o keyboard.o menu_file.o menu_main.o
OBJS    = $(GUIOBJS) main.o Display.o Prefs.o SID.o REU.o IEC.o 1541fs.o \
          1541d64.o 1541t64.o 1541job.o CmdPipe.o
SLOBJS  = $(OBJS) C64.o CPUC64.o VIC.o CIA.o CPU1541.o
SLFLAGS = -DPRECISE_CPU_CYCLES=1 -DPRECISE_CIA_CYCLES=1 -DPC_IS_POINTER=0
SCOBJS  = $(OBJS) C64_SC.o CPUC64_SC.o VIC_SC.o CIA_SC.o CPU1541_SC.o CPU_common.o
SCFLAGS = -DFRODO_SC
GUISRCS = font.c keyboard.c menu_file.c menu_main.c
SRCS    = $(GUISRCS) main.cpp Display.cpp Prefs.cpp SID.cpp REU.cpp IEC.cpp 1541fs.cpp \
          1541d64.cpp 1541t64.cpp 1541job.cpp CmdPipe.cpp C64.cpp \
          C64_SC.cpp CPUC64.cpp CPUC64_SC.cpp VIC.cpp VIC_SC.cpp CIA.cpp \
          CIA_SC.cpp CPU1541.cpp CPU1541_PC.cpp CPU1541_SC.cpp CPU_common.cpp

## Rules
.PHONY: install installdirs uninstall clean distclean depend dep
.SUFFIXES:
.SUFFIXES: .o .cpp .c .h

all: Frodo FrodoSC

Frodo: $(SLOBJS)
	$(CXX) -o Frodo $(SLOBJS) $(LDFLAGS) $(LIBS) 

FrodoSC: $(SCOBJS)
	$(CXX) -o FrodoSC  $(SCOBJS) $(LDFLAGS) $(LIBS)

install: Frodo FrodoSC installdirs
	$(INSTALL_PROGRAM) Frodo $(DESTDIR)$(bindir)/Frodo
	$(INSTALL_PROGRAM) FrodoSC $(DESTDIR)$(bindir)/FrodoSC
	$(INSTALL_DATA) ../Kernal\ ROM $(DESTDIR)$(datadir)/frodo/Kernal\ ROM
	$(INSTALL_DATA) glade/Frodo.glade $(DESTDIR)$(datadir)/frodo/Frodo.glade
	$(INSTALL_DATA) maemo/frodo.desktop $(DESTDIR)$(datadir)/applications/hildon/frodo.desktop
	$(INSTALL_DATA) maemo/frodo.service $(DESTDIR)$(datadir)/dbus-1/services/frodo.service
	$(INSTALL_DATA) maemo/Frodo_26_26.png $(DESTDIR)$(datadir)/icons/hicolor/26x26/hildon/frodo.png
	$(INSTALL_DATA) maemo/Frodo_40_40.png $(DESTDIR)$(datadir)/icons/hicolor/40x40/hildon/frodo.png
	$(INSTALL_DATA) maemo/Frodo_64_64.png $(DESTDIR)$(datadir)/icons/hicolor/scalable/hildon/frodo.png

installdirs:
	$(SHELL) mkinstalldirs $(DESTDIR)$(bindir) $(DESTDIR)$(datadir)/frodo $(DESTDIR)$(datadir)/applications/hildon $(DESTDIR)$(datadir)/dbus-1/services $(DESTDIR)$(datadir)/icons/hicolor/26x26/hildon  $(DESTDIR)$(datadir)/icons/hicolor/40x40/hildon $(DESTDIR)$(datadir)/icons/hicolor/scalable/hildon

uninstall:
	rm -f $(DESTDIR)$(bindir)/Frodo
	rm -f $(DESTDIR)$(bindir)/FrodoSC
	rm -f $(DESTDIR)$(datadir)/frodo/Kernal\ ROM
	rmdir $(DESTDIR)$(datadir)/frodo

clean:
	rm -f $(SLOBJS) $(SCOBJS)
	rm -f Frodo FrodoSC
	rm -f core* *.core *~ *.bak

distclean: clean
	rm -rf autom4te.cache
	rm -f Makefile sysconfig.h
	rm -f config.cache config.log config.status config.h

depend dep:
	makedepend $(CPPFLAGS) -Y. $(SRCS) 2>/dev/null

.cpp.o:
	$(CXX) $(DEFS) $(CFLAGS) -o $@ -c $*.cpp

C64.o: C64.cpp
	$(CXX) $(DEFS) $(CFLAGS) $(SLFLAGS) -o $@ -c $*.cpp

C64_SC.o: C64_SC.cpp
	$(CXX) $(DEFS) $(CFLAGS) $(SCFLAGS) -o $@ -c $*.cpp

CPUC64.o: CPUC64.cpp
	$(CXX) $(DEFS) $(CFLAGS) $(SLFLAGS) -o $@ -c $*.cpp

CPUC64_SC.o: CPUC64_SC.cpp
	$(CXX) $(DEFS) $(CFLAGS) $(SCFLAGS) -o $@ -c $*.cpp

CPU1541.o: CPU1541.cpp
	$(CXX) $(DEFS) $(CFLAGS) $(SLFLAGS) -o $@ -c $*.cpp

CPU1541_SC.o: CPU1541_SC.cpp
	$(CXX) $(DEFS) $(CFLAGS) $(SCFLAGS) -o $@ -c $*.cpp

VIC_SC.o: VIC_SC.cpp
	$(CXX) $(DEFS) $(CFLAGS) $(SCFLAGS) -o $@ -c $*.cpp

CIA_SC.o: CIA_SC.cpp
	$(CXX) $(DEFS) $(CFLAGS) $(SCFLAGS) -o $@ -c $*.cpp


#-------------------------------------------------------------------------
# DO NOT DELETE THIS LINE -- make depend depends on it.
