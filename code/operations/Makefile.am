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

libgobby_operations_a_CPPFLAGS = $(gobby_CFLAGS) -I.. \
                              -DAPPICON_DIR=\""$(appicondir)"\" \
                              -DPIXMAPS_DIR=\""$(pixmapdir)"\"
