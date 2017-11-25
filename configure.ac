dnl                    Grapher : GRAPH EXPLORE SOFTWARE
dnl                   Find protein higher-degree associations from STRING-DB data
dnl
dnl                        by Andreas Hadjiprocopis
dnl                        (andreashad2@gmail.com)
dnl              Copyright Andreas Hadjiprocopis, ICR, 2012
dnl AX_CC_MAXOPT macros (and all contents of the m4 directory)
dnl were 'borrowed' from fftw-3.1 by Matteo Frigo and Steven G. Johnson (thanks!)
dnl look at :  http://www.fftw.org/mailman/listinfo/fftw-announce

DEBUG_CXX=g++
DEBUG_CC=gcc
NORMAL_CXX=g++
NORMAL_CC=gcc
# the compiler will be decided later when we see the +/- debug switch, for the time use this
CXX=${NORMAL_CXX}
CC=${NORMAL_CC}

#MY_CFLAGS='-finline-functions -funroll-all-loops -O3 -Wall -ansi -pedantic -W' # will be added later
#MY_CFLAGS='-march=core2 -mtune=generic -O3 -pipe -Wall -W' # will be added later
# the best is this with g++-fsf-4.6
MY_CFLAGS='-Wall -W' # will be added later
#MY_CFLAGS='-O3 -Wall -ansi -pedantic -W' # will be added later
MY_CXXFLAGS=${MY_CFLAGS}

################
dnl for these see at the end of file as well
AC_INIT([Grapher], [4.2], [andreashad2@gmail.com])
AC_CANONICAL_SYSTEM
AC_CANONICAL_TARGET
AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/C/graphos.cpp])
AM_INIT_AUTOMAKE()

LT_INIT
# LT_INIT adds a -g -O2 to the CFLAGS, what the f???
# this hack is stupid, any cflags from user will be ignroed
CFLAGS=
CXXFLAGS=

m4_define([glib_required_version],[2.0.0])
m4_define([boost_required_version],[1.49.0])

m4_pattern_allow([AC_CONFIG_MACRO])
m4_pattern_allow([AS_AC_EXPAND])
m4_pattern_allow([AX_CC_MAXOPT])
m4_pattern_allow([AX_BOOST_BASE])
m4_pattern_allow([AM_PATH_GLIB_2_0])
m4_pattern_allow([AC_MSG_ERROR])
m4_pattern_allow([AC_MSG_WARN])
m4_pattern_allow([AM_PROG_LIBTOOL])

echo "You have host: ${host}, cpu: ${host_cpu}, os: ${host_os}, vendor: ${host_vendor}"
echo "Target : ${target}, cpu: ${target_cpu}, os: ${target_os}, vendor: ${target_vendor}"

#AC_PROG_RANLIB
AC_PROG_LIBTOOL

#AM_GNU_GETTEXT([external])
#AM_GNU_GETTEXT_VERSION(0.15)
#AM_ICONV

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string])
AC_CHECK_HEADERS([iostream])
AC_CHECK_HEADER(tr1/unordered_map,[AC_DEFINE([HAVE_TR1],[],["Have tr1"])],[])
AC_CHECK_HEADER(unordered_map,[AC_DEFINE([HAVE_CXX0X],[],["Have C++0x"])],[])

# debug
AC_ARG_ENABLE(debug, [AC_HELP_STRING([--enable-debug],[disable ALL optimisations and use -g flag for compilation (produces executable for to be inspected by gdb - unlikely to be used by regular user)])], [debug=$enableval], [debug=no])
AM_CONDITIONAL(DEBUG, test "$debug" = "yes")

# input data file consistency checks warnings
AC_ARG_ENABLE(data_file_consistency_checks, [AC_HELP_STRING([--enable-data_file_consistency_checks],[keep any optimisations but check input data file consistency, e.g. are all edges mentioned in the edges data file are also contained in the input edges file?)])], [data_file_consistency_checks=$enableval], [data_file_consistency_checks=no])
AM_CONDITIONAL(DATA_FILE_CONSISTENCY_CHECKS, test "$data_file_consistency_checks" = "yes")

