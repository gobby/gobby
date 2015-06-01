SUBDIRS = util core dialogs operations commands

noinst_LIBRARIES = libgobby.a

libgobby_a_SOURCES = \
	application.cpp \
	main.cpp \
	window.cpp \
	gobby-resources.c

noinst_HEADERS = \
	application.hpp \
	window.hpp \
	gobby-resources.h

appicondir = $(datadir)/pixmaps
pixmapdir = $(datadir)/pixmaps/gobby-0.5

resources = \
	resources/ui/browser-context-menu.ui \
	resources/ui/connection-dialog.ui \
	resources/ui/entry-dialog.ui \
	resources/ui/goto-dialog.ui \
	resources/ui/menu.ui \
	resources/ui/toolbar.ui

gobby-resources.h: resources/gobby.gresources.xml
	glib-compile-resources $< \
		--target=$@ --sourcedir=$(srcdir)/resources \
		--c-name _gobby --generate-header

gobby-resources.c: resources/gobby.gresources.xml $(resources)
	glib-compile-resources $< \
		--target=$@ --sourcedir=$(srcdir)/resources \
		--c-name _gobby --generate-source

BUILT_SOURCES = gobby-resources.h gobby-resources.c
CLEANFILES = gobby-resources.h gobby-resources.c

# PRIVATE_ICONS_DIR is required by application.cpp to get access to the
# private icons that are installed into prefix/share/gobby-0.5/icons instead
# of prefix/share/icons. We also set PUBLIC_ICONS_DIR explicitely so that
# we find the application icon if it is installed in a non-standard prefix.
AM_CPPFLAGS = $(gobby_CFLAGS) $(infinote_CFLAGS) \
              -I$(top_srcdir)/code \
              -DPRIVATE_ICONS_DIR=\""$(datadir)/gobby-0.5/icons"\" \
              -DPUBLIC_ICONS_DIR=\""$(datadir)/icons"\"
