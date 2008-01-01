#!/bin/sh

autoreconf -f -i && automake -a -f && glib-gettextize -f

