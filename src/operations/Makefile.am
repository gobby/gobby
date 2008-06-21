noinst_LIBRARIES = libgobby-operations.a

libgobby_operations_a_SOURCES = \
	documentinfostorage.cpp \
	operations.cpp \
	operation-new.cpp \
	operation-open.cpp

libgobby_operations_a_CPPFLAGS = $(gobby_CFLAGS) -I../../inc/ \
                              -DLOCALE_DIR=\""$(datadir)/locale"\" \
                              -DAPPICON_DIR=\""$(appicondir)"\" \
                              -DPIXMAPS_DIR=\""$(pixmapdir)"\"
