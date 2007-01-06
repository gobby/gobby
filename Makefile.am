bin_PROGRAMS = gobby

LDADD = $(gobby_LIBS)
INCLUDES = $(gobby_CFLAGS) -I../inc/

if WIN32
gobby_LDFLAGS = -Wl,--enable-runtime-pseudo-reloc
endif

noinst_HEADERS =  inc/config.hpp
noinst_HEADERS += inc/buffer_wrapper.hpp
if WIN32
noinst_HEADERS += inc/buffer_wrapper_win32.hpp
else
noinst_HEADERS += inc/buffer_wrapper_generic.hpp
endif
noinst_HEADERS += inc/historyentry.hpp
noinst_HEADERS += inc/logview.hpp
noinst_HEADERS += inc/document.hpp
noinst_HEADERS += inc/header.hpp
noinst_HEADERS += inc/folder.hpp
noinst_HEADERS += inc/userlist.hpp
noinst_HEADERS += inc/chat.hpp
noinst_HEADERS += inc/hostdialog.hpp
noinst_HEADERS += inc/joindialog.hpp
noinst_HEADERS += inc/entrydialog.hpp
noinst_HEADERS += inc/window.hpp 

gobby_SOURCES =  src/config.cpp
gobby_SOURCES += src/buffer_wrapper.cpp
if WIN32
gobby_SOURCES += src/buffer_wrapper_win32.cpp
else
gobby_SOURCES += src/buffer_wrapper_generic.cpp
endif
gobby_SOURCES += src/historyentry.cpp
gobby_SOURCES += src/logview.cpp
gobby_SOURCES += src/document.cpp
gobby_SOURCES += src/header.cpp
gobby_SOURCES += src/folder.cpp
gobby_SOURCES += src/userlist.cpp
gobby_SOURCES += src/chat.cpp
gobby_SOURCES += src/hostdialog.cpp
gobby_SOURCES += src/joindialog.cpp
gobby_SOURCES += src/entrydialog.cpp
gobby_SOURCES += src/window.cpp
gobby_SOURCES += src/main.cpp

