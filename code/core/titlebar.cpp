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

#include "core/titlebar.hpp"

Gobby::TitleBar::TitleBar(Gtk::Window& window, Folder& folder):
	m_window(window), m_folder(folder), m_current_view(NULL)
{
	folder.signal_document_removed().connect(
		sigc::mem_fun(*this, &TitleBar::on_document_removed));
	folder.signal_document_changed().connect(
		sigc::mem_fun(*this, &TitleBar::on_document_changed));

	on_document_changed(folder.get_current_document());
}

void Gobby::TitleBar::on_document_removed(SessionView& view)
{
	// TODO: Isn't this called by Folder already?
	if(m_current_view == &view)
		on_document_changed(NULL);
}

void Gobby::TitleBar::on_document_changed(SessionView* view)
{
	if(m_current_view != NULL)
	{
		InfSession* session = m_current_view->get_session();
		InfBuffer* buffer = inf_session_get_buffer(session);

		g_signal_handler_disconnect(G_OBJECT(session),
		                            m_notify_status_handler);
		g_signal_handler_disconnect(G_OBJECT(buffer),
		                            m_modified_changed_handler);
	}

	m_current_view = view;

	if(view != NULL)
	{
		InfSession* session = view->get_session();
		InfBuffer* buffer = inf_session_get_buffer(session);

		m_notify_status_handler = g_signal_connect(
			G_OBJECT(session), "notify::status",
			G_CALLBACK(on_notify_status_static), this);
		m_modified_changed_handler = g_signal_connect(
			G_OBJECT(buffer), "notify::modified",
			G_CALLBACK(on_notify_modified_static), this);
	}

	update_title();
}

void Gobby::TitleBar::on_notify_status()
{
	update_title();
}

void Gobby::TitleBar::on_notify_modified()
{
	update_title();
}

void Gobby::TitleBar::update_title()
{
	// TODO: Show path, as gedit does. This requires change notification
	// for document info storage.
	if(m_current_view != NULL)
	{
		InfSession* session = m_current_view->get_session();
		InfBuffer* buffer = inf_session_get_buffer(session);

		InfSessionStatus status = inf_session_get_status(session);
		if(status == INF_SESSION_SYNCHRONIZING ||
		   !inf_buffer_get_modified(buffer))
		{
			m_window.set_title(
				m_current_view->get_title() + " - Gobby");
		}
		else
		{
			m_window.set_title(
				"*" + m_current_view->get_title() +
				" - Gobby");
		}
	}
	else
	{
		m_window.set_title("Gobby");
	}
}
