#!/bin/sh -e

glib-gettextize --copy --force
intltoolize --copy --force --automake
gnome-doc-prepare

autoreconf -f -i && automake -a -f

