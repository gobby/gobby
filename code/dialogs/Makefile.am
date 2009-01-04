noinst_LIBRARIES = libgobby-dialogs.a

libgobby_dialogs_a_SOURCES = \
	documentlocationdialog.cpp \
	entrydialog.cpp \
	finddialog.cpp \
	gotodialog.cpp \
	initialdialog.cpp \
	openlocationdialog.cpp \
	preferencesdialog.cpp

noinst_HEADERS = \
        documentlocationdialog.hpp \
        entrydialog.hpp \
        finddialog.hpp \
        gotodialog.hpp \
        initialdialog.hpp \
	openlocationdialog.hpp \
        preferencesdialog.hpp

appicondir = $(datadir)/pixmaps
pixmapdir = $(datadir)/pixmaps/gobby-0.5

AM_CPPFLAGS = $(gobby_CFLAGS) $(infinote_CFLAGS) \
              -I$(top_srcdir)/code \
              -DAPPICON_DIR=\""$(appicondir)"\" \
              -DPIXMAPS_DIR=\""$(pixmapdir)"\"

