/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _GOBBY_APPLICATIONACTIONS_HPP_
#define _GOBBY_APPLICATIONACTIONS_HPP_

#include <giomm/actionmap.h>

namespace Gobby
{

class ApplicationActions
{
public:
	ApplicationActions(Gio::ActionMap& map);

	const Glib::RefPtr<Gio::SimpleAction> quit;
	const Glib::RefPtr<Gio::SimpleAction> preferences;
	const Glib::RefPtr<Gio::SimpleAction> help;
	const Glib::RefPtr<Gio::SimpleAction> about;

};

}

#endif // _GOBBY_APPLICATIONACTIONS_HPP_
