/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
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

#include <gtkmm/stock.h>

Gobby::TabLabel::TabLabel(Folder& folder, DocWindow& document):
	Gtk::HBox(false, 6),
	m_folder(folder), m_document(document),
	m_changed(false)
{
	m_title.set_alignment(Gtk::ALIGN_LEFT);

	update_icon();
	update_color();
	update_modified();

	m_icon.show();
	m_title.show();
	m_button.show();

	m_notify_editable_handle = g_signal_connect(
		G_OBJECT(document.get_text_view()), "notify::editable",
		G_CALLBACK(on_notify_editable_static), this);
	m_notify_status_handle = g_signal_connect(
		G_OBJECT(document.get_session()), "notify::status",
		G_CALLBACK(on_notify_status_static), this);
	m_notify_subscription_group_handle = g_signal_connect(
		G_OBJECT(document.get_session()),
		"notify::subscription-group",
		G_CALLBACK(on_notify_subscription_group_static), this);
	m_modified_changed_handle = g_signal_connect_after(
		G_OBJECT(document.get_text_buffer()), "modified-changed",
		G_CALLBACK(on_modified_changed_static), this);
	m_changed_handle = g_signal_connect_after(
		G_OBJECT(document.get_text_buffer()), "changed",
		G_CALLBACK(on_changed_static), this);

	m_folder.signal_document_changed().connect(
		sigc::mem_fun(*this, &TabLabel::on_folder_document_changed));

	pack_start(m_icon, Gtk::PACK_SHRINK);
	pack_start(m_title, Gtk::PACK_EXPAND_WIDGET);
	pack_end(m_button, Gtk::PACK_SHRINK);
}

Gobby::TabLabel::~TabLabel()
{
	g_signal_handler_disconnect(m_document.get_text_view(),
	                            m_notify_editable_handle);
	g_signal_handler_disconnect(m_document.get_session(),
	                            m_notify_status_handle);
	g_signal_handler_disconnect(m_document.get_session(),
	                            m_notify_subscription_group_handle);
	g_signal_handler_disconnect(m_document.get_text_buffer(),
	                            m_modified_changed_handle);
	g_signal_handler_disconnect(m_document.get_text_buffer(),
	                            m_changed_handle);
}

void Gobby::TabLabel::on_notify_editable()
{
	update_icon();
}

void Gobby::TabLabel::on_notify_status()
{
	update_icon();
	update_color();
	update_modified();
}

void Gobby::TabLabel::on_notify_subscription_group()
{
	update_icon();
	update_color();
}

void Gobby::TabLabel::on_modified_changed()
{
	update_modified();
}

void Gobby::TabLabel::on_changed()
{
	if(!m_changed && m_folder.get_current_document() != &m_document)
	{
		InfSession* session = INF_SESSION(m_document.get_session());
		if(inf_session_get_status(session) == INF_SESSION_RUNNING)
		{
			m_changed = true;
			update_color();
		}
	}
}

void Gobby::TabLabel::on_folder_document_changed(DocWindow* document)
{
	if(document == &m_document)
	{
		m_changed = false;
		update_color();
	}
}

void Gobby::TabLabel::update_icon()
{
	InfSession* session = INF_SESSION(m_document.get_session());
	GtkTextView* view = GTK_TEXT_VIEW(m_document.get_text_view());

	if(inf_session_get_subscription_group(session) == NULL)
	{
		m_icon.set(Gtk::Stock::DISCONNECT, Gtk::ICON_SIZE_MENU);
	}
	else
	{
		switch(inf_session_get_status(session))
		{
		case INF_SESSION_SYNCHRONIZING:
			m_icon.set(Gtk::Stock::EXECUTE, Gtk::ICON_SIZE_MENU);
			break;
		case INF_SESSION_RUNNING:
			if(gtk_text_view_get_editable(view))
			{
				m_icon.set(Gtk::Stock::EDIT,
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
	InfSession* session = INF_SESSION(m_document.get_session());

	if(m_changed)
	{
		// Document has changed: awareness -> red
		m_title.modify_fg(Gtk::STATE_NORMAL, Gdk::Color("#c00000"));
		m_title.modify_fg(Gtk::STATE_ACTIVE, Gdk::Color("#c00000"));
	}
	else if(inf_session_get_subscription_group(session) == NULL ||
	        inf_session_get_status(session) != INF_SESSION_RUNNING)
	{
		// Document disconnected or not yet running
		// (most probably synchronizing): not (yet) available -> grey
		m_title.modify_fg(Gtk::STATE_NORMAL, Gdk::Color("#606060"));
		m_title.modify_fg(Gtk::STATE_ACTIVE, Gdk::Color("#606060"));
	}
	else
	{
		Glib::RefPtr<Gtk::Style> default_style =
			Gtk::Widget::get_default_style();

		// Otherwise default
		m_title.modify_fg(
			Gtk::STATE_ACTIVE,
			default_style->get_fg(Gtk::STATE_ACTIVE));
		m_title.modify_fg(
			Gtk::STATE_NORMAL,
			default_style->get_fg(Gtk::STATE_NORMAL));
	}
}

void Gobby::TabLabel::update_modified()
{
	InfSession* session = INF_SESSION(m_document.get_session());
	bool modified = gtk_text_buffer_get_modified(
		GTK_TEXT_BUFFER(m_document.get_text_buffer()));

	if(inf_session_get_status(session) == INF_SESSION_SYNCHRONIZING)
		modified = false;

	if(modified)
		m_title.set_text("*" + m_document.get_title());
	else
		m_title.set_text(m_document.get_title());
}
