/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
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

#include "commands/view-commands.hpp"
#include "util/i18n.hpp"
#include "util/gtk-compat.hpp"

#include <libinftextgtk/inf-text-gtk-buffer.h>

Gobby::ViewCommands::ViewCommands(Header& header, const Folder& text_folder,
                                  ClosableFrame& chat_frame,
	                          const Folder& chat_folder,
                                  Preferences& preferences):
	m_header(header), m_text_folder(text_folder),
	m_chat_frame(chat_frame), m_chat_folder(chat_folder),
	m_preferences(preferences), m_current_view(NULL)
{
	m_header.action_view_hide_user_colors->signal_activate().connect(
		sigc::mem_fun(*this, &ViewCommands::on_hide_user_colors));

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

	m_menu_view_chat_connection =
		m_header.action_view_chat->signal_toggled().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_menu_chat_toggled));

	m_menu_view_document_userlist_connection =
		m_header.action_view_document_userlist->
			signal_toggled().connect(sigc::mem_fun(
				*this,
				&ViewCommands::
					on_menu_document_userlist_toggled));

	m_menu_view_chat_userlist_connection =
		m_header.action_view_chat_userlist->signal_toggled().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::
					on_menu_chat_userlist_toggled));

	// Shortcut:
	Preferences::Appearance& appearance = preferences.appearance;

	m_pref_view_toolbar_connection =
		appearance.show_toolbar.signal_changed().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_pref_toolbar_changed));

	m_pref_view_statusbar_connection =
		appearance.show_statusbar.signal_changed().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_pref_statusbar_changed));

	m_pref_view_browser_connection =
		appearance.show_browser.signal_changed().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_pref_browser_changed));

	m_pref_view_chat_connection =
		appearance.show_chat.signal_changed().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_pref_chat_changed));

	m_pref_view_document_userlist_connection =
		appearance.show_document_userlist.signal_changed().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::
					on_pref_document_userlist_changed));

	m_pref_view_chat_userlist_connection =
		appearance.show_chat_userlist.signal_changed().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::
					on_pref_chat_userlist_changed));

	m_text_folder.signal_document_changed().connect(
		sigc::mem_fun(
			*this, &ViewCommands::on_text_document_changed));

	m_chat_folder.signal_document_added().connect(
		sigc::mem_fun(
			*this, &ViewCommands::on_chat_document_added));

	m_chat_folder.signal_document_removed().connect(
		sigc::mem_fun(
			*this, &ViewCommands::on_chat_document_removed));

	m_chat_folder.signal_document_changed().connect(
		sigc::mem_fun(
			*this, &ViewCommands::on_chat_document_changed));

	m_menu_language_changed_connection =
		m_header.action_view_highlight_none->signal_changed().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_menu_language_changed));

	m_chat_frame.signal_show().connect(
		sigc::mem_fun(*this, &ViewCommands::on_chat_show));
	m_chat_frame.signal_hide().connect(
		sigc::mem_fun(*this, &ViewCommands::on_chat_hide));

	// Chat View by default not sensitive, becomes sensitive if a server
	// connection is made.
	m_header.action_view_chat->set_sensitive(false);
	m_chat_frame.set_allow_visible(false);

	// Setup initial sensitivity:
	on_text_document_changed(m_text_folder.get_current_document());
	on_chat_document_changed(m_chat_folder.get_current_document());
}

Gobby::ViewCommands::~ViewCommands()
{
	// Disconnect handlers from current document:
	on_text_document_changed(NULL);
	on_chat_document_changed(NULL);
}

void Gobby::ViewCommands::on_text_document_changed(SessionView* view)
{
	if(m_current_view != NULL)
		m_document_language_changed_connection.disconnect();

	m_current_view = dynamic_cast<TextSessionView*>(view);

	if(m_current_view != NULL)
	{
		m_header.action_view_hide_user_colors->set_sensitive(true);
		m_header.action_view_highlight_mode->set_sensitive(true);
		m_header.action_view_document_userlist->set_sensitive(true);

		m_document_language_changed_connection =
			m_current_view->signal_language_changed().connect(
				sigc::mem_fun(
					*this,
					&ViewCommands::
						on_doc_language_changed));
	}
	else
	{
		m_header.action_view_hide_user_colors->set_sensitive(false);

		m_menu_language_changed_connection.block();
		m_header.action_view_highlight_mode->set_sensitive(false);
		m_header.action_view_highlight_none->set_active(true);
		m_menu_language_changed_connection.unblock();

		m_header.action_view_document_userlist->set_sensitive(false);
	}

	on_doc_language_changed(
		m_current_view ? m_current_view->get_language() : NULL);
}

