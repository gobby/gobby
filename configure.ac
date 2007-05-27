#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([gobby], [0.4.3], [crew@0x539.de])
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

required_libs="libxml++-2.6 gtkmm-2.4 >= 2.6.0 gthread-2.0 net6-1.3 obby-0.4 >= 0.4.1"

# GtkSourceView2 support
AC_ARG_WITH([gtksourceview2],
            AS_HELP_STRING([--with-gtksourceview2],
               [whether to use gtksourceview2 instead of
	        gtksourceview1 [[default=no]]]),
            [ac_cv_with_gtksourceview2=$withval],
	    [ac_cv_with_gtksourceview2=no])

AC_CACHE_CHECK([whether to use gtksourceview2 instead of gtksourceview1],
               [ac_cv_with_gtksourceview2], [ac_cv_with_gtksourceview2=no])

if test "x$ac_cv_with_gtksourceview2" = "xyes" ; then
  AC_DEFINE([WITH_GTKSOURCEVIEW2], 1,
    [Define whether to use gtksourceview2 instead of gtksourceview1])
  required_libs+=" gtksourceview-2.0"
else
  required_libs+=" gtksourceview-1.0"
fi

AM_CONDITIONAL(WITH_GTKSOURCEVIEW2, test x$ac_cv_with_gtksourceview2 != xno)
AC_CACHE_SAVE

# Checks for libraries.
PKG_CHECK_MODULES([gobby], [$required_libs])
AC_CACHE_SAVE

# Check for the Gtkmm version available (either 2.6 or 2.8).
PKG_CHECK_GTKMM
if test "x$GTKMM_VER" = "x28" ; then
  AC_DEFINE([GTKMM_GEQ_28], 1, [Defined if Gtkmm's version is 2.8 or greater])
fi
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
  PKG_CHECK_MODULES([gnome], [gnome-vfs-2.0])
fi
AM_CONDITIONAL(WITH_GNOME, test x$ac_cv_with_gnome != xno)
AC_CACHE_SAVE

# Check for Zeroconf support in obby.
PKG_CHECK_MODULES([obby], [obby-0.4])
AC_CHECK_LIB(obby, obby_has_zeroconf, [obby_has_zeroconf=yes],
             [obby_has_zeroconf=no], [$pkg_cv_obby_LIBS])
if test "x$ac_cv_lib_obby_obby_has_zeroconf" = "xyes" ; then
	# obby's pkg-config script takes care of any additional dependencies
	AC_DEFINE([WITH_ZEROCONF], 1, [Define whether to use Zeroconf])
fi
AM_CONDITIONAL(WITH_ZEROCONF, test x$ac_cv_lib_obby_obby_has_zeroconf = xyes)

# Check for a specific Zeroconf support in obby, namely Avahi.
AC_CHECK_LIB(obby, obby_has_avahi, [obby_has_avahi=yes],
             [obby_has_avahi=no], [$pkg_cv_obby_LIBS])
if test "x$ac_cv_lib_obby_obby_has_avahi" = "xyes" ; then
	AC_DEFINE([WITH_AVAHI], 1,
	          [Define whether to use Avahi as Zeroconf backend])
	PKG_CHECK_MODULES([avahi], [avahi-glib])
fi
AM_CONDITIONAL(WITH_AVAHI, test x$ac_cv_lib_obby_obby_has_avahi = xyes)

# Check if we are running on OS X, for special link handling.
if test "x$build_vendor" = "xapple" ; then
  AC_DEFINE([OSX], 1, [Define whether we are running on Mac OS X.])
fi
AC_CACHE_SAVE

# Glib i18n/gettext support.
ALL_LINGUAS="ca de es fr pt sv zh_CN"
AM_GLIB_GNU_GETTEXT

GETTEXT_PACKAGE=$PACKAGE
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["${GETTEXT_PACKAGE}"], [gettext domain])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT([po/Makefile.in m4/Makefile])

