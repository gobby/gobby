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

#ifndef _GOBBY_WINDOWACTIONS_HPP_
#define _GOBBY_WINDOWACTIONS_HPP_

#include "core/preferences.hpp"

#include <giomm/actionmap.h>

namespace Gobby
{

class WindowActions
{
public:
	WindowActions(Gio::ActionMap& map, const Preferences& preferences);

	const Glib::RefPtr<Gio::SimpleAction> new_document;
	const Glib::RefPtr<Gio::SimpleAction> open;
	const Glib::RefPtr<Gio::SimpleAction> open_location;
	const Glib::RefPtr<Gio::SimpleAction> save;
	const Glib::RefPtr<Gio::SimpleAction> save_as;
	const Glib::RefPtr<Gio::SimpleAction> save_all;
	const Glib::RefPtr<Gio::SimpleAction> export_html;
	const Glib::RefPtr<Gio::SimpleAction> connect;
	const Glib::RefPtr<Gio::SimpleAction> close;

	const Glib::RefPtr<Gio::SimpleAction> undo;
	const Glib::RefPtr<Gio::SimpleAction> redo;
	const Glib::RefPtr<Gio::SimpleAction> cut;
	const Glib::RefPtr<Gio::SimpleAction> copy;
	const Glib::RefPtr<Gio::SimpleAction> paste;
	const Glib::RefPtr<Gio::SimpleAction> find;
	const Glib::RefPtr<Gio::SimpleAction> find_next;
	const Glib::RefPtr<Gio::SimpleAction> find_prev;
	const Glib::RefPtr<Gio::SimpleAction> find_replace;
	const Glib::RefPtr<Gio::SimpleAction> goto_line;

	const Glib::RefPtr<Gio::SimpleAction> hide_user_colors;
	const Glib::RefPtr<Gio::SimpleAction> fullscreen;
	const Glib::RefPtr<Gio::SimpleAction> zoom_in;
	const Glib::RefPtr<Gio::SimpleAction> zoom_out;
	const Glib::RefPtr<Gio::SimpleAction> view_toolbar;
	const Glib::RefPtr<Gio::SimpleAction> view_statusbar;
	const Glib::RefPtr<Gio::SimpleAction> view_browser;
	const Glib::RefPtr<Gio::SimpleAction> view_chat;
	const Glib::RefPtr<Gio::SimpleAction> view_document_userlist;
	const Glib::RefPtr<Gio::SimpleAction> view_chat_userlist;
	const Glib::RefPtr<Gio::SimpleAction> highlight_mode;
};

}

#endif // _GOBBY_WINDOWACTIONS_HPP_
