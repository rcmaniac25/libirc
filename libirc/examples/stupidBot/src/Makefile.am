# $Id$

noinst_PROGRAMS = stupidBot

stupidBot_SOURCES = stupidBot.cpp \
					URLManager.cpp
					
stupidBot_LDADD = $(top_builddir)/src/libIRC.la

stupidBot_LDFLAGS = -lcurl

MAINTAINERCLEANFILES = Makefile.in

include $(top_srcdir)/misc/Makefile.defs
