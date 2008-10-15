SUBDIRS = m4 contrib pixmaps win32 code . po

# Tell aclocal where to find `.m4' files.
ACLOCAL_AMFLAGS = -I m4

dist_man_MANS = gobby.1

bin_PROGRAMS = gobby-0.5

gobby_0_5_LDADD = \
	code/libgobby.a \
	code/commands/libgobby-commands.a \
	code/operations/libgobby-operations.a \
	code/dialogs/libgobby-dialogs.a \
	code/core/libgobby-core.a \
	code/util/libgobby-util.a \
	$(gobby_LIBS)

if WIN32
# For some reason it does not link against libstdc++ on Windows. Perhaps
# that's because we don't specify a source file here, so it guesses C.
gobby_0_5_LDADD += -lws2_32 -lstdc++
gobby_0_5_LDFLAGS = \
	-Wl,--enable-runtime-pseudo-reloc \
	-mwindows \
	win32/gobby.res
endif

gobby_0_5_SOURCES = 
