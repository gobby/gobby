# Gobby: First Contact

[![Build Status](https://travis-ci.org/gobby/gobby.svg?branch=master)](https://travis-ci.org/gobby/gobby)

## Synopsis

Gobby is a free collaborative editor. This means that it
provides you with the possibility to edit files simultaneously
with other users over a network. The platforms on which you could use
Gobby are so far Microsoft Windows, Linux, Mac OS X and other
Unix-like ones. Developed with the GTK+ toolkit it integrates
nicely into the GNOME desktop environment if you want it to.

## Features

- Realtime collaboration through encrypted connections
- Each user has its own colour to be identified by others
- Local group Undo
- IRC-like chat
- Shows cursors and selections of remote users
- Sidebar with all the others having joined the session
- Syntax highlighting, auto indentation, configurable tab
  width
- Multiple documents in one session
- Document subscriptions
- Zeroconf support (optional)
- Unicode support
- Internationalisation
- Cross-platform: Microsoft Windows, Linux, Mac OS X, other
  flavours of Unix
- Free software, licenced under the terms of the GNU General
  Public License

## Requirements

- libinfinity (0.6.x)
- Glib (>= 2.18.0)
- Glibmm (>= 2.18.0)
- libxml++ (>= 2.6.0)
- libgsasl (>= 0.2.21)

For a build with Gtk2:

- Gtkmm-2.4 (>= 2.12.0)
- GtkSourceView-2.0 (>= 2.4)

For a build with Gtk3:

- Gtkmm-3.0 (>= 2.99.2)
- GtkSourceView-3.0 (>= 2.91.0)

## More information

The development of Gobby is coordinated on github at <http://github.com/gobby>.
The primary distribution point is <http://releases.0x539.de/>.

### How to report bugs

Bugs should be files as issues on the github issue tracker at
<https://github.com/gobby/gobby/issues>. Please include a
reachable email address in your bug report as we often need to
contact the reporters for clarifications. Commonly bugs are
scheduled for the next major release and will be listed in the
roadmap.

### How to get involved

We urgently seek all kind of people who could help us in any
way. Firstly we need documentation writers who could help us
crafting a help file and some standalone documentation for the
web. Secondly, if you are skilled in C/C++ programming, we could
need helping hands with programming. You could use our project
page as a starting point to look for tickets which need
fixing. And last, but certainly not least, we need translators
who get their hands at Gobby's string templates to get it
localised. Thus more could use Gobby in their native language.
Just contact us, if in doubt, for more details.

There are also two mailing lists which should provide a mean
of contact to other Gobby users and to the developers.

- *obby-announce*: A moderated list used to announce new
  releases of Gobby and its foundation, net6, obby and libinfinity.
  Please refer to
  <http://list.0x539.de/mailman/listinfo/obby-announce> if you
  want to subscribe to it.
- *obby-users*: Discussions about Gobby's usage and
  installation problems. The announcements are also posted
  there. Please refer to
  <http://list.0x539.de/mailman/listinfo/obby-users> if you
  want to subscribe to it.

## Licensing

This program is written by the 0x539 dev group and is licenced
under the GNU General Public License (GPL) version 2 or any
later version. A copy of the license is included in the
distribution.

This program is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU General Public License for more details.

Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>

## Development

To get in contact with the developers, either use the obby-users mailing
list mentioned above or drop by in our IRC channel #infinote on
irc.freenode.org. To get the up-to-date Gobby code to hack on, use
`git clone https://github.com/gobby/gobby.git'. Pull requests against
our [GitHub repository](https://github.com/gobby/gobby) are appreciated.
