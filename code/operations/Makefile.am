noinst_LIBRARIES = libgobby-operations.a

libgobby_operations_a_SOURCES = \
	operations.cpp \
	operation-delete.cpp \
	operation-new.cpp \
	operation-open.cpp \
	operation-save.cpp

noinst_HEADERS = \
        operations.hpp \
        operation-delete.hpp \
        operation-new.hpp \
        operation-open.hpp \
        operation-save.hpp

appicondir = $(datadir)/pixmaps
pixmapdir = $(datadir)/pixmaps/gobby-0.5

libgobby_operations_a_CPPFLAGS = $(CFLAGS) $(gobby_CFLAGS) -I$(top_srcdir)/code \
                              -DAPPICON_DIR=\""$(appicondir)"\" \
                              -DPIXMAPS_DIR=\""$(pixmapdir)"\"
