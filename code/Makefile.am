SUBDIRS = util core dialogs operations commands

noinst_LIBRARIES = libgobby.a

libgobby_a_SOURCES = \
	application.cpp \
	main.cpp \
	window.cpp

noinst_HEADERS = \
	application.hpp \
	window.hpp

appicondir = $(datadir)/pixmaps
pixmapdir = $(datadir)/pixmaps/gobby-0.5

AM_CPPFLAGS = $(gobby_CFLAGS) $(unique_CFLAGS) $(infinote_CFLAGS) \
              -I$(top_srcdir)/code

