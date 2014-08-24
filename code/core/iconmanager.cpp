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

#include "core/iconmanager.hpp"
#include "util/i18n.hpp"

#include <gtkmm/stockitem.h>
#include <gtkmm/icontheme.h>

Gtk::StockID Gobby::IconManager::STOCK_SAVE_ALL("gobby-save-all");
Gtk::StockID Gobby::IconManager::STOCK_USERLIST("gobby-user-list");
Gtk::StockID Gobby::IconManager::STOCK_DOCLIST("gobby-document-list");
Gtk::StockID Gobby::IconManager::STOCK_CHAT("gobby-chat");
Gtk::StockID Gobby::IconManager::STOCK_USER_COLOR_INDICATOR(
	"gobby-user-color-indicator");

// TODO: The save-all icon does not match the save icon for toolbar
// or menu sized items. It is not yet enabled therefore.
Gobby::IconManager::IconManager():
	m_is_save_all(Gtk::IconSet::create()),
	m_is_userlist(Gtk::IconSet::create()),
	m_is_doclist(Gtk::IconSet::create()),
	m_is_chat(Gtk::IconSet::create()),
	m_is_user_color_indicator(Gtk::IconSet::create()),
	m_icon_factory(Gtk::IconFactory::create())
{
	Gtk::IconTheme::get_default()->append_search_path(PUBLIC_ICONS_DIR);
	Gtk::IconTheme::get_default()->append_search_path(PRIVATE_ICONS_DIR);

	Gtk::StockItem save_all_item(STOCK_SAVE_ALL, _("Save All"));
	//m_icon_factory->add(STOCK_SAVE_ALL, m_is_save_all);

	Gtk::IconSource userlist_source;
	userlist_source.set_icon_name("user-list");
	m_is_userlist->add_source(userlist_source);
	Gtk::StockItem userlist_item(STOCK_USERLIST, _("User list") );
	m_icon_factory->add(STOCK_USERLIST, m_is_userlist);

	Gtk::IconSource doclist_source;
	doclist_source.set_icon_name("document-list");
	m_is_doclist->add_source(doclist_source);
	Gtk::StockItem doclist_item(STOCK_DOCLIST, _("Document list") );
	m_icon_factory->add(STOCK_DOCLIST, m_is_doclist);

	Gtk::IconSource chat_source;
	chat_source.set_icon_name("chat");
	m_is_chat->add_source(chat_source);
	Gtk::StockItem chat_item(STOCK_CHAT, _("Chat") );
	m_icon_factory->add(STOCK_CHAT, m_is_chat);

	Gtk::IconSource user_color_indicator_source;
	user_color_indicator_source.set_icon_name("user-color-indicator");
	m_is_user_color_indicator->add_source(user_color_indicator_source);
	Gtk::StockItem user_color_indicator_item(STOCK_USER_COLOR_INDICATOR,
	                                         _("User Color Indicator"));
	m_icon_factory->add(STOCK_USER_COLOR_INDICATOR,
	                    m_is_user_color_indicator);

	m_icon_factory->add_default();
}
