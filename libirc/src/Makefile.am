# $Id$

lib_LTLIBRARIES = libIRC.la

libIRC_la_SOURCES = \
	IRCClient.cpp \
	IRCServer.cpp \
	IRCTextUtils.cxx \
	IRCUserManager.cpp \
	TCPConnection.cpp \
	irClientCommands.cpp \
	irClientEvents.cpp \
	ircBasicCommands.cpp \
	ircBasicCommands.h \
	ircCommands.cpp	\
	libIRC.cpp \
	net.cpp

libIRC_la_LIBADD = ../botlib/src/libbot.la

include $(top_srcdir)/misc/Makefile.defs
