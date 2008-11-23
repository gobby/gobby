SUBDIRS = artwork

if WITH_GNOME
# GNOME-specific installation.
desktopdir = $(datadir)/applications
desktop_in_files = gobby-0.5.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@
endif

EXTRA_DIST = $(desktop_in_files)
