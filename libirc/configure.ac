dnl BRL-CAD
dnl Copyright (c) 2004 Christopher Sean Morrison
dnl $Id$
dnl
dnl This program is free software; you can redistribute it and/or
dnl modify it under the terms of the appropriate license described in
dnl the file named COPYING that should have accompanied this file.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
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
dnl to autoconf 2.50.
dnl

AC_PREREQ(2.50)

AC_INIT(libIRC, 0.1, [http://libirc.sf.net], libIRC)
AC_REVISION($Revision$)
AC_CONFIG_AUX_DIR(misc)

dnl If any of the version numbers are incremented up to a 2-digit
dnl number, remember to add an asterisk to both echo lines below.
dnl
dnl See HACKING for details on how to properly update the version
MAJOR_VERSION=0
MINOR_VERSION=1
PATCH_VERSION=0
AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(PATCH_VERSION)

CONFIG_DATE=`date +%Y%m%d`
CONFIG_TIME="`date '+%H %M %S'`"
LIBIRC_VERSION="$MAJOR_VERSION.$MINOR_VERSION.$PATCH_VERSION"
echo "********************************************************"
echo "*** Configuring libIRC Release $LIBIRC_VERSION, Build $CONFIG_DATE ***"
echo "********************************************************"
AC_SUBST(CONFIG_DATE)
AC_SUBST(CONFIG_TIME)
AC_SUBST(LIBIRC_VERSION)

# necessary for proper file creation on nfs volumes
umask 022

# classify this machine
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

# am_init_automake performs a ac_prog_install check so need to handle
# problematic /usr/brl/bin/install on irix
if test "x$build_vendor" = "xsgi" ; then
   PATH="`echo $PATH | sed 's/\/brl\/bin/bin/g'`"
fi

AC_CONFIG_SRCDIR([src/libIRC.cpp])

AC_MSG_CHECKING([whether dependency tracking should be enabled])
if test -d $srcdir/CVS ; then
	AC_MSG_RESULT(it should)
	AM_INIT_AUTOMAKE
else
	AC_MSG_RESULT(it should not)
	AM_INIT_AUTOMAKE(no-dependencies)
fi
AM_CONFIG_HEADER(include/config.h)

# automatically enable and load our configure cache file if available
AC_MSG_CHECKING([whether a configure cache exists])
if test "x$cache_file" = "x/dev/null" ; then
	libirc_cache="config.cache.${host_os}.${ac_hostname}"
	CONFIG_CACHE=""
	if test -f "$libirc_cache"; then
		# if the configure script has been modified since
		# the last caching, assume it to be invalid.
		last_modified="`ls -Lt $libirc_cache configure`"
		case "x$last_modified" in
			xconfigure*)
				AC_MSG_RESULT([found but out of date])
				rm -f $libirc_cache
				;;
			*)
				AC_MSG_RESULT([found $libirc_cache])

				dnl go ahead and load our cache
				case $libirc_cache in
					[\\/]* | ?:[\\/]* )
						. $libirc_cache
						;;
					*)
						. ./$libirc_cache
						;;
				esac
		esac
	else
		AC_MSG_RESULT([not found])
	fi

	dnl if we are on sgi, bash may choke on sed syntax in the cache
	if test "x$host_os" != "xirix6.5" ; then
		AC_MSG_NOTICE([*** Automatically caching to $libirc_cache ***])
		>$libirc_cache
		cache_file="$libirc_cache"
		CONFIG_CACHE="$cache_file"
	else
		AC_MSG_NOTICE([Automatic caching is unavailable on IRIX])
	fi
	AC_SUBST(CONFIG_CACHE)
else
	AC_MSG_RESULT($cache_file)
fi


dnl ***********************
dnl *** Check arguments ***
dnl ***********************

# Allow user to specify flags without relying on environment
AC_ARG_WITH(cflags, AC_HELP_STRING(--with-cflags,
		    [Specify additional flags to pass to the C compiler]) ,
	[
		if test "x$withval" != "xno" ; then
			CFLAGS="$CFLAGS $withval"
		fi
	]	
)
AC_ARG_WITH(cppflags, AC_HELP_STRING(--with-cppflags,
		      [Specify additional flags to pass to C preprocessor]) ,
	[
		if test "x$withval" != "xno"; then
			CPPFLAGS="$CPPFLAGS $withval"
		fi
	]
)
AC_ARG_WITH(ldflags, AC_HELP_STRING(--with-ldflags,
		     [Specify additional flags to pass to linker]) ,
	[
		if test "x$withval" != "xno" ; then
			LDFLAGS="$LDFLAGS $withval"
		fi
	]	
)
AC_ARG_WITH(libs, AC_HELP_STRING(--with-libs,
		  [Specify additional libraries to link against]) ,
	[
		if test "x$withval" != "xno" ; then
			LIBS="$LIBS $withval"
		fi
	]	
)

# doxygen
AC_MSG_CHECKING(whether to generate documentation with doxygen)
use_doxygen=no
AC_ARG_ENABLE(doxygen,
	AC_HELP_STRING(--enable-doxygen,
		[enable documentation generation with doxygen (default=no)]),
	[if test "xenable_doxygen" != "xno" ; then
		use_doxygen=yes
	fi]
)
AC_MSG_RESULT($use_doxygen)

# optimized
dnl optimized has alias(es)
use_optimized=no
AC_ARG_ENABLE(optimized,
	AC_HELP_STRING(--enable-optimized,
		[enable optimized compilation (default=no)]),
	[if test "x$enable_optimized" != "xno" ; then
		use_optimized=yes
	fi]
)

