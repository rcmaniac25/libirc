# $Id$

SUBDIRS = sdl_net

lib_LTLIBRARIES = libIRC.la

libIRC_la_SOURCES = \
	TCPConnection.cpp \
	avltree.cpp \
	bufsock.cpp \
	ctcp.cpp \
	hashtable.cpp \
	irc_core.cpp \
	irc_funcs.cpp \
	libIRC.cpp \
	list.cpp \
	map.cpp \
	tcp.cpp \
	trace.cpp \
	util.cpp

libIRC_la_CPPFLAGS = -DCALLBACK_COLLECTION_IS_AVLTREE -I$(top_srcdir)/src/sdl_net

libIRC_la_LIBADD = sdl_net/libSDLnet.la

MAINTAINERCLEANFILES = Makefile.in
