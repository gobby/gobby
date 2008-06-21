noinst_LIBRARIES = libgobby-util.a

libgobby_util_a_SOURCES = \
	color.cpp \
	config.cpp \
	encoding.cpp \
	file.cpp \
	i18n.cpp \
	resolv.cpp \
	serialize.cpp

libgobby_util_a_CPPFLAGS = $(gobby_CFLAGS) \
                          -I../../inc/ -DLOCALE_DIR=\""$(datadir)/locale"\" \
                          -DAPPICON_DIR=\""$(appicondir)"\" \
                          -DPIXMAPS_DIR=\""$(pixmapdir)"\"
