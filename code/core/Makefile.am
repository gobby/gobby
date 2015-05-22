noinst_LIBRARIES = libgobby-core.a

libgobby_core_a_SOURCES = \
	applicationactions.cpp \
	browser.cpp \
	certificatemanager.cpp \
	chatsessionview.cpp \
	chattablabel.cpp \
	closableframe.cpp \
	connectionmanager.cpp \
	credentialsgenerator.cpp \
	documentinfostorage.cpp \
	filechooser.cpp \
	folder.cpp \
	foldermanager.cpp \
	huebutton.cpp \
	knownhoststorage.cpp \
	menumanager.cpp \
	nodewatch.cpp \
	noteplugin.cpp \
	preferences.cpp \
	selfhoster.cpp \
	server.cpp \
	sessionuserview.cpp \
	sessionview.cpp \
	statusbar.cpp \
	tablabel.cpp \
	textsessionuserview.cpp \
	textsessionview.cpp \
	textundogrouping.cpp \
	texttablabel.cpp \
	titlebar.cpp \
	toolbar.cpp \
	userjoin.cpp \
	userlist.cpp \
	windowactions.cpp

noinst_HEADERS = \
	applicationactions.hpp \
	browser.hpp \
	certificatemanager.hpp \
	chatsessionview.hpp \
	chattablabel.hpp \
	closableframe.hpp \
	connectionmanager.hpp \
	credentialsgenerator.hpp \
	documentinfostorage.hpp \
	filechooser.hpp \
	folder.hpp \
	foldermanager.hpp \
	menumanager.hpp \
	huebutton.hpp \
	knownhoststorage.hpp \
	nodewatch.hpp \
	noteplugin.hpp \
	preferences.hpp \
	selfhoster.hpp \
	server.hpp \
	sessionuserview.hpp \
	sessionview.hpp \
	statusbar.hpp \
	tablabel.hpp \
	textsessionuserview.hpp \
	textsessionview.hpp \
	textundogrouping.hpp \
	texttablabel.hpp \
	titlebar.hpp \
	toolbar.hpp \
	userjoin.hpp \
	userlist.hpp \
	windowactions.hpp

appicondir = $(datadir)/pixmaps
pixmapdir = $(datadir)/pixmaps/gobby-0.5

AM_CPPFLAGS = $(gobby_CFLAGS) $(infinote_CFLAGS) \
              -I$(top_srcdir)/code

SUBDIRS = gobject