# optimisations, these will not have an effect if the --enable-debug above is set
AC_ARG_ENABLE(sse, [AC_HELP_STRING([--disable-sse],[disable SSE optimizations])], have_sse=$enableval, have_sse=yes)
if test "$have_sse" = "yes"; then have_sse2="yes"; fi

AC_ARG_ENABLE(sse2, [AC_HELP_STRING([--disable-sse2],[disable SSE2 optimizations])], have_sse2=$enableval, have_sse2=yes)
AM_CONDITIONAL(HAVE_SSE2, test "$have_sse2" = "yes")

AC_ARG_ENABLE(altivec, [AC_HELP_STRING([--disable-altivec],[disable Altivec optimizations])], have_altivec=$disableval, have_altivec=yes)
AM_CONDITIONAL(HAVE_ALTIVEC, test "$have_altivec" = "yes")

AC_ARG_ENABLE(pipe, [AC_HELP_STRING([--disable-pipe],[disable pipe optimizations])], have_pipe=$disableval, have_pipe=yes)
AM_CONDITIONAL(HAVE_PIPE, test "$have_pipe" = "yes")

AC_ARG_ENABLE(ftree-vectorize, [AC_HELP_STRING([--disable-ftree-vectorize],[disable Tree Vectorize optimizations])], have_ftree_vectorize=$enableval, have_ftree_vectorize=yes)
AM_CONDITIONAL(HAVE_TREE_VECTORIZE, test "$have_ftree_vectorize" = "yes")

AC_ARG_ENABLE(avx, [AC_HELP_STRING([--enable-avx],[enable AVX optimizations])], have_avx=$enableval, have_avx=no)
AM_CONDITIONAL(HAVE_AVX, test "$have_avx" = "yes")

AC_ARG_ENABLE(neon, [AC_HELP_STRING([--enable-neon],[enable ARM NEON optimizations])], have_neon=$enableval, have_neon=no)
AM_CONDITIONAL(HAVE_NEON, test "$have_neon" = "yes")

# don't do the optimisation flags if on debug mode
if test "$debug" = "no"; then
	CXX=${NORMAL_CXX}
	CC=${NORMAL_CC}
	# check for C++ preprocessor and compiler and the library compiler
	AC_PROG_CXX
#	AC_PROG_CPP
	AC_PROG_CC

	AX_CC_MAXOPT
	case "${ax_cv_c_compiler_vendor}" in
	    gnu|intel)
		# SSE/SSE2
		if test "$have_sse2" = "yes" -a "x$SSE2_CFLAGS" = x; then
		    flag="msse2"
		    AX_CHECK_COMPILER_FLAGS(-$flag, [SSE2_CFLAGS="-$flag"],
			[AC_MSG_WARN([Need a version of gcc with -$flag])])
		fi

		# Tree vectorise
		if test "$have_ftree_vectorize" = "yes" -a "x$TREE_VECTORIZE_CFLAGS" = x; then
		    AX_CHECK_COMPILER_FLAGS(-ftree-vectorize, [TREE_VECTORIZE_CFLAGS="-ftree-vectorize"],
			[AC_MSG_WARN([Need a version of gcc with -ftree-vectorize])])
		fi

		# -pipe
		if test "$have_pipe" = "yes" -a "x$PIPE_CFLAGS" = x; then
		    AX_CHECK_COMPILER_FLAGS(-pipe, [PIPE_CFLAGS="-pipe"],
			[AC_MSG_WARN([Need a version of gcc with -pipe])])
		fi

		# AVX
		if test "$have_avx" = "yes" -a "x$AVX_CFLAGS" = x; then
		    AX_CHECK_COMPILER_FLAGS(-mavx, [AVX_CFLAGS="-mavx"],
			[AC_MSG_WARN([Need a version of gcc with -mavx])])
		fi

		if test "$have_altivec" = "yes" -a "x$ALTIVEC_CFLAGS" = x; then
		    # -DFAKE__VEC__ is a workaround because gcc-3.3 does not
		    # #define __VEC__ with -maltivec.
		    AX_CHECK_COMPILER_FLAGS(-faltivec, [ALTIVEC_CFLAGS="-faltivec"],
			[AX_CHECK_COMPILER_FLAGS(-maltivec -mabi=altivec,
			    [ALTIVEC_CFLAGS="-maltivec -mabi=altivec -DFAKE__VEC__"],
			    [AX_CHECK_COMPILER_FLAGS(-fvec, [ALTIVEC_CFLAGS="-fvec"],
				[AC_MSG_WARN([Need a version of gcc with -maltivec])])])])
		fi

		if test "$have_neon" = "yes" -a "x$NEON_CFLAGS" = x; then
		    AX_CHECK_COMPILER_FLAGS(-mfpu=neon, [NEON_CFLAGS="-mfpu=neon"],
			[AC_MSG_WARN([Need a version of gcc with -mfpu=neon])])
		fi
	esac
	AC_SUBST(SSE2_CFLAGS)
	AC_SUBST(PIPE_CFLAGS)
	AC_SUBST(TREE_VECTORIZE_CFLAGS)
	AC_SUBST(AVX_CFLAGS)
	AC_SUBST(ALTIVEC_CFLAGS)
	AC_SUBST(NEON_CFLAGS)
	OPTIMISATION_DETECTED="-O12 ${SSE2_CFLAGS} ${TREE_VECTORIZE_CFLAGS} ${AVX_CFLAGS} ${ALTIVEC_CFLAGS} ${NEON_CFLAGS} ${PIPE_CFLAGS}"
	AC_SUBST(OPTIMISATION_DETECTED)
