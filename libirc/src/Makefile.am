# $Id$

SUBDIRS = sdl_net

DIST_SUBDIRS = \
	old_code \
	sdl_net

lib_LTLIBRARIES = libIRC.la

libIRC_la_SOURCES = \
	TCPConnection.cpp \
	ircCommands.cpp	\
	IRCChannels.cpp	\
	libIRC.cpp \
	ircBasicCommands.cpp

libIRC_la_CPPFLAGS = -I$(top_srcdir)/src/sdl_net

libIRC_la_LIBADD = sdl_net/libSDLnet.la

MAINTAINERCLEANFILES = Makefile.in
