noinst_LIBRARIES = libgobby-commands.a

libgobby_commands_a_SOURCES = \
	browser-commands.cpp \
	file-commands.cpp

libgobby_commands_a_CPPFLAGS = $(gobby_CFLAGS) -I../../inc/ \
                              -DLOCALE_DIR=\""$(datadir)/locale"\" \
                              -DAPPICON_DIR=\""$(appicondir)"\" \
                              -DPIXMAPS_DIR=\""$(pixmapdir)"\"