else
	CXX=${DEBUG_CXX}
	CC=${DEBUG_CC}
	# check for C++ preprocessor and compiler and the library compiler
	AC_PROG_CXXCPP
	AC_PROG_CXX
#	AC_PROG_CPP
	AC_PROG_CC
fi

CFLAGS="${MY_CFLAGS} ${CFLAGS}"
CXXFLAGS="${MY_CXXFLAGS} ${CXXFLAGS}"

AM_COND_IF([DEBUG],
# use the above compiler for mac
	[
		CFLAGS="${CFLAGS} -g"
		CXXFLAGS="${CXXFLAGS} -g"
		echo "DEBUG selected : OPTIMISATIONS * USED *: ${OPTIMISATION_FLAGS}"
		echo "DEBUG selected : CFLAGS USED: $CFLAGS"
		echo "DEBUG selected : CXXFLAGS USED: $CXXFLAGS"
	],[	CFLAGS="${CFLAGS} ${OPTIMISATION_DETECTED}"
		CXXFLAGS="${CXXFLAGS} ${OPTIMISATION_DETECTED}"
		echo "OPTIMISATIONS * USED *: ${OPTIMISATION_FLAGS}"
		echo "CFLAGS USED: $CFLAGS"
		echo "CXXFLAGS USED: $CXXFLAGS"
	]
)
AM_COND_IF([DATA_FILE_CONSISTENCY_CHECKS],
[
	CFLAGS="${CFLAGS} -DDATA_FILE_CONSISTENCY_CHECKS=1"
	CXXFLAGS="${CXXFLAGS} -DDATA_FILE_CONSISTENCY_CHECKS=1"
],[echo]
)

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AX_BOOST_BASE([boost_required_version],,[AC_MSG_ERROR([boost libraries ][boost_required_version][ or newer is required])])
AC_SUBST(BOOST_CXXFLAGS)
AC_SUBST(BOOST_LDFLAGS)

PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.24.1])

#AM_PATH_GLIB_2_0([glib_required_version],,[AC_MSG_ERROR([glib ][glib_required_version][ or newer is required])])

AC_SUBST(AUTHOR, "Andreas Hadjiprocopis (andreashad2\@gmail.com)")
AC_SUBST(BUILTDATE, `date`)
AC_SUBST(TITLE, "Graph-searcher for STRING DB")
AC_SUBST(DATESTARTED, "Nov 2010")
AC_SUBST(SEARCHPATH, $PATH)

AC_CONFIG_FILES([src/C/graphos.pc])

AC_OUTPUT(
	Makefile
	src/Makefile
	src/C/Makefile
)

#AC_MSG_WARN([CFLAGS ${CFLAGS}])
#AC_MSG_WARN([CXXFLAGS ${CXXFLAGS}])
#AC_MSG_WARN([LDFLAGS ${LDFLAGS}])

