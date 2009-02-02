/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008, 2009 Armin Burgmeier <armin@arbur.net>
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

#include "core/iconmanager.hpp"
#include "util/i18n.hpp"

#include <gtkmm/stockitem.h>
#include <gtkmm/icontheme.h>

Gtk::StockID Gobby::IconManager::STOCK_USERLIST("gobby-user-list");
Gtk::StockID Gobby::IconManager::STOCK_DOCLIST("gobby-document-list");
Gtk::StockID Gobby::IconManager::STOCK_SAVE_ALL("gobby-save-all");

// TODO: The save-all icon does not match the save icon for toolbar
// or menu sized items. It is not yet enabled therefore.
Gobby::IconManager::IconManager():
	m_icon_factory(Gtk::IconFactory::create() )
{
	Gtk::IconTheme::get_default()->append_search_path(ICONS_DIR);

	Gtk::IconSource userlist_source;
	userlist_source.set_icon_name("user-list");
	m_is_userlist.add_source(userlist_source);
	Gtk::StockItem userlist_item(STOCK_USERLIST, _("User list") );
	m_icon_factory->add(STOCK_USERLIST, m_is_userlist);

	Gtk::IconSource doclist_source;
	doclist_source.set_icon_name("document-list");
	m_is_doclist.add_source(doclist_source);
	Gtk::StockItem doclist_item(STOCK_DOCLIST, _("Document list") );
	m_icon_factory->add(STOCK_DOCLIST, m_is_doclist);

	Gtk::StockItem save_all_item(STOCK_SAVE_ALL, _("Save All"));

	m_icon_factory->add_default();
}
