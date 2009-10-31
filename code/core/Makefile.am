noinst_LIBRARIES = libgobby-core.a

libgobby_core_a_SOURCES = \
	browser.cpp \
	closableframe.cpp \
	documentinfostorage.cpp \
	docwindow.cpp \
	filechooser.cpp \
	folder.cpp \
	header.cpp \
	huebutton.cpp \
	iconmanager.cpp \
	nodewatch.cpp \
	noteplugin.cpp \
	preferences.cpp \
	statusbar.cpp \
	tablabel.cpp \
	titlebar.cpp \
	userlist.cpp

noinst_HEADERS = \
        browser.hpp \
        closableframe.hpp \
        documentinfostorage.hpp \
        docwindow.hpp \
        filechooser.hpp \
        folder.hpp \
        header.hpp \
        huebutton.hpp \
        iconmanager.hpp \
        nodewatch.hpp \
        noteplugin.hpp \
        preferences.hpp \
        statusbar.hpp \
	tablabel.hpp \
	titlebar.hpp \
        userlist.hpp

appicondir = $(datadir)/pixmaps
pixmapdir = $(datadir)/pixmaps/gobby-0.5

# PRIVATE_ICONS_DIR is required by iconmanager.cpp to get access to the
# private icons that are installed into prefix/share/gobby-0.5/icons instead
# of prefix/share/icons. We also set PUBLIC_ICONS_DIR explicitely so that
# we find the application icon if it is installed in a non-standard prefix.
AM_CPPFLAGS = $(gobby_CFLAGS) $(unique_CFLAGS) $(infinote_CFLAGS) \
              -I$(top_srcdir)/code \
	      -DPRIVATE_ICONS_DIR=\""$(datadir)/gobby-0.5/icons"\" \
	      -DPUBLIC_ICONS_DIR=\""$(datadir)/icons"\"
