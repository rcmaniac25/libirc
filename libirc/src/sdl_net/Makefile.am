# $Id$

noinst_LTLIBRARIES = libSDLnet.la

libSDLnet_la_LDFLAGS = -static
libSDLnet_la_SOURCES = \
	SDLnet.cpp \
	SDLnetTCP.cpp \
	SDLnetUDP.cpp \
	SDLnetselect.cpp

noinst_HEADERS = \
	SDL_net.h \
	SDLnetsys.h

EXTRA_DIST = \
	CHANGES \
	COPYING \
	README

MAINTAINERCLEANFILES = Makefile.in
