noinst_LIBRARIES = libgobby-operations.a

libgobby_operations_a_SOURCES = \
	operations.cpp \
	operation-delete.cpp \
	operation-rename.cpp \
	operation-export-html.cpp \
	operation-new.cpp \
	operation-open.cpp \
	operation-open-multiple.cpp \
	operation-save.cpp \
	operation-subscribe-path.cpp

noinst_HEADERS = \
        operations.hpp \
        operation-delete.hpp \
        operation-rename.hpp \
	operation-export-html.hpp \
        operation-new.hpp \
        operation-open.hpp \
	operation-open-multiple.hpp \
        operation-save.hpp \
	operation-subscribe-path.hpp

appicondir = $(datadir)/pixmaps
pixmapdir = $(datadir)/pixmaps/gobby-0.5

AM_CPPFLAGS = $(gobby_CFLAGS) $(unique_CFLAGS) $(infinote_CFLAGS) \
              -I$(top_srcdir)/code
