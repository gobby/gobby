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
	ViewCommands(Header& header, const Folder& text_folder,
	             ClosableFrame& chat_frame, const Folder& chat_folder,
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
	void on_zoom_in();
	void on_zoom_out();
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
	const Folder& m_text_folder;
	ClosableFrame& m_chat_frame;
	const Folder& m_chat_folder;
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
