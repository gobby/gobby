SUBDIRS = . po

bin_PROGRAMS = gobby

gobby_LDADD    = $(gobby_LIBS)
gobby_CPPFLAGS = $(gobby_CFLAGS) -I../inc/ -DLOCALE_DIR=\""$(datadir)/locale"\"

if WIN32
gobby_LDFLAGS = -Wl,--enable-runtime-pseudo-reloc -mwindows win32/gobby.res
endif

if WITH_GTKSOURCEVIEW
gobby_CPPFLAGS += $(gtksourceview_CFLAGS)
gobby_LDADD    += $(gtksourceview_LIBS)
endif

noinst_HEADERS =  inc/config.hpp
noinst_HEADERS += inc/common.hpp
noinst_HEADERS += inc/buffer_wrapper.hpp
if WITH_GTKSOURCEVIEW
noinst_HEADERS += inc/mimemap.hpp
endif
noinst_HEADERS += inc/defaultdialog.hpp
noinst_HEADERS += inc/historyentry.hpp
noinst_HEADERS += inc/logview.hpp
noinst_HEADERS += inc/document.hpp
noinst_HEADERS += inc/docwindow.hpp
noinst_HEADERS += inc/statusbar.hpp
noinst_HEADERS += inc/header.hpp
noinst_HEADERS += inc/folder.hpp
noinst_HEADERS += inc/userlist.hpp
noinst_HEADERS += inc/chat.hpp
noinst_HEADERS += inc/preferences.hpp
noinst_HEADERS += inc/preferencesdialog.hpp
noinst_HEADERS += inc/hostdialog.hpp
noinst_HEADERS += inc/joindialog.hpp
noinst_HEADERS += inc/progressdialog.hpp
noinst_HEADERS += inc/joinprogressdialog.hpp
noinst_HEADERS += inc/hostprogressdialog.hpp
noinst_HEADERS += inc/entrydialog.hpp
noinst_HEADERS += inc/window.hpp 
if WITH_GTKSOURCEVIEW
noinst_HEADERS += inc/sourceview/sourceview.hpp
noinst_HEADERS += inc/sourceview/sourcebuffer.hpp
noinst_HEADERS += inc/sourceview/sourcelanguage.hpp
noinst_HEADERS += inc/sourceview/sourcelanguagesmanager.hpp
noinst_HEADERS += inc/sourceview/private/sourceview_p.hpp
noinst_HEADERS += inc/sourceview/private/sourcebuffer_p.hpp
noinst_HEADERS += inc/sourceview/private/sourcelanguage_p.hpp
noinst_HEADERS += inc/sourceview/private/sourcelanguagesmanager_p.hpp
endif
noinst_HEADERS += inc/icon.hpp

if WIN32
win32/gobby.res: win32/gobby.rc
	windres -i win32/gobby.rc --input-format=rc -o win32/gobby.res -O coff
BUILT_SOURCES = win32/gobby.res
endif

gobby_SOURCES =  src/config.cpp
gobby_SOURCES += src/buffer_wrapper.cpp
if WIN32
gobby_SOURCES += src/io_connection_win32.cpp
else
gobby_SOURCES += src/io_connection_generic.cpp
endif
if WITH_GTKSOURCEVIEW
gobby_SOURCES += src/mimemap.cpp
endif
gobby_SOURCES += src/defaultdialog.cpp
gobby_SOURCES += src/historyentry.cpp
gobby_SOURCES += src/logview.cpp
gobby_SOURCES += src/document.cpp
gobby_SOURCES += src/docwindow.cpp
gobby_SOURCES += src/statusbar.cpp
gobby_SOURCES += src/header.cpp
gobby_SOURCES += src/folder.cpp
gobby_SOURCES += src/userlist.cpp
gobby_SOURCES += src/chat.cpp
gobby_SOURCES += src/preferences.cpp
gobby_SOURCES += src/preferencesdialog.cpp
gobby_SOURCES += src/hostdialog.cpp
gobby_SOURCES += src/joindialog.cpp
gobby_SOURCES += src/progressdialog.cpp
gobby_SOURCES += src/joinprogressdialog.cpp
gobby_SOURCES += src/hostprogressdialog.cpp
gobby_SOURCES += src/entrydialog.cpp
gobby_SOURCES += src/window.cpp
gobby_SOURCES += src/main.cpp
if WITH_GTKSOURCEVIEW
gobby_SOURCES += src/sourceview/sourceview.cpp
gobby_SOURCES += src/sourceview/sourcebuffer.cpp
gobby_SOURCES += src/sourceview/sourcelanguage.cpp
gobby_SOURCES += src/sourceview/sourcelanguagesmanager.cpp
endif
if ! WITH_GNOME
gobby_SOURCES += src/icon.cpp
endif

if WITH_GNOME
# GNOME-specific installation.
gobby_CPPFLAGS += -DPIXMAPS_DIR=\""$(datadir)/pixmaps"\"
appicondir = $(datadir)/pixmaps
appicon_DATA = contrib/artwork/gobby.png
desktopdir = $(datadir)/applications
desktop_DATA = contrib/gobby.desktop
endif

EXTRA_DIST =  contrib/artwork/CREDITS
EXTRA_DIST += contrib/artwork/gobby.png
EXTRA_DIST += contrib/artwork/gobby.svg
EXTRA_DIST += contrib/artwork/gobby.xpm.gz
EXTRA_DIST += contrib/gobby.desktop
EXTRA_DIST += win32/gobby.ico
EXTRA_DIST += win32/gobby.rc

