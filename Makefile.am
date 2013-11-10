SUBDIRS = m4 win32 code . icons po

if HAVE_GNOME_DOC_UTILS
SUBDIRS += help
endif

# Tell aclocal where to find `.m4' files.
ACLOCAL_AMFLAGS = -I m4

dist_man_MANS = gobby-0.5.1

bin_PROGRAMS = gobby-0.5

gobby_0_5_LDADD = \
	code/libgobby.a \
	code/commands/libgobby-commands.a \
	code/commands/file-tasks/libgobby-file-tasks.a \
	code/operations/libgobby-operations.a \
	code/dialogs/libgobby-dialogs.a \
	code/core/libgobby-core.a \
	code/util/libgobby-util.a \
	$(gobby_LIBS) \
	$(unique_LIBS) \
	$(infinote_LIBS) \
	$(LIBS)

if WIN32
gobby_0_5_LDADD += -lws2_32
gobby_0_5_LDFLAGS = \
	-Wl,--enable-runtime-pseudo-reloc \
	-mwindows \
	win32/gobby.res
endif

gobby_0_5_SOURCES = 

# .desktop file
if !WIN32
desktopdir = $(datadir)/applications
desktop_in_files = gobby-0.5.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

appdatadir = $(datadir)/appdata
appdata_DATA = gobby-0.5.appdata.xml
endif

clean-local:
	-rm -rf gobby-0.5.desktop

@INTLTOOL_DESKTOP_RULE@

# Dummy C++ source to cause C++ linking, otherwise some platforms don't link
# correctly against libstdc++. See also #399. Note that this file does not
# (need to) exist in the source tree.
nodist_EXTRA_gobby_0_5_SOURCES = dummy.cpp

# Debian seems to need an XPM of the program icon, so we provide one here.
# It's currently not used for anything else, though.
EXTRA_DIST = gobby-0.5.xpm

# For the manual
EXTRA_DIST += gnome-doc-utils.make gobby-0.5.desktop.in
DISTCLEANFILES = gnome-doc-utils.make
DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper

MAINTAINERCLEANFILES = \
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
