include code/util/Makefile.am
include code/core/Makefile.am
include code/operations/Makefile.am
include code/dialogs/Makefile.am
include code/commands/Makefile.am

gobby_0_5_SOURCES += \
	code/application.cpp \
	code/main.cpp \
	code/window.cpp \
	code/gobby-resources.c

noinst_HEADERS += \
	code/application.hpp \
	code/window.hpp \
	code/gobby-resources.h

appicondir = $(datadir)/pixmaps
pixmapdir = $(datadir)/pixmaps/gobby-0.5

resources = \
	code/resources/ui/browser-context-menu.ui \
	code/resources/ui/connection-dialog.ui \
	code/resources/ui/connection-info-dialog.ui \
	code/resources/ui/document-location-dialog.ui \
	code/resources/ui/entry-dialog.ui \
	code/resources/ui/find-dialog.ui \
	code/resources/ui/goto-dialog.ui \
	code/resources/ui/initial-dialog.ui \
	code/resources/ui/menu.ui \
	code/resources/ui/open-location-dialog.ui \
	code/resources/ui/password-dialog.ui \
	code/resources/ui/preferences-dialog.ui \
	code/resources/ui/toolbar.ui

code/gobby-resources.h: code/resources/gobby.gresources.xml
	glib-compile-resources $< \
		--target=$@ --sourcedir=$(top_srcdir)/code/resources \
		--c-name _gobby --generate-header

code/gobby-resources.c: code/resources/gobby.gresources.xml $(resources)
	glib-compile-resources $< \
		--target=$@ --sourcedir=$(top_srcdir)/code/resources \
		--c-name _gobby --generate-source

BUILT_SOURCES += code/gobby-resources.h code/gobby-resources.c
CLEANFILES += code/gobby-resources.h code/gobby-resources.c
