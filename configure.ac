#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([gobby], [0.4.95], [crew@0x539.de])
AM_INIT_AUTOMAKE(1.9 check-news)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES])
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([code/window.hpp])
AC_CONFIG_HEADER([code/features.hpp])

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

###################################
# Check for Win32
###################################

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

###################################
# Check for GTK+ 3
###################################

AC_ARG_WITH([gtk3], AS_HELP_STRING([--with-gtk3],
            [Builds gobby against GTK+ 3 instead of GTK+ 2[[default=no]]]),
	    [with_gtk3=$withval], [with_gtk3=no])

if test "x$with_gtk3" = "xyes"; then
	# TODO: Increase to 2.99.2 once it's out
	gtk_requirement="gtkmm-3.0 >= 2.99.1 gtksourceview-3.0 >= 2.91.0"
	gtk_version=3
else
	gtk_requirement="gtkmm-2.4 >= 2.12.0 gtksourceview-2.0 >= 2.4"
	gtk_version=2
fi

required_libs="libxml++-2.6 glibmm-2.4 >= 2.18.0 giomm-2.4 >= 2.18.0 gthread-2.0 libgsasl >= 0.2.21 $gtk_requirement"

###################################
# Check for Mac OS X
###################################

# Check if we are running on OS X, for special link handling.
if test "x$build_vendor" = "xapple" ; then
  AC_DEFINE([PLATFORM_OSX], 1, [Define whether we are running on Mac OS X.])

  # Check whether we are running the native quartz backend
  if test "$gtk_version" == "2"; then
    target=`pkg-config --variable=target gtk+-2.0`
  else
    target=`pkg-config --variable=targets gtk+-3.0`
  fi

  # If so use ige-mac-integration
  if test "$target" = "quartz" ; then
    AC_DEFINE([PLATFORM_OSX_NATIVE], 1, [Define whether we are running on
                                         Mac OS X with the quartz GDK target])
    required_libs="$required_libs ige-mac-integration"
  fi
fi
AC_CACHE_SAVE

###################################
# Check for libraries.
###################################

PKG_CHECK_MODULES([gobby], [$required_libs])
PKG_CHECK_MODULES([infinote], [libinfinity-0.6 libinftext-0.6 libinfgtk-0.6 libinftextgtk-0.6])
AC_CACHE_SAVE

###################################
# Check libinfinity GTK+ version
###################################

inf_gtk_version=`pkg-config --variable=gtk_version libinfgtk-0.6`
if test "$inf_gtk_version" != "$gtk_version"; then
	AC_MSG_ERROR([Gobby is being built with GTK+ $gtk_version but libinfinity was built against GTK+ $inf_gtk_version])
fi

###################################
# Check for unique (optional)
###################################

AC_ARG_WITH([unique], AS_HELP_STRING([--with-unique],
            [Enables support for single-instance [[default=auto]]]),
            [use_unique=$withval], [use_unique=auto])

if test "$with_gtk3" != "yes"; then
  unique_req="unique-1.0 >= 1.1.2"
else
  unique_req="unique-3.0"
fi

if test "x$use_unique" = "xauto"; then
  PKG_CHECK_MODULES([unique], [$unique_req], [use_unique=yes], [use_unique=no])
elif test "x$use_unique" = "xyes"; then
  PKG_CHECK_MODULES([unique], [$unique_req])
fi

if test "x$use_unique" = "xyes"; then
  AC_DEFINE([WITH_UNIQUE], 1, [Whether support for unique is enabled])
fi

AM_CONDITIONAL([WITH_UNIQUE], test "x$use_unique" = "xyes")
AC_CACHE_SAVE

###################################
# Glib i18n/gettext support.
###################################
ALL_LINGUAS="`grep -v '^#' "$srcdir/po/LINGUAS" | tr '\n' ' '`"
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR(GOBBY_LOCALEDIR)
IT_PROG_INTLTOOL([0.35.0])

GETTEXT_PACKAGE=gobby05
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["${GETTEXT_PACKAGE}"], [gettext domain])

###################################
# Help file building
###################################
GNOME_DOC_INIT([0.9.0],,
  [AC_MSG_WARN([[gnome-doc-utils not found: documentation will not be built.]])])

# AC_DEFINE(GOBBY_CONFIGDIR, [".gobby-0.5"],
#           [where to store Gobby's config files,
#            relative to the user's home directory])

###################################
# Output
###################################

AC_OUTPUT([
  Makefile
    m4/Makefile
    po/Makefile.in

    code/Makefile
      code/util/Makefile
      code/core/Makefile
      code/dialogs/Makefile
      code/operations/Makefile
      code/commands/Makefile
        code/commands/file-tasks/Makefile

    icons/Makefile
      icons/hicolor/Makefile
        icons/hicolor/48x48/Makefile
          icons/hicolor/48x48/apps/Makefile
          icons/hicolor/48x48/actions/Makefile
          icons/hicolor/48x48/status/Makefile
        icons/hicolor/scalable/Makefile
          icons/hicolor/scalable/apps/Makefile
          icons/hicolor/scalable/actions/Makefile
          icons/hicolor/scalable/status/Makefile
      icons/HighContrastLargePrint/Makefile
        icons/HighContrastLargePrint/48x48/Makefile
          icons/HighContrastLargePrint/48x48/apps/Makefile
          icons/HighContrastLargePrint/48x48/actions/Makefile
        icons/HighContrastLargePrint/scalable/Makefile
          icons/HighContrastLargePrint/scalable/apps/Makefile
          icons/HighContrastLargePrint/scalable/actions/Makefile
      icons/HighContrastLargePrintInverse/Makefile
        icons/HighContrastLargePrintInverse/48x48/Makefile
          icons/HighContrastLargePrintInverse/48x48/apps/Makefile
          icons/HighContrastLargePrintInverse/48x48/actions/Makefile
        icons/HighContrastLargePrintInverse/scalable/Makefile
          icons/HighContrastLargePrintInverse/scalable/apps/Makefile
          icons/HighContrastLargePrintInverse/scalable/actions/Makefile

    help/Makefile

    win32/Makefile
])

# vim:set et sw=2 ts=2:
