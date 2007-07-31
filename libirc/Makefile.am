# $Id$

AUTOMAKE_OPTIONS = 1.6 dist-zip dist-bzip2
ACLOCAL_AMFLAGS = -I m4

if BUILD_EXAMPLES
EXAMPLES=examples
else
EXAMPLES=
endif

SUBDIRS = \
	include \
	m4 \
	misc \
	src \
	$(EXAMPLES)

DIST_SUBDIRS = \
	examples \
	include \
	m4 \
	misc \
	src \
	vc7.1 \
	vc8

EXTRA_DIST = \
	AUTHORS \
	BUGS \
	COPYING \
	ChangeLog \
	HACKING \
	INSTALL \
	NEWS \
	ReadMe.txt \
	TODO \
	autogen.sh \
	configure.ac \
	libtool

MAINTAINERCLEANFILES = \
	@CONFIG_CACHE@ \
	Makefile.in \
	aclocal.m4 \
	autom4te.cache \
	config.log \
	config.status \
	configure \
	libtool

examples: all-am
	cd examples && $(MAKE) $(AM_MAKEFLAGS)


# Print out an informative summary.  As just about everything seems to
# end up calling the all-am hook, which in turns calls the all-local
# hook.  The initial target goal is used to print out a custom summary
# message.  If the make being used doesn't set the MAKECMDGOALS
# variable, something generic is printed still.  For a make install,
# print out rule lines according to the size of the installation path
# to emphasize the achievement.
#
all-local:
	@@ECHO@ "Done."
	@@ECHO@
	@@ECHO@ "libIRC @LIBIRC_VERSION@, Build @CONFIG_DATE@"
	@@ECHO@
	@@ECHO@ "$(MAKECMDGOALS) is goals"
	@if test "x$(MAKECMDGOALS)" = "xall-am" ; then \
	  @ECHO@ @ECHO_N@ "Elapsed compilation time: " ;\
	  sh $(top_srcdir)/misc/elapsed.sh `cat $(top_builddir)/include/libirc_version.h | grep Compilation` ;\
	  @ECHO@ @ECHO_N@ "Elapsed time since configuration: " ;\
	  sh $(top_srcdir)/misc/elapsed.sh @CONFIG_TIME@ ;\
	  @ECHO@ "---" ;\
	  @ECHO@ "Run 'make examples' to build the libIRC framework examples" ;\
	  @ECHO@ "Run 'make install' to begin installation into $(prefix)" ;\
	elif test "x$(MAKECMDGOALS)" = "xinstall-am" ; then \
	  @ECHO@ @ECHO_N@ "Elapsed installation time: " ;\
	  sh $(top_srcdir)/misc/elapsed.sh `cat $(top_builddir)/include/libirc_version.h | grep Compilation` ;\
	  @ECHO@ @ECHO_N@ "Elapsed time since configuration: " ;\
	  sh $(top_srcdir)/misc/elapsed.sh @CONFIG_TIME@ ;\
	  @ECHO@ "---" ;\
	  @ECHO@ "Run 'make examples' to build the libIRC framework examples" ;\
	  @ECHO@ ;\
	  line1="  libIRC @LIBIRC_VERSION@ is now installed into $(prefix)" ;\
	  line2="  Be sure to add $(prefix)/bin to your PATH" ;\
	  length="`@ECHO@ $(prefix)@LIBIRC_VERSION@ | wc | awk '{print $$3}'`" ;\
	  length="`expr $$length + 34`" ;\
	  len=$$length ;\
	  while test $$len -gt 0  ; do \
	    @ECHO@ -n "***" ;\
	    len="`expr $$len - 3`" ;\
          done ;\
	  @ECHO@ ;\
	  @ECHO@ "$$line1" ;\
	  @ECHO@ "$$line2" ;\
	  while [ $$length -gt 0 ] ; do \
	    @ECHO@ @ECHO_N@ "***" ;\
	    length="`expr $$length - 3`" ;\
          done ;\
	  @ECHO@ ;\
	elif test "x$(MAKECMDGOALS)" = "x" ; then \
	  @ECHO@ @ECHO_N@ "Elapsed time: " ;\
	  sh $(top_srcdir)/misc/elapsed.sh `cat $(top_builddir)/include/libirc_version.h | grep Compilation` ;\
	  @ECHO@ @ECHO_N@ "Elapsed time since configuration: " ;\
	  sh $(top_srcdir)/misc/elapsed.sh @CONFIG_TIME@ ;\
	  @ECHO@ "---" ;\
	  @ECHO@ "Run 'make test' to test the libIRC framework" ;\
	fi
	@@ECHO@
