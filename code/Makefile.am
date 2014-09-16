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

# PRIVATE_ICONS_DIR is required by application.cpp to get access to the
# private icons that are installed into prefix/share/gobby-0.5/icons instead
# of prefix/share/icons. We also set PUBLIC_ICONS_DIR explicitely so that
# we find the application icon if it is installed in a non-standard prefix.
AM_CPPFLAGS = $(gobby_CFLAGS) $(infinote_CFLAGS) \
              -I$(top_srcdir)/code \
              -DPRIVATE_ICONS_DIR=\""$(datadir)/gobby-0.5/icons"\" \
              -DPUBLIC_ICONS_DIR=\""$(datadir)/icons"\"
