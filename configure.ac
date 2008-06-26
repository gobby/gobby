#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([gobby], [0.4.90], [crew@0x539.de])
AM_INIT_AUTOMAKE(1.9 check-news)
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([inc/window.hpp])
AC_CONFIG_HEADER([inc/features.hpp])

# Extract host information.
AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AM_PROG_CC_C_O
AC_CACHE_SAVE

AC_MSG_CHECKING([whether to enable WIN32 specific flags])
case "$host_os" in
*mingw*)
        win32=true
        AC_MSG_RESULT([yes])
	AC_CHECK_TOOL(WINDRES, windres)
	;;
*)
        win32=false
        AC_MSG_RESULT([no])
	;;
esac
AM_CONDITIONAL(WIN32, test x$win32 = xtrue)

required_libs="libxml++-2.6 glibmm-2.4 >= 2.16.0 giomm-2.4 >= 2.16.0 gtkmm-2.4 >= 2.12.0 gthread-2.0 gtksourceview-2.0 libinfinity-1.0 libinftext-1.0 libinfgtk-1.0 libinftextgtk-1.0"

# Check if we are running on OS X, for special link handling.
if test "x$build_vendor" = "xapple" ; then
  AC_DEFINE([OSX], 1, [Define whether we are running on Mac OS X.])
  target=`pkg-config --variable=target gtk+-2.0`
  if test "$target" = "quartz" ; then
    required_libs="$required_libs ige-mac-integration"
  fi
fi
AC_CACHE_SAVE

# Checks for libraries.
PKG_CHECK_MODULES([gobby], [$required_libs])
AC_CACHE_SAVE

# As we could not yet auto-detect the GNOME desktop environment, it's the
# user's choice to enable GNOME integration or not.
AC_ARG_WITH([gnome],
            AS_HELP_STRING([--with-gnome],
               [integrate into the GNOME desktop environment [[default=no]]]),
            [ac_cv_with_gnome=$withval], [ac_cv_with_gnome=no])
AC_CACHE_CHECK([whether to integrate into the GNOME desktop environment],
               [ac_cv_with_gnome], [ac_cv_with_gnome=no])
if test "x$ac_cv_with_gnome" = "xyes" ; then
  AC_DEFINE([WITH_GNOME], 1,
    [Define whether to integrate into the GNOME desktop environment.])
fi
AM_CONDITIONAL(WITH_GNOME, test x$ac_cv_with_gnome != xno)
AC_CACHE_SAVE

# Glib i18n/gettext support.
ALL_LINGUAS="ca de es fr ko it pl pt pt_BR ru sv zh_CN"
AM_GLIB_GNU_GETTEXT

GETTEXT_PACKAGE=$PACKAGE
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["${GETTEXT_PACKAGE}"], [gettext domain])

AC_OUTPUT([
	Makefile
		m4/Makefile
		po/Makefile.in

		inc/Makefile
			inc/util/Makefile
			inc/core/Makefile
			inc/dialogs/Makefile
			inc/operations/Makefile
			inc/commands/Makefile
		src/Makefile
			src/util/Makefile
			src/core/Makefile
			src/dialogs/Makefile
			src/operations/Makefile
			src/commands/Makefile

		contrib/Makefile
			contrib/artwork/Makefile
		pixmaps/Makefile
		win32/Makefile
])

