SUBDIRS = util core dialogs operations commands

noinst_LIBRARIES = libgobby.a

libgobby_a_SOURCES = \
	main.cpp \
	window.cpp

noinst_HEADERS = window.hpp

appicondir = $(datadir)/pixmaps
pixmapdir = $(datadir)/pixmaps/gobby-0.5

AM_CPPFLAGS = $(gobby_CFLAGS) $(infinote_CFLAGS) \
              -I$(top_srcdir)/code \
              -DAPPICON_DIR=\""$(appicondir)"\" \
              -DPIXMAPS_DIR=\""$(pixmapdir)"\"

