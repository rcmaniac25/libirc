# $Id$

include_HEADERS = \
	TCPConnection.h \
	avltree.h \
	bufsock.h \
	cmd.h \
	conf.h \
	config.h \
	ctcp.h \
	defs.h \
	errcodes.h \
	funcptrs.h \
	hashtable.h \
	ircCommands.h \
	irc_core.h \
	irc_rpl.h \
	irc_version.h \
	libIRC.h \
	list.h \
	macros_priv.h \
	map.h \
	tcp.h \
	trace.h \
	util.h

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
