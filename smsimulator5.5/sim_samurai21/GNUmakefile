# $Id: GNUmakefile,v 1.1 1999/01/07 16:05:40 gunter Exp $
# --------------------------------------------------------------
# GNUmakefile for examples module.  Gabriele Cosmo, 06/04/98.
# --------------------------------------------------------------

name := sim_samurai21
G4TARGET := $(name)
G4EXLIB := true
G4WORKDIR := ..

.PHONY: all
all: bin lib rootlib
rootlib:
	g++ -Wl,-soname=lib$(name).so -shared -o lib$(name).so $(G4WORKDIR)/tmp/Linux-g++/$(name)/*.o
	@cp lib$(name).so ../lib

CPPFLAGS += -fPIC $(shell $(ROOTSYS)/bin/root-config --cflags)  -g -UG4UI_USE_QT $(G4SMCPPFLAGS)
CPPFLAGS  += -I$(TARTSYS)/include
ROOTLIBS = $(shell $(ROOTSYS)/bin/root-config --glibs)
TARTLIBS = -L$(TARTSYS)/lib -lanaroot -lXMLParser
EXTRALIBS += $(ROOTLIBS) $(G4SMLDLIBS) $(TARTLIBS)

## for kondo
ifdef TANASYS
CPPFLAGS  += -DTANA -I$(TANASYS)/include
EXTRALIBS += -L$(TANASYS)/lib -lmyanaroot -lMinuit
endif
#

include $(G4INSTALL)/config/binmake.gmk

cleanall: clean
	rm -vf include/*~
	rm -vf src/*~
	rm -vf ./*~
	rm -vf ./*.so
	rm -vf ../lib/lib$(name).so
