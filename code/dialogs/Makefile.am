noinst_LIBRARIES = libgobby-dialogs.a

libgobby_dialogs_a_SOURCES = \
	connection-dialog.cpp \
	connection-info-dialog.cpp \
	document-location-dialog.cpp \
	entry-dialog.cpp \
	find-dialog.cpp \
	goto-dialog.cpp \
	initial-dialog.cpp \
	open-location-dialog.cpp \
	password-dialog.cpp \
	preferences-dialog.cpp

noinst_HEADERS = \
	connection-dialog.hpp \
	connection-info-dialog.hpp \
        document-location-dialog.hpp \
        entry-dialog.hpp \
        find-dialog.hpp \
        goto-dialog.hpp \
        initial-dialog.hpp \
	open-location-dialog.hpp \
	password-dialog.hpp \
        preferences-dialog.hpp

appicondir = $(datadir)/pixmaps
pixmapdir = $(datadir)/pixmaps/gobby-0.5

AM_CPPFLAGS = $(gobby_CFLAGS) $(infinote_CFLAGS) \
              -I$(top_srcdir)/code
