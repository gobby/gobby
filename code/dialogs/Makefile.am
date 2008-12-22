noinst_LIBRARIES = libgobby-dialogs.a

libgobby_dialogs_a_SOURCES = \
	documentlocationdialog.cpp \
	entrydialog.cpp \
	finddialog.cpp \
	gotodialog.cpp \
	initialdialog.cpp \
	preferencesdialog.cpp

noinst_HEADERS = \
        documentlocationdialog.hpp \
        entrydialog.hpp \
        finddialog.hpp \
        gotodialog.hpp \
        initialdialog.hpp \
        preferencesdialog.hpp

appicondir = $(datadir)/pixmaps
pixmapdir = $(datadir)/pixmaps/gobby-0.5

libgobby_dialogs_a_CPPFLAGS = $(CFLAGS) $(gobby_CFLAGS) -I$(top_srcdir)/code \
                              -DAPPICON_DIR=\""$(appicondir)"\" \
                              -DPIXMAPS_DIR=\""$(pixmapdir)"\"
