noinst_LIBRARIES = libgobby-core.a

libgobby_core_a_SOURCES = \
	browser.cpp \
	certificatemanager.cpp \
	chatsessionview.cpp \
	chattablabel.cpp \
	closableframe.cpp \
	credentialsgenerator.cpp \
	documentinfostorage.cpp \
	filechooser.cpp \
	folder.cpp \
	header.cpp \
	huebutton.cpp \
	iconmanager.cpp \
	nodewatch.cpp \
	noteplugin.cpp \
	preferences.cpp \
	sessionuserview.cpp \
	sessionview.cpp \
	statusbar.cpp \
	tablabel.cpp \
	textsessionuserview.cpp \
	textsessionview.cpp \
	textundogrouping.cpp \
	texttablabel.cpp \
	titlebar.cpp \
	userlist.cpp

noinst_HEADERS = \
        browser.hpp \
	certificatemanager.hpp \
	chatsessionview.hpp \
	chattablabel.hpp \
        closableframe.hpp \
	credentialsgenerator.hpp \
        documentinfostorage.hpp \
        filechooser.hpp \
        folder.hpp \
        header.hpp \
        huebutton.hpp \
        iconmanager.hpp \
        nodewatch.hpp \
        noteplugin.hpp \
        preferences.hpp \
	sessionuserview.hpp \
	sessionview.hpp \
        statusbar.hpp \
	tablabel.hpp \
        textsessionuserview.hpp \
        textsessionview.hpp \
	textundogrouping.hpp \
	texttablabel.hpp \
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