# warnings
dnl warnings has alias(es)
use_warnings=no
AC_ARG_ENABLE(warnings,
	AC_HELP_STRING(--enable-warnings,
		[enable verbose compilation warnings (default=no)]),
	[if test "x$enable_warnings" != "xno" ; then
		use_warnings=yes
	fi]
)

# debug
dnl debug has alias(es)
use_debug=yes
AC_ARG_ENABLE(debug,
	AC_HELP_STRING(--disable-debug,
		[disable debug symbols (default=no)]),
	[if test "x$disable_debug" != "xno" ; then
		use_debug=no
	fi]
)

# profiling
dnl profiling has alias(es)
use_profiling=no
AC_ARG_ENABLE(profiling,
	AC_HELP_STRING(--enable-profiling,
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


dnl XXX should only add to build paths when something fails that is
dnl necessary additionally, a compiler might complain if /usr/local or
dnl /sw are already in the system search path -- so need to try a
dnl compile test..

dnl autosearch /usr/local
AC_MSG_CHECKING([for /usr/local])
if test -d /usr/local/include ; then
	AC_MSG_RESULT([found, adding /usr/local to search paths])
	CPPFLAGS="$CPPFLAGS -I/usr/local/include"
	if test -d /usr/local/lib ; then
		LDFLAGS="$LDFLAGS -L/usr/local/lib"
	fi
else
	AC_MSG_RESULT([not found])
fi

dnl autosearch fink paths
AC_MSG_CHECKING([for fink in /sw])
if test -d /sw/include ; then
	AC_MSG_RESULT([found, adding /sw to search paths])
	CPPFLAGS="$CPPFLAGS -I/sw/include"
	if test -d /sw/lib ; then
		LDFLAGS="$LDFLAGS -L/sw/lib"
	fi
else
	AC_MSG_RESULT([not found])
fi


# override the default autoconf cflags if user has not modified them
if test "x$CFLAGS" = "x" ; then
	# an unset CFLAGS var is set to "-O2 -g" during AC_PROG_CC, so
	# set it to something benign
	CFLAGS=" "
fi


dnl **************************
dnl *** Check for programs ***
dnl **************************

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

if test "x$use_doxygen" = "xyes" ; then
	AC_PATH_PROG(DOXYGEN, doxygen, , $PATH)
	if test "x$DOXYGEN" = "x" ; then
		AC_MSG_NOTICE([>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>])
		AC_MSG_ERROR([*** Unable to find doxygen ***])
	fi
fi
AC_MSG_CHECKING(whether to enable doxygen)
AC_MSG_RESULT($use_doxygen)


dnl ***************************
dnl *** Check for libraries ***
dnl ***************************



dnl *************************
dnl *** Check for headers ***
dnl *************************

AC_HEADER_STDC
dnl AC_CHECK_HEADERS( \
dnl )


dnl **********************************
dnl *** Check for types/structures ***
dnl **********************************



dnl ******************************************
dnl *** Check for compiler characteristics ***
dnl ******************************************

dnl include our include directory
CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/include"

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
AC_MSG_CHECKING(compiler and flags for sanity)
AC_TRY_RUN([
#include <stdio.h>
int main(){exit(0);}
	],
	[	AC_MSG_RESULT(yes) ],
	[
		AC_MSG_RESULT(no)
		AC_MSG_NOTICE([>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>])
		AC_MSG_ERROR([*** compiler cannot create working executables, check config.log ***])
	]
)


dnl ***************************
dnl *** Check for functions ***
dnl ***************************

dnl AC_CHECK_FUNCS(\
dnl )



dnl *********************************
dnl *** Check for system services ***
dnl *********************************



dnl ***************************
dnl *** Configure Makefiles ***
dnl ***************************

AC_CONFIG_FILES([
	Makefile
	include/Makefile
	src/Makefile
	src/sdl_net/Makefile
])

AC_OUTPUT

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

dnl
dnl Compute configuration time elapsed
dnl
if test -x "${srcdir}/sh/elapsed.sh" ; then
	time_elapsed="`${srcdir}/sh/elapsed.sh $CONFIG_TIME`"
else
	time_elapsed="unknown"
fi


dnl **********************
dnl *** Report Summary ***
dnl **********************

echo "Done."
echo
echo "libIRC $LIBIRC_VERSION, Build $CONFIG_DATE"
echo
echo "             Prefix: ${prefix}"
echo "           Binaries: ${bindir}"
echo "Configuration files: ${sysconfdir}"
echo "       Manual pages: ${mandir}"
echo
echo "CC       = ${CC}"
echo "CFLAGS   = ${CFLAGS}"
echo "CPPFLAGS = ${CPPFLAGS}"
echo "LDFLAGS  = ${LDFLAGS}"
echo "LIBS     = ${LIBS}"
echo
echo Build optimized release ..............: $use_optimized
echo Build debug release ..................: $use_debug
echo Build profile release ................: $use_profiling
echo Build static libraries ...............: $enable_static
echo Build shared/dynamic libraries .......: $enable_shared
echo Print verbose compilation warnings ...: $use_warnings
echo Generate doxygen documentation .......: $use_doxygen
echo
if test "x$time_elapsed" != "xunknown" ; then
echo Elapsed configuration time ...........: $time_elapsed
fi
echo ---
echo "$0 complete, type 'make' to begin building"
echo
