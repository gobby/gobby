noinst_LIBRARIES = libgobby-operations.a

libgobby_operations_a_SOURCES = \
	operations.cpp \
	operation-delete.cpp \
	operation-new.cpp \
	operation-open.cpp \
	operation-save.cpp

libgobby_operations_a_CPPFLAGS = $(gobby_CFLAGS) -I../../inc/ \
                              -DLOCALE_DIR=\""$(datadir)/locale"\" \
                              -DAPPICON_DIR=\""$(appicondir)"\" \
                              -DPIXMAPS_DIR=\""$(pixmapdir)"\"
