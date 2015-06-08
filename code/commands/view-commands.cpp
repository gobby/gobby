/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
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

#include "commands/view-commands.hpp"
#include "util/i18n.hpp"

#include <gtkmm/window.h>

#include <libinftextgtk/inf-text-gtk-buffer.h>

class Gobby::ViewCommands::Fullscreen
{
public:
	class Preserve: public sigc::trackable
	{
	public:
		Preserve(const Glib::RefPtr<Gio::Action>& action);
		~Preserve();
	
	private:
		void on_toggled();

		Glib::RefPtr<Gio::Action> m_action;
		bool m_value;
	};

	Fullscreen(ViewCommands& commands);
	~Fullscreen();

private:
	ViewCommands& m_commands;

	Preserve m_preserve_toolbar;
	Preserve m_preserve_browser;
	Preserve m_preserve_chat;
	Preserve m_preserve_document_userlist;
	Preserve m_preserve_chat_userlist;
};

Gobby::ViewCommands::Fullscreen::Preserve::Preserve(
	const Glib::RefPtr<Gio::Action>& action)
:
	m_action(action)
{
	g_assert(action->get_state_type().equal(
		Glib::VariantType(G_VARIANT_TYPE_BOOLEAN)));

	// De-activate all the clutter, but remember their state before
	// we went fullscreen, so that we can restore when we leave
	// fullscreen again.
	action->get_state(m_value);
	action->change_state(false);

	action->property_state().signal_changed().connect(
		sigc::mem_fun(*this, &Preserve::on_toggled));
}

Gobby::ViewCommands::Fullscreen::Preserve::~Preserve()
{
	m_action->change_state(m_value);
}

void Gobby::ViewCommands::Fullscreen::Preserve::on_toggled()
{
	// Update value; if one of the options is toggled we keep the
	// toggled version when fullscreen is left.
	m_action->get_state(m_value);
}

Gobby::ViewCommands::Fullscreen::Fullscreen(ViewCommands& commands):
	m_commands(commands),
	m_preserve_toolbar(commands.m_actions.view_toolbar),
	m_preserve_browser(commands.m_actions.view_browser),
	m_preserve_chat(commands.m_actions.view_chat),
	m_preserve_document_userlist(
		commands.m_actions.view_document_userlist),
	m_preserve_chat_userlist(
		commands.m_actions.view_chat_userlist)
{
	m_commands.m_parent.fullscreen();
}

Gobby::ViewCommands::Fullscreen::~Fullscreen()
{
	m_commands.m_parent.unfullscreen();
}


