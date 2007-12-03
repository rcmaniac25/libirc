dnl BRL-CAD
dnl Copyright (c) 2004-2007 Christopher Sean Morrison
dnl
dnl This program is free software; you can redistribute it and/or
dnl modify it under the terms of the appropriate license described in
dnl the file named COPYING that should have accompanied this file.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
dnl
dnl $Id$
dnl
dnl ******************************************************************
dnl ***                  libIRC's configure.ac                     ***
dnl ******************************************************************
dnl
dnl Herein lies the venerable GNU Autotools configure template for
dnl libIRC.  As best is reasonably possible, proper ordering and
dnl separation of tests and settings should be maintained per the
dnl recommended standard layout.  The tests should be added to the
dnl rather clearly labeled sections below so that they are as follows:
dnl
dnl      0) information on the package
dnl	 1) check command-line arguments
dnl	 2) check programs
dnl	 3) check libraries
dnl	 4) check headers
dnl	 5) check types/structures
dnl      6) check compiler characteristics
dnl	 7) check functions
dnl	 8) check system services
dnl	 9) output a summary
dnl
dnl Any useful build arguments should be added to the output summary
dnl at the end.  Although it may be possible to check for certain
dnl features of command line arguments near the top, related tests
dnl should be delayed and placed into the appropriate test section.
dnl
dnl Features of the GNU Autotools that would require an increase in
dnl the minimum version requirement are highly discouraged.  Likewise
dnl discouraged is rolling back support for versions released prior
dnl to the AC_PREREQ version shown below unless extensive testing has
dnl been performed.
dnl

AC_PREREQ(2.52)

