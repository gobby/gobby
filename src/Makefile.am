SUBDIRS = commands

noinst_LIBRARIES = libgobby.a

libgobby_a_SOURCES = \
	browser.cpp \
	colorutil.cpp \
	config.cpp \
	docwindow.cpp \
	dragdrop.cpp \
	encoding.cpp \
	encoding_selector.cpp \
	finddialog.cpp \
	folder.cpp \
	gotodialog.cpp \
	header.cpp \
	i18n.cpp \
	icon.cpp \
	initialdialog.cpp \
	main.cpp \
	preferences.cpp \
	preferencesdialog.cpp \
	resolv.cpp \
	serialize.cpp \
	statusbar.cpp \
	toolwindow.cpp \
	window.cpp

appicondir = $(datadir)/pixmaps
pixmapdir = $(datadir)/pixmaps/gobby

libgobby_a_CPPFLAGS = $(gobby_CFLAGS) \
                      -I../inc/ -DLOCALE_DIR=\""$(datadir)/locale"\" \
                      -DAPPICON_DIR=\""$(appicondir)"\" \
                      -DPIXMAPS_DIR=\""$(pixmapdir)"\"

# We need to link all libraries directly into the executable in the toplevel
# Makefile.am:
#libgobby_a_LIBADD = commands/libgobby-commands.a