void Gobby::ViewCommands::on_chat_document_added(SessionView& view)
{
	// Allow the chat frame to be visible if the option allows it
	m_chat_frame.set_allow_visible(true);

	m_header.action_view_chat->set_sensitive(true);
}

void Gobby::ViewCommands::on_chat_document_removed(SessionView& view)
{
	if(m_chat_folder.get_n_pages() == 1)
	{
		// This is the last document, and it is about to be removed.
		m_header.action_view_chat->set_sensitive(false);
		// Hide the chat frame independent of the option
		m_chat_frame.set_allow_visible(false);
	}
}

void Gobby::ViewCommands::on_chat_document_changed(SessionView* view)
{
	if(view != NULL)
	{
		if(GtkCompat::is_visible(m_chat_frame))
		{
			m_header.action_view_chat_userlist->set_sensitive(
				true);
		}
	}
	else
	{
		m_header.action_view_chat_userlist->set_sensitive(false);
	}
}

void Gobby::ViewCommands::on_chat_show()
{
	SessionView* view = m_chat_folder.get_current_document();
	if(view != NULL)
		m_header.action_view_chat_userlist->set_sensitive(true);
}

void Gobby::ViewCommands::on_chat_hide()
{
	m_header.action_view_chat_userlist->set_sensitive(false);
}

void Gobby::ViewCommands::on_hide_user_colors()
{
	SessionView* view = m_text_folder.get_current_document();
	TextSessionView* text_view = dynamic_cast<TextSessionView*>(view);
	g_assert(text_view != NULL);

	InfSession* session = INF_SESSION(text_view->get_session());
	GtkTextBuffer* textbuffer =
		GTK_TEXT_BUFFER(text_view->get_text_buffer());
	InfBuffer* buffer = inf_session_get_buffer(session);
	InfTextGtkBuffer* infbuffer = INF_TEXT_GTK_BUFFER(buffer);

	GtkTextIter start, end;
	gtk_text_buffer_get_start_iter(textbuffer, &start);
	gtk_text_buffer_get_end_iter(textbuffer, &end);

	inf_text_gtk_buffer_show_user_colors(infbuffer, FALSE, &start, &end);
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

void Gobby::ViewCommands::on_menu_chat_toggled()
{
	m_pref_view_chat_connection.block();
	m_preferences.appearance.show_chat =
		m_header.action_view_chat->get_active();
	m_pref_view_chat_connection.unblock();
}

void Gobby::ViewCommands::on_menu_document_userlist_toggled()
{
	m_pref_view_document_userlist_connection.block();
	m_preferences.appearance.show_document_userlist =
		m_header.action_view_document_userlist->get_active();
	m_pref_view_document_userlist_connection.unblock();
}

void Gobby::ViewCommands::on_menu_chat_userlist_toggled()
{
	m_pref_view_chat_userlist_connection.block();
	m_preferences.appearance.show_chat_userlist =
		m_header.action_view_chat_userlist->get_active();
	m_pref_view_chat_userlist_connection.unblock();
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

void Gobby::ViewCommands::on_pref_chat_changed()
{
	m_menu_view_chat_connection.block();
	m_header.action_view_chat->set_active(
		m_preferences.appearance.show_chat);
	m_menu_view_chat_connection.unblock();
}

void Gobby::ViewCommands::on_pref_document_userlist_changed()
{
	m_menu_view_document_userlist_connection.block();
	m_header.action_view_document_userlist->set_active(
		m_preferences.appearance.show_document_userlist);
	m_menu_view_document_userlist_connection.unblock();
}

void Gobby::ViewCommands::on_pref_chat_userlist_changed()
{
	m_menu_view_chat_userlist_connection.block();
	m_header.action_view_chat_userlist->set_active(
		m_preferences.appearance.show_chat_userlist);
	m_menu_view_chat_userlist_connection.unblock();
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
