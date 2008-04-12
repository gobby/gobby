SUBDIRS = m4 contrib pixmaps win32 inc src . po

# Tell aclocal where to find `.m4' files.
ACLOCAL_AMFLAGS = -I m4

dist_man_MANS = gobby.1

bin_PROGRAMS = gobby

if WIN32
gobby_LDADD = $(gobby_LIBS) src/libgobby.a -lole32 -luuid
gobby_LDFLAGS = -Wl,--enable-runtime-pseudo-reloc -mwindows win32/gobby.res
else
gobby_LDADD = $(gobby_LIBS) src/libgobby.a
endif

gobby_SOURCES = 
