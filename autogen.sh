#!/bin/sh -e

glib-gettextize --copy --force
intltoolize --copy --force --automake

autoreconf -f -i && automake -a -f

