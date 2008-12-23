SUBDIRS = m4 contrib pixmaps icons win32 code . po

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
	$(gobby_LIBS) \
	$(infinote_LIBS) \
	$(LIBS)

if WIN32
gobby_0_5_LDADD += -lws2_32
gobby_0_5_LDFLAGS = \
	-Wl,--enable-runtime-pseudo-reloc \
	-mwindows \
	win32/gobby.res
endif

gobby_0_5_SOURCES = 

# Dummy C++ source to cause C++ linking, otherwise some platforms don't link
# correctly against libstdc++. See also #399. Note that this file does not
# (need to) exist in the source tree.
nodist_EXTRA_gobby_0_5_SOURCES = dummy.cpp
