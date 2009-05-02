noinst_LIBRARIES = libgobby-util.a

libgobby_util_a_SOURCES = \
	closebutton.cpp \
	color.cpp \
	config.cpp \
	file.cpp \
	historyentry.cpp \
	i18n.cpp \
	resolv.cpp \
	serialize.cpp

noinst_HEADERS = \
        closebutton.hpp \
        color.hpp \
        config.hpp \
        defaultaccumulator.hpp \
        file.hpp \
	historyentry.hpp \
        i18n.hpp \
        resolv.hpp \
        serialize.hpp

AM_CPPFLAGS = $(gobby_CFLAGS) $(unique_CFLAGS) $(infinote_CFLAGS) \
              -I$(top_srcdir)/code