AC_INIT(libIRC, 0.3, [http://libirc.sf.net], libIRC)
AC_REVISION($Revision$)

AC_CONFIG_AUX_DIR(misc)

# keep track of cmd-line options for later
ARG0="$0"
AC_SUBST(ARG0)
ARGS="$*"
AC_SUBST(ARGS)

dnl just in case
PACKAGE_NAME="libIRC"
AC_SUBST(PACKAGE_NAME)

dnl See HACKING for details on how to properly update the version
MAJOR_VERSION=0
MINOR_VERSION=3
PATCH_VERSION=0
AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(PATCH_VERSION)

LIBIRC_VERSION="$MAJOR_VERSION.$MINOR_VERSION.$PATCH_VERSION"
AC_SUBST(LIBIRC_VERSION)

# force locale setting to C so things like date output as expected
LC_ALL=C

CONFIG_DAY=`date +%d`
CONFIG_MONTH=`date +%m`
CONFIG_YEAR=`date +%Y`
CONFIG_DATE="${CONFIG_YEAR}${CONFIG_MONTH}${CONFIG_DAY}"
CONFIG_TIME="`date '+%H %M %S'`"
AC_SUBST(CONFIG_DAY)
AC_SUBST(CONFIG_MONTH)
AC_SUBST(CONFIG_YEAR)
AC_SUBST(CONFIG_DATE)
AC_SUBST(CONFIG_TIME)

title="Configuring libIRC Release $LIBIRC_VERSION, Build $CONFIG_DATE"
length="`echo x${title}x | wc -c`"
separator=""
while test $length -gt 1 ; do
separator="${separator}*"
length="`expr $length - 1`"
done

AC_MSG_RESULT([***${separator}***])
AC_MSG_RESULT([*** ${title} ***])
AC_MSG_RESULT([***${separator}***])

# necessary for proper file creation on nfs volumes
umask 022

# override the default autoconf cflags if user has not modified them
if test "x$CFLAGS" = "x" ; then
	# an unset CFLAGS var is set to "-O2 -g" during AC_PROG_CC, so
	# set it to something benign instead like whitespace
	CFLAGS=" "
fi
if test "x$CXXFLAGS" = "x" ; then
	# an unset CXXFLAGS var is set to "-O2 -g" during AC_PROG_CXX, so
	# set it to something benign instead like whitespace
	CXXFLAGS=" "
fi

# cannot override LD directly, so warn about that (configure sets it)
if test "x$LD" != "x" ; then
	AC_MSG_WARN([LD cannot be set directly yet it seems to be set ($LD)])
fi

# classify this machine
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

# am_init_automake performs a ac_prog_install check so need to handle
# problematic /usr/brl/bin/install on irix
if test "x$build_vendor" = "xsgi" ; then
   PATH="`echo $PATH | sed 's/\/brl\/bin/bin/g'`"
fi

# sanity check, make sure we have sources where we expect them
AC_CONFIG_SRCDIR([src/libIRC.cpp])

# if this is not a SVN checkout, disable dependency tracking for a
# faster build. it's not likely that the user is doing development
# from a source release. at least they really should not given the
# extra steps that are necessary for making a proper useful diff.
AC_MSG_CHECKING([whether dependency tracking should be enabled])
if test -d $srcdir/.svn ; then
	if test "x$enable_maintainer_mode" = "x" ; then
		enable_maintainer_mode="yes"
	fi
	if test "x$enable_dependency_tracking" = "x" ; then
		enable_dependency_tracking="yes"
	fi
	AC_MSG_RESULT([yes])
else
	if test "x$enable_maintainer_mode" = "x" ; then
		enable_maintainer_mode="no"
	fi
	if test "x$enable_dependency_tracking" = "x" ; then
		enable_dependency_tracking="no"
	fi
	AC_MSG_RESULT([no])
fi

# init the venerable automake only _once_ or incur the wrath of
# several automake bugs (like "aclocal-" and install detection)
AM_INIT_AUTOMAKE([1.6 dist-zip dist-bzip2])

# disables the build system dependency tracking by default for the
# automake and autoconf template files.
AM_MAINTAINER_MODE

# write out all of our definitions to this header
AM_CONFIG_HEADER([include/config.h])

# automatically enable and load our configure cache file if available
BC_CONFIG_CACHE


dnl ***********************
dnl *** Check arguments ***
dnl ***********************

BC_CONFIGURE_STAGE([arguments], [1 of 9])

# Allow user to specify flags without relying on environment
BC_WITH_FLAG_ARGS

# build examples
AC_MSG_CHECKING(whether to build the libIRC examples)
build_examples=no
AC_ARG_WITH(examples, AC_HELP_STRING(--with-examples,
		[build the libIRC examples (default=no)]),
	[if test "x$enable_examples" != "xno" ; then
		build_examples=yes
	fi]
)
AC_MSG_RESULT($build_examples)

# optimized
dnl optimized has alias(es)
use_optimized=no
AC_ARG_ENABLE(optimized, AC_HELP_STRING(--enable-optimized,
		[enable optimized compilation (default=no)]),
	[if test "x$enable_optimized" != "xno" ; then
		use_optimized=yes
	fi]
)

# warnings
dnl warnings has alias(es)
use_warnings=no
AC_ARG_ENABLE(warnings,	AC_HELP_STRING(--enable-warnings,
		[enable verbose compilation warnings (default=no)]),
	[if test "x$enable_warnings" != "xno" ; then
		use_warnings=yes
	fi]
)

# debug
dnl debug has alias(es)
use_debug=yes
AC_ARG_ENABLE(debug, AC_HELP_STRING(--disable-debug,
		[disable debug symbols (default=no)]),
	[if test "x$disable_debug" != "xno" ; then
		use_debug=no
	fi]
)

# profiling
dnl profiling has alias(es)
use_profiling=no
AC_ARG_ENABLE(profiling, AC_HELP_STRING(--enable-profiling,
		[enable profiling (default=no)]),
	[if test "x$enable_profiling" != "xno" ; then
		use_profiling=yes
	fi]
)

# argument aliases go below here (they should all become a single blank line)

# optimized alias
AC_ARG_ENABLE(optimization,,
	[if test "x$enable_optimization" != "xno" ; then
		use_optimized=yes
	fi]
)
AC_ARG_ENABLE(optimizations,,
	[if test "x$enable_optimizations" != "xno" ; then
		use_optimized=yes
	fi]
)
AC_MSG_CHECKING(whether to enable optimized compilation)
AC_MSG_RESULT($use_optimized)

# debug alias
AC_ARG_ENABLE(debugging,,
	[if test "x$disable_debug" != "xno" ; then
		use_debug=no
	fi]
)
AC_MSG_CHECKING(whether to disable debug mode compilation)
AC_MSG_RESULT($use_debug)

# profiling alias
AC_ARG_ENABLE(profile,,
	[if test "x$enable_profile" != "xno" ; then
		use_profiling=yes
	fi]
)
AC_ARG_ENABLE(profiled,,
	[if test "x$enable_profiled" != "xno" ; then
		use_profiling=yes
	fi]
)
AC_MSG_CHECKING(whether to enable profile mode compilation)
AC_MSG_RESULT($use_profiling)

# verbose warnings alias
AC_ARG_ENABLE(verbose-warnings,,
	[if test "x$enable_verbose_warnings" != "xno" ; then
		use_warnings=yes
	fi]
)
AC_ARG_ENABLE(warning,,
	[if test "x$enable_warning" != "xno" ; then
		use_warnings=yes
	fi]
)
AC_MSG_CHECKING(whether to enable verbose compilation warnings)
AC_MSG_RESULT($use_warnings)


dnl automatically scan /usr/local (e.g. BSD uses /usr/local for ports)
BC_SEARCH_DIRECTORY([/usr/local])


dnl **************************
dnl *** Check for programs ***
dnl **************************

BC_CONFIGURE_STAGE([programs], [2 of 9])

AC_PROG_CC
AM_PROG_CC_STDC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL

dnl check for basic shell facilities
AC_PATH_PROG(SH, sh)
AC_PATH_PROG(MV, mv)
AC_PATH_PROG(CP, cp)
AC_PATH_PROG(RM, rm)

dnl figure out which compiler is seems like we are using
dnl detected values include: gcc, mipspro, sunpro
dnl XXX -- none of the tests should be compiler-specific, but it's good to know in the log
compiler=unknown

dnl Determine whether we are using GCC
if test "x$GCC" = "xyes" ; then
	compiler=gcc
fi

dnl Determine whether we are using the MIPSpro compiler on SGI systems
if test "x$build_vendor" = "xsgi" ; then
	AC_MSG_CHECKING([whether we are using the MIPSpro compiler])
	using_mipspro=no
	AC_LINK_IFELSE([#if defined(sgi) && defined(_COMPILER_VERSION) && !defined(__GNUC__)
If we get here, we are using the MIPSPro compiler (and the link should fail).
#endif
int main() { return 0; }
	], [using_mipspro=no], [using_mipspro=yes])
	AC_MSG_RESULT($using_mipspro)
	if test "x$using_mipspro" = "xyes" ; then
		compiler=mipspro
	fi
fi

dnl Determine whether we are using the SUNWspro Sun Workshop compiler
if test "x$build_vendor" = "xsun" ; then
	AC_MSG_CHECKING([whether we are using the SUNWspro compiler])
	using_sunpro=no
	AC_LINK_IFELSE([#if defined(sun) && defined(__SUNPRO_C) && !defined(__GNUC__)
If we get here, we are using the SUNWspro compiler (and the link should fail).
#endif
int main() { return 0; }
	], [using_sunpro=no], [using_sunpro=yes])
	AC_MSG_RESULT($using_sunpro)
	if test "x$using_sunpro" = "xyes" ; then
		AR="CC -xar"
		LD="CC -G"
		export AR LD
		compiler=sunpro
	fi
fi

AC_DEFINE(EXTERN_STDOUT,,[Stdout is extern])


dnl ***************************
dnl *** Check for libraries ***
dnl ***************************

BC_CONFIGURE_STAGE([libraries], [3 of 9])


dnl *************************
dnl *** Check for headers ***
dnl *************************

BC_CONFIGURE_STAGE([headers], [4 of 9])

AC_HEADER_STDC
dnl AC_CHECK_HEADERS( \
dnl )


dnl **********************************
dnl *** Check for types/structures ***
dnl **********************************

BC_CONFIGURE_STAGE([types], [5 of 9])


dnl ******************************************
dnl *** Check for compiler characteristics ***
dnl ******************************************

BC_CONFIGURE_STAGE([compiler], [6 of 9])

dnl include our include directory
CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/include -I\$(top_srcdir)/botlib/inc"

dnl try to use -pipe to speed up the compiles
pipe_flag="-pipe"
AC_MSG_CHECKING([if the compiler understands $pipe_flag])
pipe_flag_works=yes
PRECFLAGS="$CFLAGS"
PRELDFLAGS="$LDFLAGS"
CFLAGS="$CFLAGS $pipe_flag"
LDFLAGS="$LDFLAGS $pipe_flag"
AC_TRY_COMPILE( [], [], [], [pipe_flag_works=no])
AC_MSG_RESULT($pipe_flag_works)
if test "x$pipe_flag_works" = "xno" ; then
	CFLAGS="$PRECFLAGS"
	LDFLAGS="$PRELDFLAGS"
fi

dnl check for -fno-strict-aliasing
fno_strict_aliasing_flag="-fno-strict-aliasing"
AC_MSG_CHECKING([if the compiler understands $fno_strict_aliasing_flag])
fno_strict_aliasing_flag_works=yes
PRECFLAGS="$CFLAGS"
PRELDFLAGS="$LDFLAGS"
CFLAGS="$CFLAGS $fno_strict_aliasing_flag"
LDFLAGS="$LDFLAGS $fno_strict_aliasing_flag"
AC_TRY_COMPILE( [], [], [], [fno_strict_aliasing_flag_works=no])
AC_MSG_RESULT($fno_strict_aliasing_flag_works)
if test "x$fno_strict_aliasing_flag_works" = "xno" ; then
	CFLAGS="$PRECFLAGS"
	LDFLAGS="$PRELDFLAGS"
fi

dnl check for -search_paths_first linker flag
search_paths_first_flag="-Wl,-search_paths_first"
AC_MSG_CHECKING([if the linker understands $search_paths_first_flag])
search_paths_first_flag_works=yes
PRELDFLAGS="$LDFLAGS"
LDFLAGS="$LDFLAGS $search_paths_first_flag"
AC_TRY_LINK( [], [], [], [search_paths_first_flag_works=no])
AC_MSG_RESULT($search_paths_first_flag_works)
if test "x$search_paths_first_flag_works" = "xno" ; then
	LDFLAGS="$PRELDFLAGS"
fi

dnl optimization flags
if test "x$use_optimized" != "xno" ; then
	O3_flag="-O3"
	AC_MSG_CHECKING([if the compiler understands $O3_flag])
	O3_flag_works=yes
	PRECFLAGS="$CFLAGS"
	PRELDFLAGS="$LDFLAGS"
	CFLAGS="$CFLAGS $O3_flag"
	LDFLAGS="$LDFLAGS $O3_flag"
	AC_TRY_COMPILE( [], [], [], [O3_flag_works=no])
	AC_MSG_RESULT($O3_flag_works)
	if test "x$O3_flag_works" = "xno" ; then
		AC_MSG_WARN([Don't know how to compile optimized with this compiler])
		CFLAGS="$PRECFLAGS"
		LDFLAGS="$PRELDFLAGS"
	fi

	dnl check for a slew of gcc optimizations that may or may not be available with -O3
	extra_optimization_flag="-fno-common -ffast-math -fstrength-reduce -fexpensive-optimizations -fomit-frame-pointer -finline-functions"
	AC_MSG_CHECKING([if the compiler understands extra optimization flags])
	extra_optimization_flag_works=yes
	PRECFLAGS="$CFLAGS"
	PRELDFLAGS="$LDFLAGS"
	CFLAGS="$CFLAGS $extra_optimization_flag"
	LDFLAGS="$LDFLAGS $extra_optimization_flag"
	AC_TRY_COMPILE( [], [], [], [extra_optimization_flag_works=no])
	AC_MSG_RESULT($extra_optimization_flag_works)
	if test "x$extra_optimization_flag_works" = "xno" ; then
		CFLAGS="$PRECFLAGS"
		LDFLAGS="$PRELDFLAGS"
	fi
fi

dnl debug flags
if test "x$use_debug" != "xno" ; then
	g_flag="-g"
	AC_MSG_CHECKING([if the compiler understands $g_flag])
	g_flag_works=yes
	PRECFLAGS="$CFLAGS"
	PRELDFLAGS="$LDFLAGS"
	CFLAGS="$CFLAGS $g_flag"
	LDFLAGS="$LDFLAGS $g_flag"
	AC_TRY_COMPILE( [], [], [], [g_flag_works=no])
	AC_MSG_RESULT($g_flag_works)
	if test "x$g_flag_works" = "xno" ; then
		AC_MSG_WARN([Do not know how to debug with this compiler])
		CFLAGS="$PRECFLAGS"
		LDFLAGS="$PRELDFLAGS"
	fi
fi

dnl profile flags
if test "x$use_profiling" != "xno" ; then
	pg_flag="-pg"
	AC_MSG_CHECKING([if the compiler understands $pg_flag])
	pg_flag_works=yes
	PRECFLAGS="$CFLAGS"
	PRELDFLAGS="$LDFLAGS"
	CFLAGS="$CFLAGS $pg_flag"
	LDFLAGS="$LDFLAGS $pg_flag"
	AC_TRY_COMPILE( [], [], [], [pg_flag_works=no])
	AC_MSG_RESULT($pg_flag_works)
	if test "x$pg_flag_works" = "xno" ; then
		AC_MSG_WARN([Don't know how to profile with this compiler])
		CFLAGS="$PRECFLAGS"
		LDFLAGS="$PRELDFLAGS"
	fi
fi

dnl verbose warning flags
if test "x$use_warnings" != "xno" ; then
	warning_flag="-W -Wall -Wundef -Wfloat-equal -Wshadow -Wunreachable-code -Winline -Wconversion"
		# XXX also of interest
		# -Wmissing-declarations -Wmissing-prototypes -Wstrict-prototypes -pedantic -ansi -Werror
	AC_MSG_CHECKING([if the compiler understands warning flags])
	warning_flag_works=yes
	PRECFLAGS="$CFLAGS"
	PRELDFLAGS="$LDFLAGS"
	CFLAGS="$CFLAGS $warning_flag"
	LDFLAGS="$LDFLAGS $warning_flag"
	AC_TRY_COMPILE( [], [], [], [warning_flag_works=no])
	AC_MSG_RESULT($warning_flag_works)
	if test "x$warning_flag_works" = "xno" ; then
		AC_MSG_WARN([Don't know how to output verbose warnings with this compiler])
		CFLAGS="$PRECFLAGS"
		LDFLAGS="$PRELDFLAGS"
	fi
fi

AC_C_CONST
AC_C_INLINE
AC_C_VOLATILE
AM_C_PROTOTYPES

dnl Last step is to make sure that we can actually compile
BC_SANITY_CHECK


dnl ***************************
dnl *** Check for functions ***
dnl ***************************

BC_CONFIGURE_STAGE([functions], [7 of 9])

dnl AC_CHECK_FUNCS(\
dnl )



dnl *********************************
dnl *** Check for system services ***
dnl *********************************

BC_CONFIGURE_STAGE([services], [8 of 9])



dnl **************************************
dnl *** Configure Makefiles and output ***
dnl **************************************

BC_CONFIGURE_STAGE([output], [9 of 9])

# remove surrounding whitspace
CFLAGS="`echo $CFLAGS`"
CXXFLAGS="`echo $CXXFLAGS`"
CPPFLAGS="`echo $CPPFLAGS`"
LDFLAGS="`echo $LDFLAGS`"
LIBS="`echo $LIBS`"

AM_CONDITIONAL(BUILD_EXAMPLES, [test "x$build_examples" != "xno"])

AC_CONFIG_FILES([
	Makefile
	botlib/Makefile
	botlib/inc/Makefile
	botlib/src/Makefile
	examples/Makefile
	examples/simpleIRCConnect/Makefile
	examples/simpleIRCConnect/src/Makefile
	examples/simpleTCPConnect/Makefile
	examples/simpleTCPConnect/src/Makefile
	examples/spamBot/Makefile
	examples/spamBot/src/Makefile
	examples/stupidBot/Makefile
	examples/stupidBot/src/Makefile
	examples/stupidServer/Makefile
	examples/stupidServer/src/Makefile
	include/Makefile
	m4/Makefile
	misc/Makefile
	src/Makefile
])

AC_OUTPUT

# patch libtool if it has the -all_load bug
BC_PATCH_LIBTOOL

dnl
dnl Expand the variables for summary reporting
dnl
prefix=`eval "echo $prefix"`
prefix=`eval "echo $prefix"`
bindir=`eval "echo $bindir"`
bindir=`eval "echo $bindir"`
sysconfdir=`eval "echo $sysconfdir"`
sysconfdir=`eval "echo $sysconfdir"`
mandir=`eval "echo $mandir"`
mandir=`eval "echo $mandir"`
datadir=`eval "echo $datadir"`
datadir=`eval "echo $datadir"`

dnl
dnl Compute configuration time elapsed
dnl
if test -x "${srcdir}/sh/elapsed.sh" ; then
	time_elapsed="`${srcdir}/sh/elapsed.sh $CONFIG_TIME`"
else
	time_elapsed="unknown"
fi


# AC_OUTPUT won't update the brlcad_config.h.in template timestamp if
# the file is unchanged.  forcibly update the timestamp so autoheader
# doesn't try to invoke.
if test ! "x`cat include/config.h.in`" = "x" ; then
    touch include/config.h.in
fi


dnl **********************
dnl *** Report Summary ***
dnl **********************

AC_MSG_RESULT([Done.])
AC_MSG_RESULT([])
AC_MSG_RESULT([libIRC $LIBIRC_VERSION, Build $CONFIG_DATE])
AC_MSG_RESULT([])
AC_MSG_RESULT([             Prefix: ${prefix}])
AC_MSG_RESULT([           Binaries: ${bindir}])
AC_MSG_RESULT([       Manual pages: ${mandir}])
AC_MSG_RESULT([Configuration files: ${sysconfdir}])
AC_MSG_RESULT([Data resource files: ${datadir}])
if test ! "x$ARGS" = "x" ; then
AC_MSG_RESULT([Options & variables: $ARGS])
fi
AC_MSG_RESULT([])
AC_MSG_RESULT([CC       = ${CC}])
AC_MSG_RESULT([CXX      = ${CXX}])
if test ! "x$CFLAGS" = "x" ; then
AC_MSG_RESULT([CFLAGS   = ${CFLAGS}])
fi
if test ! "x$CXXFLAGS" = "x" ; then
AC_MSG_RESULT([CXXFLAGS = ${CXXFLAGS}])
fi
if test ! "x$CPPFLAGS" = "x" ; then
AC_MSG_RESULT([CPPFLAGS = ${CPPFLAGS}])
fi
if test ! "x$LDFLAGS" = "x" ; then
AC_MSG_RESULT([LDFLAGS  = ${LDFLAGS}])
fi
if test ! "x$LIBS" = "x" ; then
AC_MSG_RESULT([LIBS     = ${LIBS}])
fi
AC_MSG_RESULT([])
AC_MSG_RESULT([Build optimized release ..............: $use_optimized])
AC_MSG_RESULT([Build debug release ..................: $use_debug])
AC_MSG_RESULT([Build profile release ................: $use_profiling])
AC_MSG_RESULT([Build static libraries ...............: $enable_static])
AC_MSG_RESULT([Build shared/dynamic libraries .......: $enable_shared])
AC_MSG_RESULT([Print verbose compilation warnings ...: $use_warnings])
AC_MSG_RESULT([Build examples .......................: $build_examples])
AC_MSG_RESULT([])
if test "x$time_elapsed" != "xunknown" ; then
AC_MSG_RESULT([Elapsed configuration time ...........: $time_elapsed])
fi
AC_MSG_RESULT([---])
AC_MSG_RESULT(["$0 complete, type 'make' to begin building"])
AC_MSG_RESULT([])
