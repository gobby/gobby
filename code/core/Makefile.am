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
	tablabel.cpp \
	userlist.cpp

noinst_HEADERS = \
        browser.hpp \
        closableframe.hpp \
        documentinfostorage.hpp \
        docwindow.hpp \
        filechooser.hpp \
        folder.hpp \
        header.hpp \
        huebutton.hpp \
        iconmanager.hpp \
        nodewatch.hpp \
        noteplugin.hpp \
        preferences.hpp \
        statusbar.hpp \
	tablabel.hpp \
        userlist.hpp

libgobby_core_a_CPPFLAGS = $(gobby_CFLAGS) -I.. \
                          -DAPPICON_DIR=\""$(appicondir)"\" \
                          -DPIXMAPS_DIR=\""$(pixmapdir)"\"
