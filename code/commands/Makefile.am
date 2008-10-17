noinst_LIBRARIES = libgobby-commands.a

libgobby_commands_a_SOURCES = \
	browser-commands.cpp \
	browser-context-commands.cpp \
	edit-commands.cpp \
	file-commands.cpp \
	folder-commands.cpp \
	help-commands.cpp \
	view-commands.cpp

noinst_HEADERS = \
        browser-commands.hpp \
        browser-context-commands.hpp \
        edit-commands.hpp \
        file-commands.hpp \
        folder-commands.hpp \
        help-commands.hpp \
        view-commands.hpp

appicondir = $(datadir)/pixmaps
pixmapdir = $(datadir)/pixmaps/gobby-0.5

libgobby_commands_a_CPPFLAGS = $(gobby_CFLAGS) -I$(top_srcdir)/code \
                              -DAPPICON_DIR=\""$(appicondir)"\" \
                              -DPIXMAPS_DIR=\""$(pixmapdir)"\"
