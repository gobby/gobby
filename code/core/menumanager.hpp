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

#ifndef _GOBBY_MENUMANAGER_HPP_
#define _GOBBY_MENUMANAGER_HPP_

#include <giomm/menumodel.h>
#include <giomm/menu.h>

#include <gtksourceview/gtksource.h>

namespace Gobby
{

class MenuManager
{
public:
	MenuManager(GtkSourceLanguageManager* language_manager);

	Glib::RefPtr<Gio::MenuModel> get_app_menu() { return m_app_menu; }
	Glib::RefPtr<Gio::MenuModel> get_menu() { return m_menu; }
protected:
	Glib::RefPtr<Gio::Menu> m_app_menu;
	Glib::RefPtr<Gio::Menu> m_menu;

private:
	Glib::RefPtr<Gio::Menu> get_highlight_mode_menu();
};

}

#endif // _GOBBY_MENUMARAGER_HPP_
