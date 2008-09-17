noinst_LIBRARIES = libgobby-commands.a

libgobby_commands_a_SOURCES = \
	browser-commands.cpp \
	browser-context-commands.cpp \
	edit-commands.cpp \
	file-commands.cpp \
	folder-commands.cpp \
	help-commands.cpp \
	view-commands.cpp

libgobby_commands_a_CPPFLAGS = $(gobby_CFLAGS) -I../../inc/ \
                              -DAPPICON_DIR=\""$(appicondir)"\" \
                              -DPIXMAPS_DIR=\""$(pixmapdir)"\"