Gobby::ViewCommands::ViewCommands(Gtk::Window& parent,
                                  WindowActions& actions,
                                  GtkSourceLanguageManager* language_manager,
                                  const Folder& text_folder,
                                  ClosableFrame& chat_frame,
	                          const Folder& chat_folder,
                                  Preferences& preferences):
	m_parent(parent), m_actions(actions),
	m_language_manager(language_manager),
	m_text_folder(text_folder), m_chat_frame(chat_frame),
	m_chat_folder(chat_folder), m_preferences(preferences),
	m_current_view(NULL)
{
	actions.hide_user_colors->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &ViewCommands::on_hide_user_colors)));
	actions.fullscreen->property_state().signal_changed().connect(
		sigc::mem_fun(*this, &ViewCommands::on_fullscreen_toggled));
	actions.zoom_in->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &ViewCommands::on_zoom_in)));
	actions.zoom_out->signal_activate().connect(sigc::hide(
		sigc::mem_fun(*this, &ViewCommands::on_zoom_out)));

	m_menu_view_toolbar_connection = actions.view_toolbar->
		property_state().signal_changed().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_menu_toolbar_toggled));

	m_menu_view_statusbar_connection = actions.view_statusbar->
		property_state().signal_changed().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_menu_statusbar_toggled));

	m_menu_view_browser_connection = actions.view_browser->
		property_state().signal_changed().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_menu_browser_toggled));

	m_menu_view_chat_connection = actions.view_chat->
		property_state().signal_changed().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_menu_chat_toggled));

	m_menu_view_document_userlist_connection =
		actions.view_document_userlist->
			property_state().signal_changed().connect(sigc::mem_fun(
				*this,
				&ViewCommands::
					on_menu_document_userlist_toggled));

	m_menu_view_chat_userlist_connection = actions.view_chat_userlist->
		property_state().signal_changed().connect(
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

	m_menu_language_changed_connection = actions.highlight_mode->
		property_state().signal_changed().connect(
			sigc::mem_fun(
				*this,
				&ViewCommands::on_menu_language_changed));

	m_chat_frame.signal_show().connect(
		sigc::mem_fun(*this, &ViewCommands::on_chat_show));
	m_chat_frame.signal_hide().connect(
		sigc::mem_fun(*this, &ViewCommands::on_chat_hide));

	// Chat View by default not sensitive, becomes sensitive if a server
	// connection is made.
	actions.view_chat->set_enabled(false);
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
		m_actions.hide_user_colors->set_enabled(true);
		m_actions.zoom_in->set_enabled(true);
		m_actions.zoom_out->set_enabled(true);
		m_actions.highlight_mode->set_enabled(true);
		m_actions.view_document_userlist->set_enabled(true);

		m_document_language_changed_connection =
			m_current_view->signal_language_changed().connect(
				sigc::mem_fun(
					*this,
					&ViewCommands::
						on_doc_language_changed));
	}
	else
	{
		m_actions.hide_user_colors->set_enabled(false);
		m_actions.zoom_in->set_enabled(false);
		m_actions.zoom_out->set_enabled(false);

		m_menu_language_changed_connection.block();
		m_actions.highlight_mode->set_enabled(false);
		m_actions.highlight_mode->change_state(
			Glib::Variant<Glib::ustring>::create(""));
		m_menu_language_changed_connection.unblock();

		m_actions.view_document_userlist->set_enabled(false);
	}

	on_doc_language_changed(
		m_current_view ? m_current_view->get_language() : NULL);
}

void Gobby::ViewCommands::on_chat_document_added(SessionView& view)
{
	// Allow the chat frame to be visible if the option allows it
	m_chat_frame.set_allow_visible(true);

	m_actions.view_chat->set_enabled(true);
}

void Gobby::ViewCommands::on_chat_document_removed(SessionView& view)
{
	if(m_chat_folder.get_n_pages() == 1)
	{
		// This is the last document, and it is about to be removed.
		m_actions.view_chat->set_enabled(false);
		// Hide the chat frame independent of the option
		m_chat_frame.set_allow_visible(false);
	}
}

void Gobby::ViewCommands::on_chat_document_changed(SessionView* view)
{
	if(view != NULL)
	{
		if(m_chat_frame.get_visible())
		{
			m_actions.view_chat_userlist->set_enabled(true);
		}
	}
	else
	{
		m_actions.view_chat_userlist->set_enabled(false);
	}
}

void Gobby::ViewCommands::on_chat_show()
{
	SessionView* view = m_chat_folder.get_current_document();
	if(view != NULL)
		m_actions.view_chat_userlist->set_enabled(true);
}

void Gobby::ViewCommands::on_chat_hide()
{
	m_actions.view_chat_userlist->set_enabled(false);
}

void Gobby::ViewCommands::on_zoom_in()
{
	Pango::FontDescription desc = m_preferences.appearance.font;
	desc.set_size(desc.get_size() * PANGO_SCALE_LARGE);
	m_preferences.appearance.font = desc;
}

