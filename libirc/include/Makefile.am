# $Id$

include_HEADERS = \
	TCPConnection.h \
	TextUtils.h	\
	Singleton.h \
	IRCChannels.h \
	IRCNumerics.h \
	IRCEvents.h \
	ircCommands.h \
	libIRC.h 

noinst_HEADERS = \
	libirc_version.h

EXTRA_DIST = \
	config.h.in \
	version

MAINTAINERCLEANFILES = Makefile.in

libirc_version.h:
	@sh $(top_srcdir)/misc/vers.sh libirc_version "libIRC" > $@

# make sure that the build date/version is updated every time make is run
all-local:
	@sh $(top_srcdir)/misc/vers.sh libirc_version "libIRC" > libirc_version.h
