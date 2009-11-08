SUBDIRS = file-tasks

noinst_LIBRARIES = libgobby-commands.a

libgobby_commands_a_SOURCES = \
	autosave-commands.cpp \
	browser-commands.cpp \
	browser-context-commands.cpp \
	edit-commands.cpp \
	file-commands.cpp \
	folder-commands.cpp \
	help-commands.cpp \
	subscription-commands.cpp \
	synchronization-commands.cpp \
	user-join-commands.cpp \
	view-commands.cpp

noinst_HEADERS = \
	autosave-commands.hpp \
        browser-commands.hpp \
        browser-context-commands.hpp \
        edit-commands.hpp \
        file-commands.hpp \
        folder-commands.hpp \
        help-commands.hpp \
	subscription-commands.hpp \
	synchronization-commands.hpp \
	user-join-commands.hpp \
        view-commands.hpp

appicondir = $(datadir)/pixmaps
pixmapdir = $(datadir)/pixmaps/gobby-0.5

AM_CPPFLAGS = $(gobby_CFLAGS) $(unique_CFLAGS) $(infinote_CFLAGS) \
              -I$(top_srcdir)/code
