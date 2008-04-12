/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005, 2006 0x539 dev group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <glib/gtypes.h>
#include <gtkmm/stockitem.h>
#include "i18n.hpp"
#include "icon.hpp"

#ifdef _WIN32
# include <windows.h>
#endif

namespace
{
	Glib::RefPtr<Gdk::Pixbuf> load_pixbuf(const char* dir, const char* file)
	{
		try
		{
			return Gdk::Pixbuf::create_from_file(
				Glib::build_filename(dir, file)
			);
		}
		catch(Glib::FileError& e)
		{
			/* Not installed */
#ifdef _WIN32
			TCHAR path[MAX_PATH];
			if(!GetModuleFileNameA(NULL, path, MAX_PATH))
				throw e;

			std::string utf_path = Glib::locale_to_utf8(path);

			return Gdk::Pixbuf::create_from_file(
				Glib::build_filename(
					Glib::build_filename(
						Glib::path_get_dirname(
							utf_path
						),
						"pixmaps"
					),
					file
				)
			);
#else
			return Gdk::Pixbuf::create_from_file(
				Glib::build_filename("pixmaps", file)
			);
#endif
		}
	}
}

Gtk::StockID Gobby::IconManager::STOCK_USERLIST("gobby-userlist");
Gtk::StockID Gobby::IconManager::STOCK_DOCLIST("gobby-doclist");
Gtk::StockID Gobby::IconManager::STOCK_CHAT("gobby-chat");

Gobby::IconManager::IconManager():
	gobby(load_pixbuf(APPICON_DIR, "gobby.png") ),
	userlist(load_pixbuf(PIXMAPS_DIR, "userlist.png") ),
	doclist(load_pixbuf(PIXMAPS_DIR, "doclist.png") ),
	chat(load_pixbuf(PIXMAPS_DIR, "chat.png") ),
	m_is_userlist(userlist),
	m_is_doclist(doclist),
	m_is_chat(chat),
	m_icon_factory(Gtk::IconFactory::create() )
{
	Gtk::StockItem userlist_item(STOCK_USERLIST, _("User list") );
	m_icon_factory->add(STOCK_USERLIST, m_is_userlist);

	Gtk::StockItem doclist_item(STOCK_DOCLIST, _("Document list") );
	m_icon_factory->add(STOCK_DOCLIST, m_is_doclist);

	Gtk::StockItem chat_item(STOCK_CHAT, _("Chat") );
	m_icon_factory->add(STOCK_CHAT, m_is_chat);

	m_icon_factory->add_default();
}
