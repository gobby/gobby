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

#include "commands/view-commands.hpp"
#include "util/i18n.hpp"

Gobby::ViewCommands::ViewCommands(Header& header, Folder& folder,
                                  Preferences& preferences):
	m_header(header), m_folder(folder), m_preferences(preferences),
	m_current_view(NULL)
{
	m_menu_view_toolbar_connection = 
		m_header.action_view_toolbar->signal_toggled().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_menu_toolbar_toggled));

	m_menu_view_statusbar_connection =
		m_header.action_view_statusbar->signal_toggled().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_menu_statusbar_toggled));

	m_menu_view_browser_connection =
		m_header.action_view_browser->signal_toggled().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_menu_browser_toggled));

	m_menu_view_userlist_connection =
		m_header.action_view_userlist->signal_toggled().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_menu_userlist_toggled));

	m_pref_view_toolbar_connection =
		preferences.appearance.show_toolbar.signal_changed().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_pref_toolbar_changed));

	m_pref_view_statusbar_connection =
		preferences.appearance.show_statusbar.signal_changed().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_pref_statusbar_changed));

	m_pref_view_browser_connection =
		preferences.appearance.show_browser.signal_changed().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_pref_browser_changed));

	m_pref_view_userlist_connection =
		preferences.appearance.show_userlist.signal_changed().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_pref_userlist_changed));

	m_folder.signal_document_changed().connect(
		sigc::mem_fun(*this, &ViewCommands::on_document_changed));

	m_menu_language_changed_connection =
		m_header.action_view_highlight_none->signal_changed().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_menu_language_changed));

	// Setup initial sensitivity:
	on_document_changed(m_folder.get_current_document());
}

Gobby::ViewCommands::~ViewCommands()
{
	// Disconnect handlers from current document:
	on_document_changed(NULL);
}

void Gobby::ViewCommands::on_document_changed(SessionView* view)
{
	if(m_current_view != NULL)
		m_document_language_changed_connection.disconnect();

	m_current_view = dynamic_cast<TextSessionView*>(view);

	if(m_current_view != NULL)
	{
		m_header.action_view_highlight_mode->set_sensitive(true);
		m_header.action_view_userlist->set_sensitive(true);

		m_document_language_changed_connection =
			m_current_view->signal_language_changed().connect(
				sigc::mem_fun(
					*this,
					&ViewCommands::
						on_doc_language_changed));
	}
	else
	{
		m_menu_language_changed_connection.block();
		m_header.action_view_highlight_mode->set_sensitive(false);
		m_header.action_view_highlight_none->set_active(true);
		m_menu_language_changed_connection.unblock();

		// Can toggle userlist also if it's not a textsession:
		m_header.action_view_userlist->set_sensitive(view != NULL);
	}

	on_doc_language_changed(
		m_current_view ? m_current_view->get_language() : NULL);
}

void Gobby::ViewCommands::on_menu_toolbar_toggled()
{
	m_pref_view_toolbar_connection.block();
	m_preferences.appearance.show_toolbar =
		m_header.action_view_toolbar->get_active();
	m_pref_view_toolbar_connection.unblock();
}

void Gobby::ViewCommands::on_menu_statusbar_toggled()
{
	m_pref_view_statusbar_connection.block();
	m_preferences.appearance.show_statusbar =
		m_header.action_view_statusbar->get_active();
	m_pref_view_statusbar_connection.unblock();
}

void Gobby::ViewCommands::on_menu_browser_toggled()
{
	m_pref_view_browser_connection.block();
	m_preferences.appearance.show_browser =
		m_header.action_view_browser->get_active();
	m_pref_view_browser_connection.unblock();
}

void Gobby::ViewCommands::on_menu_userlist_toggled()
{
	m_pref_view_userlist_connection.block();
	m_preferences.appearance.show_userlist =
		m_header.action_view_userlist->get_active();
	m_pref_view_userlist_connection.unblock();
}

void Gobby::ViewCommands::on_pref_toolbar_changed()
{
	m_menu_view_toolbar_connection.block();
	m_header.action_view_toolbar->set_active(
		m_preferences.appearance.show_toolbar);
	m_menu_view_toolbar_connection.unblock();
}

void Gobby::ViewCommands::on_pref_statusbar_changed()
{
	m_menu_view_statusbar_connection.block();
	m_header.action_view_statusbar->set_active(
		m_preferences.appearance.show_statusbar);
	m_menu_view_statusbar_connection.unblock();
}

void Gobby::ViewCommands::on_pref_browser_changed()
{
	m_menu_view_browser_connection.block();
	m_header.action_view_browser->set_active(
		m_preferences.appearance.show_browser);
	m_menu_view_browser_connection.unblock();
}

void Gobby::ViewCommands::on_pref_userlist_changed()
{
	m_menu_view_userlist_connection.block();
	m_header.action_view_userlist->set_active(
		m_preferences.appearance.show_userlist);
	m_menu_view_userlist_connection.unblock();
}

void Gobby::ViewCommands::on_menu_language_changed(
	const Glib::RefPtr<Gtk::RadioAction>& action)
{
	Glib::RefPtr<Header::LanguageAction> language_action =
		Glib::RefPtr<Header::LanguageAction>::cast_static(action);

	g_assert(m_current_view != NULL);

	m_document_language_changed_connection.block();
	m_current_view->set_language(language_action->get_language());
	m_document_language_changed_connection.unblock();
}

void Gobby::ViewCommands::on_doc_language_changed(GtkSourceLanguage* language)
{
	// Select the language of document:
	const Glib::RefPtr<Header::LanguageAction> action =
		(language != NULL) ?
			m_header.lookup_language_action(
				m_current_view->get_language()) :
			m_header.action_view_highlight_none;

	m_menu_language_changed_connection.block();
	// lookup_language_action guarantees not to return NULL:
	action->set_active(true);
	m_menu_language_changed_connection.unblock();
}
