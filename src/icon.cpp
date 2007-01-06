/* GdkPixbuf RGBA C-Source image dump */

#include <glib/gtypes.h>
#include <gtkmm/stockitem.h>
#include "common.hpp"
#include "icon.hpp"

Gtk::StockID Gobby::IconManager::STOCK_USERLIST("gobby-userlist");
Gtk::StockID Gobby::IconManager::STOCK_DOCLIST("gobby-doclist");
Gtk::StockID Gobby::IconManager::STOCK_CHAT("gobby-chat");

Gobby::IconManager::IconManager():
	gobby(Gdk::Pixbuf::create_from_file(APPICON_DIR"/gobby.png") ),
	userlist(Gdk::Pixbuf::create_from_file(PIXMAPS_DIR"/userlist.png") ),
	doclist(Gdk::Pixbuf::create_from_file(PIXMAPS_DIR"/doclist.png") ),
	chat(Gdk::Pixbuf::create_from_file(PIXMAPS_DIR"/chat.png") ),
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

