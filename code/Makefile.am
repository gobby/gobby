SUBDIRS = util core dialogs operations commands

noinst_LIBRARIES = libgobby.a

libgobby_a_SOURCES = \
	main.cpp \
	window.cpp

noinst_HEADERS = window.hpp

appicondir = $(datadir)/pixmaps
pixmapdir = $(datadir)/pixmaps/gobby

libgobby_a_CPPFLAGS = $(gobby_CFLAGS) -I. \
                      -DAPPICON_DIR=\""$(appicondir)"\" \
                      -DPIXMAPS_DIR=\""$(pixmapdir)"\"
