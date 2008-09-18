noinst_LIBRARIES = libgobby-core.a

libgobby_core_a_SOURCES = \
	browser.cpp \
	closableframe.cpp \
	documentinfostorage.cpp \
	docwindow.cpp \
	filechooser.cpp \
	folder.cpp \
	header.cpp \
	huebutton.cpp \
	iconmanager.cpp \
	nodewatch.cpp \
	noteplugin.cpp \
	preferences.cpp \
	statusbar.cpp \
	userlist.cpp

libgobby_core_a_CPPFLAGS = $(gobby_CFLAGS) -I../../inc/ \
                          -DAPPICON_DIR=\""$(appicondir)"\" \
                          -DPIXMAPS_DIR=\""$(pixmapdir)"\"
