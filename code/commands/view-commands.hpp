/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2011 Armin Burgmeier <armin@arbur.net>
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
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _GOBBY_VIEW_COMMANDS_HPP_
#define _GOBBY_VIEW_COMMANDS_HPP_

#include "core/header.hpp"
#include "core/folder.hpp"
#include "core/closableframe.hpp"

#include <sigc++/trackable.h>

namespace Gobby
{

class ViewCommands: public sigc::trackable
{
public:
	ViewCommands(Header& header, Folder& text_folder,
	             ClosableFrame& chat_frame, Folder& chat_folder,
	             Preferences& preferences);
	~ViewCommands();

protected:
	void on_text_document_changed(SessionView* view);

	void on_chat_document_added(SessionView& view);
	void on_chat_document_removed(SessionView& view);
	void on_chat_document_changed(SessionView* view);

	void on_chat_show();
	void on_chat_hide();

	void on_hide_user_colors();

	void on_menu_toolbar_toggled();
	void on_menu_statusbar_toggled();
	void on_menu_browser_toggled();
	void on_menu_chat_toggled();
	void on_menu_document_userlist_toggled();
	void on_menu_chat_userlist_toggled();

	void on_pref_toolbar_changed();
	void on_pref_statusbar_changed();
	void on_pref_browser_changed();
	void on_pref_chat_changed();
	void on_pref_document_userlist_changed();
	void on_pref_chat_userlist_changed();

	void on_menu_language_changed(
		const Glib::RefPtr<Gtk::RadioAction>& action);
	void on_doc_language_changed(GtkSourceLanguage* language);

	Header& m_header;
	Folder& m_text_folder;
	ClosableFrame& m_chat_frame;
	Folder& m_chat_folder;
	Preferences& m_preferences;

	TextSessionView* m_current_view;

	sigc::connection m_menu_language_changed_connection;
	sigc::connection m_document_language_changed_connection;

	sigc::connection m_menu_view_toolbar_connection;
	sigc::connection m_menu_view_statusbar_connection;
	sigc::connection m_menu_view_browser_connection;
	sigc::connection m_menu_view_chat_connection;
	sigc::connection m_menu_view_document_userlist_connection;
	sigc::connection m_menu_view_chat_userlist_connection;

	sigc::connection m_pref_view_statusbar_connection;
	sigc::connection m_pref_view_toolbar_connection;
	sigc::connection m_pref_view_browser_connection;
	sigc::connection m_pref_view_chat_connection;
	sigc::connection m_pref_view_document_userlist_connection;
	sigc::connection m_pref_view_chat_userlist_connection;

private:
	void ensure_find_dialog();
};

}

#endif // _GOBBY_VIEW_COMMANDS_HPP_
