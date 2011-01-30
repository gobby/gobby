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
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "core/tablabel.hpp"
#include "core/folder.hpp"
#include "util/gtk-compat.hpp"

#include <gtkmm/stock.h>

Gobby::TabLabel::TabLabel(Folder& folder, SessionView& view,
                          Gtk::StockID active_icon):
	Gtk::HBox(false, 6), m_folder(folder), m_view(view),
	m_title(view.get_title()), m_changed(false),
	m_active_icon(active_icon)
{
	m_title.set_alignment(GtkCompat::ALIGN_LEFT);

	update_icon();
	update_color();

	m_icon.show();
	m_title.show();
	m_extra.show();
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

	pack_start(m_icon, Gtk::PACK_SHRINK);
	pack_start(m_title, Gtk::PACK_SHRINK);
	pack_start(m_extra, Gtk::PACK_EXPAND_WIDGET);
	pack_end(m_button, Gtk::PACK_SHRINK);
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
		m_icon.set(Gtk::Stock::DISCONNECT, Gtk::ICON_SIZE_MENU);
	}
	else
	{
		switch(inf_session_get_status(session))
		{
		case INF_SESSION_PRESYNC:
		case INF_SESSION_SYNCHRONIZING:
			m_icon.set(Gtk::Stock::EXECUTE, Gtk::ICON_SIZE_MENU);
			break;
		case INF_SESSION_RUNNING:
			if(m_view.get_active_user() != NULL)
			{
				m_icon.set(m_active_icon,
				           Gtk::ICON_SIZE_MENU);
			}
			else
			{
				m_icon.set(Gtk::Stock::FILE,
				           Gtk::ICON_SIZE_MENU);
			}

			break;
		case INF_SESSION_CLOSED:
			m_icon.set(Gtk::Stock::STOP, Gtk::ICON_SIZE_MENU);
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
#ifdef USE_GTKMM3
		m_title.override_color(Gdk::RGBA("#c00000"));
#else
		m_title.modify_fg(Gtk::STATE_NORMAL, Gdk::Color("#c00000"));
		m_title.modify_fg(Gtk::STATE_ACTIVE, Gdk::Color("#c00000"));
#endif
	}
	else if(inf_session_get_subscription_group(session) == NULL ||
	        inf_session_get_status(session) != INF_SESSION_RUNNING)
	{
		// Document disconnected or not yet running
		// (most probably synchronizing): not (yet) available -> grey
#ifdef USE_GTKMM3
		m_title.override_color(Gdk::RGBA("#606060"));
#else
		m_title.modify_fg(Gtk::STATE_NORMAL, Gdk::Color("#606060"));
		m_title.modify_fg(Gtk::STATE_ACTIVE, Gdk::Color("#606060"));
#endif
	}
	else
	{
		// Otherwise default
#ifdef USE_GTKMM3
		m_title.unset_color();
#else
		Glib::RefPtr<Gtk::Style> default_style =
			Gtk::Widget::get_default_style();

		m_title.modify_fg(
			Gtk::STATE_ACTIVE,
			default_style->get_fg(Gtk::STATE_ACTIVE));
		m_title.modify_fg(
			Gtk::STATE_NORMAL,
			default_style->get_fg(Gtk::STATE_NORMAL));
#endif
	}
}
