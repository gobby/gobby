/* IconZZzzzzzzzzz */

#include <gtkmm/stockitem.h>
#include <gtkmm/icontheme.h>

#include "common.hpp"
#include "icon.hpp"

Gtk::StockID Gobby::IconManager::STOCK_USERLIST("gobby-user-list");
Gtk::StockID Gobby::IconManager::STOCK_DOCLIST("gobby-document-list");
Gtk::StockID Gobby::IconManager::STOCK_CHAT("gobby-chat");

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

	Gtk::IconSource chat_source;
	chat_source.set_icon_name("chat");
	m_is_chat.add_source(chat_source);
	Gtk::StockItem chat_item(STOCK_CHAT, _("Chat") );
	m_icon_factory->add(STOCK_CHAT, m_is_chat);

	m_icon_factory->add_default();
}

