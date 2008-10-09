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

libgobby_dialogs_a_CPPFLAGS = $(gobby_CFLAGS) -I.. \
                              -DAPPICON_DIR=\""$(appicondir)"\" \
                              -DPIXMAPS_DIR=\""$(pixmapdir)"\"
