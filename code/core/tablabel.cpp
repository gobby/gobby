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

#include "core/tablabel.hpp"
#include "core/folder.hpp"

Gobby::TabLabel::TabLabel(Folder& folder, SessionView& view,
                          const Glib::ustring& active_icon_name):
	m_folder(folder), m_view(view),
	m_title(view.get_title()), m_changed(false),
	m_active_icon_name(active_icon_name)
{
	set_column_spacing(6);
	m_title.set_halign(Gtk::ALIGN_START);

	update_icon();
	update_color();

	m_icon.show();
	m_title.show();
	m_button.set_halign(Gtk::ALIGN_END);
	m_button.show();

	view.signal_active_user_changed().connect(
		sigc::mem_fun(*this, &TabLabel::on_active_user_changed));

	m_notify_status_handle = g_signal_connect(
		G_OBJECT(view.get_session()), "notify::status",
		G_CALLBACK(on_notify_status_static), this);
	m_notify_subscription_group_handle = g_signal_connect(
		G_OBJECT(view.get_session()),
		"notify::subscription-group",
		G_CALLBACK(on_notify_subscription_group_static), this);

	m_folder.signal_document_changed().connect(
		sigc::mem_fun(*this, &TabLabel::on_folder_document_changed));

	attach(m_icon, 0, 0, 1, 1);
	attach(m_title, 1, 0, 1, 1);
	attach(m_button, 2, 0, 1, 1);
}

Gobby::TabLabel::~TabLabel()
{
	g_signal_handler_disconnect(m_view.get_session(),
	                            m_notify_status_handle);
	g_signal_handler_disconnect(m_view.get_session(),
	                            m_notify_subscription_group_handle);
}

void Gobby::TabLabel::on_folder_document_changed(SessionView* view)
{
	if(view == &m_view)
		on_activate();
}

void Gobby::TabLabel::on_active_user_changed(InfUser* user)
{
	update_icon();
}

void Gobby::TabLabel::on_notify_status()
{
	update_icon();
	update_color();
}

void Gobby::TabLabel::on_notify_subscription_group()
{
	update_icon();
	update_color();
}

void Gobby::TabLabel::on_activate()
{
	m_changed = false;
	update_color();
}

void Gobby::TabLabel::set_changed()
{
	if(m_folder.get_current_document() != &m_view)
	{
		m_changed = true;
		update_color();
	}
}

void Gobby::TabLabel::update_icon()
{
	InfSession* session = INF_SESSION(m_view.get_session());

	if(inf_session_get_subscription_group(session) == NULL)
	{
		m_icon.set_from_icon_name("network-offline",
		                          Gtk::ICON_SIZE_MENU);
	}
	else
	{
		switch(inf_session_get_status(session))
		{
		case INF_SESSION_PRESYNC:
		case INF_SESSION_SYNCHRONIZING:
			// TODO: Switch to process-working, if/when m_icon can
			// show animations.
			m_icon.set_from_icon_name("system-run",
			                          Gtk::ICON_SIZE_MENU);
			break;
		case INF_SESSION_RUNNING:
			if(m_view.get_active_user() != NULL)
			{
				m_icon.set_from_icon_name(m_active_icon_name,
				                          Gtk::ICON_SIZE_MENU);
			}
			else
			{
				m_icon.set_from_icon_name("text-x-generic",
				                          Gtk::ICON_SIZE_MENU);
			}

			break;
		case INF_SESSION_CLOSED:
			m_icon.set_from_icon_name("network-offline",
			                          Gtk::ICON_SIZE_MENU);
			break;
		}
	}
}

void Gobby::TabLabel::update_color()
{
	InfSession* session = INF_SESSION(m_view.get_session());

	if(m_changed)
	{
		// Document has changed: awareness -> red
		m_title.override_color(Gdk::RGBA("#c00000"));
	}
	else if(inf_session_get_subscription_group(session) == NULL ||
	        inf_session_get_status(session) != INF_SESSION_RUNNING)
	{
		// Document disconnected or not yet running
		// (most probably synchronizing): not (yet) available -> grey
		m_title.override_color(Gdk::RGBA("#606060"));
	}
	else
	{
		// Otherwise default
		m_title.unset_color();
	}
}
