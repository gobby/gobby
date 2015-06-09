noinst_LIBRARIES = libgobby-util.a

libgobby_util_a_SOURCES = \
	asyncoperation.cpp \
	closebutton.cpp \
	color.cpp \
	config.cpp \
	file.cpp \
	historyentry.cpp \
	i18n.cpp \
	serialize.cpp \
	uri.cpp

noinst_HEADERS = \
	asyncoperation.hpp \
	closebutton.hpp \
	color.hpp \
	config.hpp \
	defaultaccumulator.hpp \
	file.hpp \
	historyentry.hpp \
	i18n.hpp \
	serialize.hpp \
	uri.hpp

AM_CPPFLAGS = $(gobby_CFLAGS) $(infinote_CFLAGS) \
              -I$(top_srcdir)/code