void Gobby::ViewCommands::on_zoom_out()
{
	Pango::FontDescription desc = m_preferences.appearance.font;
	desc.set_size(desc.get_size() / PANGO_SCALE_LARGE);
	m_preferences.appearance.font = desc;
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

void Gobby::ViewCommands::on_fullscreen_toggled()
{
	bool is_fullscreen;
	m_actions.fullscreen->get_state(is_fullscreen);

	if(is_fullscreen)
	{
		if(!m_fullscreen.get())
			m_fullscreen.reset(new Fullscreen(*this));
	}
	else
	{
		m_fullscreen.reset(NULL);
	}
}

void Gobby::ViewCommands::on_menu_toolbar_toggled()
{
	bool value;
	m_actions.view_toolbar->get_state(value);

	m_pref_view_toolbar_connection.block();
	m_preferences.appearance.show_toolbar = value;
	m_pref_view_toolbar_connection.unblock();
}

void Gobby::ViewCommands::on_menu_statusbar_toggled()
{
	bool value;
	m_actions.view_statusbar->get_state(value);

	m_pref_view_statusbar_connection.block();
	m_preferences.appearance.show_statusbar = value;
	m_pref_view_statusbar_connection.unblock();
}

void Gobby::ViewCommands::on_menu_browser_toggled()
{
	bool value;
	m_actions.view_browser->get_state(value);

	m_pref_view_browser_connection.block();
	m_preferences.appearance.show_browser = value;
	m_pref_view_browser_connection.unblock();
}

void Gobby::ViewCommands::on_menu_chat_toggled()
{
	bool value;
	m_actions.view_chat->get_state(value);

	m_pref_view_chat_connection.block();
	m_preferences.appearance.show_chat = value;
	m_pref_view_chat_connection.unblock();
}

void Gobby::ViewCommands::on_menu_document_userlist_toggled()
{
	bool value;
	m_actions.view_document_userlist->get_state(value);

	m_pref_view_document_userlist_connection.block();
	m_preferences.appearance.show_document_userlist = value;
	m_pref_view_document_userlist_connection.unblock();
}

void Gobby::ViewCommands::on_menu_chat_userlist_toggled()
{
	bool value;
	m_actions.view_chat_userlist->get_state(value);

	m_pref_view_chat_userlist_connection.block();
	m_preferences.appearance.show_chat_userlist = value;
	m_pref_view_chat_userlist_connection.unblock();
}

void Gobby::ViewCommands::on_pref_toolbar_changed()
{
	m_menu_view_toolbar_connection.block();
	m_actions.view_toolbar->change_state(
		static_cast<bool>(m_preferences.appearance.show_toolbar));
	m_menu_view_toolbar_connection.unblock();
}

void Gobby::ViewCommands::on_pref_statusbar_changed()
{
	m_menu_view_statusbar_connection.block();
	m_actions.view_statusbar->change_state(
		static_cast<bool>(m_preferences.appearance.show_statusbar));
	m_menu_view_statusbar_connection.unblock();
}

void Gobby::ViewCommands::on_pref_browser_changed()
{
	m_menu_view_browser_connection.block();
	m_actions.view_browser->change_state(
		static_cast<bool>(m_preferences.appearance.show_browser));
	m_menu_view_browser_connection.unblock();
}

void Gobby::ViewCommands::on_pref_chat_changed()
{
	m_menu_view_chat_connection.block();
	m_actions.view_chat->change_state(
		static_cast<bool>(m_preferences.appearance.show_chat));
	m_menu_view_chat_connection.unblock();
}

void Gobby::ViewCommands::on_pref_document_userlist_changed()
{
	m_menu_view_document_userlist_connection.block();
	m_actions.view_document_userlist->change_state(
		static_cast<bool>(
			m_preferences.appearance.show_document_userlist));
	m_menu_view_document_userlist_connection.unblock();
}

void Gobby::ViewCommands::on_pref_chat_userlist_changed()
{
	m_menu_view_chat_userlist_connection.block();
	m_actions.view_chat_userlist->change_state(
		static_cast<bool>(
			m_preferences.appearance.show_chat_userlist));
	m_menu_view_chat_userlist_connection.unblock();
}

void Gobby::ViewCommands::on_menu_language_changed()
{
	// TODO: Get ID, lookup language, set language.
	Glib::ustring language_id;
	m_actions.highlight_mode->get_state(language_id);

	GtkSourceLanguage* language = NULL;
	if(!language_id.empty())
	{
		language = gtk_source_language_manager_get_language(
			m_language_manager, language_id.c_str());

		// The language should exist by construction, if the languages
		// available in the language manager don't change at runtime
		g_assert(language != NULL);
	}

	g_assert(m_current_view != NULL);

	m_document_language_changed_connection.block();
	m_current_view->set_language(language);
	m_document_language_changed_connection.unblock();
}

void Gobby::ViewCommands::on_doc_language_changed(GtkSourceLanguage* language)
{
	m_menu_language_changed_connection.block();
	const gchar* language_id = "";
	if(language != NULL)
		language_id = gtk_source_language_get_id(language);
	m_actions.highlight_mode->change_state(
		Glib::Variant<Glib::ustring>::create(language_id));
	m_menu_language_changed_connection.unblock();
}
