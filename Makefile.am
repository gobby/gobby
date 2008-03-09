SUBDIRS = m4 . po

# Tell aclocal where to find `.m4' files.
ACLOCAL_AMFLAGS = -I m4

dist_man_MANS = gobby.1

bin_PROGRAMS = gobby

gobby_LDADD     = $(gobby_LIBS) $(gtksourceview_LIBS)
gobby_CPPFLAGS  = $(gobby_CFLAGS) $(gtksourceview_CFLAGS)
gobby_CPPFLAGS += -I../inc/ -DLOCALE_DIR=\""$(datadir)/locale"\"

if WITH_AVAHI
gobby_LDADD += $(avahi_LIBS)
gobby_CPPFLAGS += $(avahi_CFLAGS)
endif

if WITH_GNOME
gobby_LDADD += $(gnome_LIBS)
gobby_CPPFLAGS += $(gnome_CFLAGS)
endif

if WIN32
gobby_LDADD += -lole32 -luuid
gobby_LDFLAGS = -Wl,--enable-runtime-pseudo-reloc -mwindows win32/gobby.res
endif

noinst_HEADERS = inc/icon.hpp
if WIN32
else
noinst_HEADERS += inc/unix.hpp
endif
noinst_HEADERS += inc/gselector.hpp
noinst_HEADERS += inc/browser.hpp
noinst_HEADERS += inc/ipc.hpp
noinst_HEADERS += inc/colorutil.hpp
noinst_HEADERS += inc/config.hpp
noinst_HEADERS += inc/common.hpp
noinst_HEADERS += inc/encoding.hpp
noinst_HEADERS += inc/resolv.hpp
noinst_HEADERS += inc/regex.hpp
noinst_HEADERS += inc/application_state.hpp
noinst_HEADERS += inc/encoding_selector.hpp
noinst_HEADERS += inc/historyentry.hpp
noinst_HEADERS += inc/fileentry.hpp
noinst_HEADERS += inc/logview.hpp
noinst_HEADERS += inc/colorsel.hpp
noinst_HEADERS += inc/preferences.hpp
noinst_HEADERS += inc/toolwindow.hpp
noinst_HEADERS += inc/togglewindow.hpp
noinst_HEADERS += inc/header.hpp
noinst_HEADERS += inc/statusbar.hpp
noinst_HEADERS += inc/document.hpp
noinst_HEADERS += inc/buffer_def.hpp
noinst_HEADERS += inc/docwindow.hpp
noinst_HEADERS += inc/folder.hpp
noinst_HEADERS += inc/document_settings.hpp
noinst_HEADERS += inc/chat.hpp
noinst_HEADERS += inc/preferencesdialog.hpp
noinst_HEADERS += inc/initialdialog.hpp
noinst_HEADERS += inc/passworddialog.hpp
noinst_HEADERS += inc/hostdialog.hpp
noinst_HEADERS += inc/joindialog.hpp
noinst_HEADERS += inc/progressdialog.hpp
noinst_HEADERS += inc/joinprogressdialog.hpp
noinst_HEADERS += inc/hostprogressdialog.hpp
noinst_HEADERS += inc/entrydialog.hpp
noinst_HEADERS += inc/finddialog.hpp
noinst_HEADERS += inc/gotodialog.hpp
noinst_HEADERS += inc/dragdrop.hpp
noinst_HEADERS += inc/window.hpp 

if WIN32
win32/gobby.res: win32/gobby.rc
	$(WINDRES) -i win32/gobby.rc --input-format=rc -o win32/gobby.res -O coff
BUILT_SOURCES = win32/gobby.res
endif

gobby_SOURCES = src/icon.cpp
if WIN32
else
gobby_SOURCES += src/unix.cpp
endif
gobby_SOURCES += src/gselector.cpp
gobby_SOURCES += src/browser.cpp
gobby_SOURCES += src/ipc.cpp
gobby_SOURCES += src/colorutil.cpp
gobby_SOURCES += src/config.cpp
gobby_SOURCES += src/common.cpp
gobby_SOURCES += src/encoding.cpp
gobby_SOURCES += src/resolv.cpp
gobby_SOURCES += src/regex.cpp
gobby_SOURCES += src/application_state.cpp
gobby_SOURCES += src/encoding_selector.cpp
gobby_SOURCES += src/historyentry.cpp
gobby_SOURCES += src/fileentry.cpp
gobby_SOURCES += src/logview.cpp
gobby_SOURCES += src/colorsel.cpp
gobby_SOURCES += src/preferences.cpp
gobby_SOURCES += src/toolwindow.cpp
gobby_SOURCES += src/togglewindow.cpp
gobby_SOURCES += src/header.cpp
gobby_SOURCES += src/statusbar.cpp
gobby_SOURCES += src/document.cpp
gobby_SOURCES += src/buffer_def.cpp
gobby_SOURCES += src/docwindow.cpp
gobby_SOURCES += src/folder.cpp
gobby_SOURCES += src/document_settings.cpp
gobby_SOURCES += src/chat.cpp
gobby_SOURCES += src/preferencesdialog.cpp
gobby_SOURCES += src/initialdialog.cpp
gobby_SOURCES += src/passworddialog.cpp
gobby_SOURCES += src/hostdialog.cpp
gobby_SOURCES += src/joindialog.cpp
gobby_SOURCES += src/progressdialog.cpp
gobby_SOURCES += src/joinprogressdialog.cpp
gobby_SOURCES += src/hostprogressdialog.cpp
gobby_SOURCES += src/entrydialog.cpp
gobby_SOURCES += src/finddialog.cpp
gobby_SOURCES += src/gotodialog.cpp
gobby_SOURCES += src/dragdrop.cpp
gobby_SOURCES += src/window.cpp
gobby_SOURCES += src/main.cpp

appicondir        = $(datadir)/pixmaps
pixmapdir         = $(datadir)/pixmaps/gobby
gobby_CPPFLAGS   += -DAPPICON_DIR=\""$(appicondir)"\"
gobby_CPPFLAGS   += -DPIXMAPS_DIR=\""$(pixmapdir)"\"
dist_appicon_DATA = pixmaps/gobby.png
dist_pixmap_DATA  = pixmaps/userlist.png
dist_pixmap_DATA += pixmaps/doclist.png
dist_pixmap_DATA += pixmaps/chat.png

if WITH_GNOME
# GNOME-specific installation.
desktopdir = $(datadir)/applications
desktop_DATA = contrib/gobby.desktop
endif

EXTRA_DIST =  contrib/artwork/CREDITS
EXTRA_DIST += contrib/artwork/gobby.svg
EXTRA_DIST += contrib/artwork/gobby.xpm.gz
EXTRA_DIST += contrib/gobby.desktop
EXTRA_DIST += win32/gobby.ico
EXTRA_DIST += win32/gobby.rc

check_PROGRAMS = document
TESTS = document

document_SOURCES  = test/test_document.cpp
document_SOURCES += src/document.cpp
document_SOURCES += src/gselector.cpp
document_LDADD    = $(gobby_LIBS) $(gtksourceview_LIBS)
document_CPPFLAGS = $(gobby_CFLAGS) $(gtksourceview_CFLAGS)

