SUBDIRS = m4 . icons po
if HAVE_YELP
SUBDIRS += help
endif

# Tell aclocal where to find `.m4' files.
ACLOCAL_AMFLAGS = -I m4

dist_man_MANS = gobby-0.5.1

bin_PROGRAMS = gobby-0.5

noinst_HEADERS =
gobby_0_5_SOURCES =
BUILT_SOURCES =

EXTRA_DIST =
CLEANFILES =
DISTCLEANFILES =
MAINTAINERCLEANFILES =

include code/Makefile.am
include win32/Makefile.am

gobby_0_5_LDADD = \
	$(gobby_LIBS) \
	$(infinote_LIBS) \
	$(LIBS)

if WIN32
gobby_0_5_LDADD += -lws2_32
gobby_0_5_LDFLAGS = \
	-Wl,--enable-runtime-pseudo-reloc \
	-mwindows \
	win32/gobby.res
endif

# PRIVATE_ICONS_DIR is required by application.cpp to get access to the
# private icons that are installed into prefix/share/gobby-0.5/icons instead
# of prefix/share/icons. We also set PUBLIC_ICONS_DIR explicitely so that
# we find the application icon if it is installed in a non-standard prefix.
AM_CPPFLAGS = $(gobby_CFLAGS) $(infinote_CFLAGS) \
              -I$(top_srcdir)/code \
              -DPRIVATE_ICONS_DIR=\""$(datadir)/gobby-0.5/icons"\" \
              -DPUBLIC_ICONS_DIR=\""$(datadir)/icons"\"

# .desktop file
if !WIN32
desktopdir = $(datadir)/applications
desktop_in_files = gobby-0.5.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

metainfodir = $(datadir)/metainfo
metainfo_DATA = gobby-0.5.metainfo.xml
endif

gsettings_SCHEMAS = de.0x539.gobby.gschema.xml

EXTRA_DIST += de.0x539.gobby.gschema.xml.in
CLEANFILES += de.0x539.gobby.gschema.xml
DISTCLEANFILES += de.0x539.gobby.gschema.xml
MAINTAINERCLEANFILES += $(gsettings_SCHEMAS:.xml=.valid)

@GSETTINGS_RULES@

clean-local:
	-rm -rf gobby-0.5.desktop

@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_XML_RULE@

EXTRA_DIST += README.md

# For the manual
EXTRA_DIST += gobby-0.5.desktop.in gobby-0.5.metainfo.xml.in
DISTCLEANFILES += gnome-doc-utils.make
DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper

MAINTAINERCLEANFILES += \
	ChangeLog

distclean-local:
	if test "$(srcdir)" = "."; then :; else \
		rm -f ChangeLog; \
	fi

ChangeLog:
	$(AM_V_GEN) if test -d "$(srcdir)/.git"; then \
	  (GIT_DIR=$(top_srcdir)/.git ./missing --run git log \
		6570cc49a6c2c29fe9cf4c85e3b74f133ae4c1f8..HEAD --stat=78) \
		| fmt --split-only --width=78 > $@.tmp \
	  && cat $@.tmp ChangeLog.manual ChangeLog.pre-0.5 > $@ && $(RM) $@.tmp \
	  || ($(RM) $@.tmp; \
	      echo Failed to generate ChangeLog, your ChangeLog may be outdated >&2; \
	      (test -f $@ || echo git-log is required to generate this file >> $@)); \
	else \
	  test -f $@ || \
	  (echo A git checkout and git-log is required to generate ChangeLog >&2 && \
	  echo A git checkout and git-log is required to generate this file >> $@); \
	fi

.PHONY: ChangeLog
