noinst_LIBRARIES = libgobby-dialogs.a

libgobby_dialogs_a_SOURCES = \
	documentlocationdialog.cpp \
	finddialog.cpp \
	gotodialog.cpp \
	initialdialog.cpp \
	preferencesdialog.cpp

libgobby_dialogs_a_CPPFLAGS = $(gobby_CFLAGS) -I../../inc/ \
                              -DLOCALE_DIR=\""$(datadir)/locale"\" \
                              -DAPPICON_DIR=\""$(appicondir)"\" \
                              -DPIXMAPS_DIR=\""$(pixmapdir)"\"
