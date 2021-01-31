# Gobby: First Contact

![Linux build](https://github.com/gobby/gobby/workflows/Linux%20build/badge.svg)
![Windows build](https://github.com/gobby/gobby/workflows/Windows%20build/badge.svg)

## Synopsis

Gobby is a free collaborative editor. This means that it
provides you with the possibility to edit files simultaneously
with other users over a network. The platforms on which you could use
Gobby are so far Microsoft Windows, Linux, Mac OS X and other
Unix-like ones. Developed with the GTK+ toolkit it integrates
nicely into the GNOME desktop environment if you want it to.

## Features

- Realtime collaboration through encrypted connections (including PFS)
- Each user has its own colour to be identified by others
- Local group Undo
- Group chat
- Shows cursors and selections of remote users
- Sidebar with all the others having joined the session
- Syntax highlighting, auto indentation, configurable tab
  width
- Multiple documents in one session
- Zeroconf support (optional)
- Unicode support
- Internationalisation
- Highly configurable dedicated server
- Sophisticated Access Control Lists (ACLs)
- Cross-platform: Microsoft Windows, Linux, Mac OS X, other
  flavours of Unix
- Free software, licenced under the terms of the ISC license

## Requirements

- libinfinity (0.7.x)
- Glib (>= 2.40.0)
- Glibmm (>= 2.40.0)
- libxml++ (>= 2.6.0)
- libgsasl (>= 0.2.21)
- Gtkmm-3.0 (>= 3.6)
- GtkSourceView-3.0 (>= 3.0)

## More information

The development of Gobby is coordinated on github at <https://github.com/gobby>.
The primary distribution point is <https://github.com/gobby/gobby/releases>.

### How to report bugs

Bugs should be files as issues on the github issue tracker at
<https://github.com/gobby/gobby/issues>. 

### How to get involved

We urgently seek all kind of people who could help us in any
way. Firstly we need documentation writers who could help us
crafting a help file and some standalone documentation for the
web. Secondly, if you are skilled in C/C++ programming, we could
need helping hands with programming. A starting point could be
[github issues](https://github.com/gobby/gobby.git).
And last, but certainly not least, we need translators
who get their hands at Gobby's string templates to get it
localised. Thus more could use Gobby in their native language.
Just contact us, if in doubt, for more details.

There are also two mailing lists which should provide a mean
of contact to other Gobby users and to the developers.

- *obby-announce*: A moderated list used to announce new
  releases of Gobby and its foundation, libinfinity. To subscribe,
  please send an email with the topic "subscribe" to
  <obby-announce-subscribe@list.0x539.de>. A list archive is
  available at <https://www.mail-archive.com/obby-announce@list.0x539.de>.
- *obby-users*: Discussions about Gobby's usage and
  installation problems. The announcements are also posted
  there. To subscribe, please send an email with the topic
  "subscribe" to <obby-users-subscribe@list.0x539.de>. A list
  archive is available at <https://www.mail-archive.com/obby-users@list.0x539.de>.

## Licensing

Copyright (C) 2008-2015, Armin Burgmeier <armin@arbur.net>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

## Development

To get in contact with the developers, please use the obby-users mailing
list mentioned above, or open an issue on github. To get the up-to-date
Gobby code to hack on, use `git clone https://github.com/gobby/gobby.git`.
Pull requests against our [GitHub repository](https://github.com/gobby/gobby)
are appreciated.
