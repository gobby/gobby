if WIN32
win32/gobby.res: win32/gobby.rc
	$(WINDRES) -i win32/gobby.rc --input-format=rc -o win32/gobby.res -O coff

BUILT_SOURCES += win32/gobby.res
endif

EXTRA_DIST += win32/gobby.ico win32/gobby.rc
