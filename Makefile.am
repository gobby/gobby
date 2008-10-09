SUBDIRS = m4 contrib pixmaps win32 inc src . po

# Tell aclocal where to find `.m4' files.
ACLOCAL_AMFLAGS = -I m4

dist_man_MANS = gobby.1

bin_PROGRAMS = gobby-0.5

gobby_0_5_LDADD = $(gobby_LIBS) \
	src/libgobby.a \
	src/commands/libgobby-commands.a \
	src/operations/libgobby-operations.a \
	src/dialogs/libgobby-dialogs.a \
	src/core/libgobby-core.a \
	src/util/libgobby-util.a

if WIN32
gobby_0_5_LDADD += -lole32 -luuid
gobby_0_5_LDFLAGS = \
	-Wl,--enable-runtime-pseudo-reloc \
	-mwindows \
	win32/gobby.res
endif

gobby_0_5_SOURCES = 
