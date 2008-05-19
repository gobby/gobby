SUBDIRS = util core dialogs operations commands

noinst_LIBRARIES = libgobby.a

libgobby_a_SOURCES = \
	dragdrop.cpp \
	main.cpp \
	window.cpp

appicondir = $(datadir)/pixmaps
pixmapdir = $(datadir)/pixmaps/gobby

libgobby_a_CPPFLAGS = $(gobby_CFLAGS) \
                      -I../inc/ -DLOCALE_DIR=\""$(datadir)/locale"\" \
                      -DAPPICON_DIR=\""$(appicondir)"\" \
                      -DPIXMAPS_DIR=\""$(pixmapdir)"\"
