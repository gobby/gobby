noinst_LIBRARIES = libgobby-core.a

libgobby_core_a_SOURCES = \
	browser.cpp \
	documentinfostorage.cpp \
	docwindow.cpp \
	encodingselector.cpp \
	filechooser.cpp \
	folder.cpp \
	header.cpp \
	iconmanager.cpp \
	noteplugin.cpp \
	preferences.cpp \
	statusbar.cpp

libgobby_core_a_CPPFLAGS = $(gobby_CFLAGS) \
                          -I../../inc/ -DLOCALE_DIR=\""$(datadir)/locale"\" \
                          -DAPPICON_DIR=\""$(appicondir)"\" \
                          -DPIXMAPS_DIR=\""$(pixmapdir)"\"
