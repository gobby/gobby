/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008 Armin Burgmeier <armin@arbur.net>
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

#include "core/titlebar.hpp"

Gobby::TitleBar::TitleBar(Gtk::Window& window, Folder& folder):
	m_window(window), m_folder(folder), m_current_document(NULL)
{
	folder.signal_document_removed().connect(
		sigc::mem_fun(*this, &TitleBar::on_document_removed));
	folder.signal_document_changed().connect(
		sigc::mem_fun(*this, &TitleBar::on_document_changed));

	on_document_changed(folder.get_current_document());
}

void Gobby::TitleBar::on_document_removed(DocWindow& document)
{
	if(m_current_document == &document)
		on_document_changed(NULL);
}

void Gobby::TitleBar::on_document_changed(DocWindow* document)
{
	if(m_current_document != NULL)
	{
		InfSession* session = INF_SESSION(
			m_current_document->get_session());
		GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
			m_current_document->get_text_buffer());

		g_signal_handler_disconnect(G_OBJECT(session),
		                            m_notify_status_handler);
		g_signal_handler_disconnect(G_OBJECT(buffer),
		                            m_modified_changed_handler);
	}

	m_current_document = document;

	if(document != NULL)
	{
		InfSession* session = INF_SESSION(document->get_session());
		GtkTextBuffer* buffer =
			GTK_TEXT_BUFFER(document->get_text_buffer());

		m_notify_status_handler = g_signal_connect(
			G_OBJECT(session), "notify::status",
			G_CALLBACK(on_notify_status_static), this);
		m_modified_changed_handler = g_signal_connect(
			G_OBJECT(buffer), "modified-changed",
			G_CALLBACK(on_modified_changed_static), this);
	}

	update_title();
}

void Gobby::TitleBar::on_notify_status()
{
	update_title();
}

void Gobby::TitleBar::on_modified_changed()
{
	update_title();
}

void Gobby::TitleBar::update_title()
{
	// TODO: Show path, as gedit does. This requires change notification
	// for document info storage.
	if(m_current_document != NULL)
	{
		InfSession* session = INF_SESSION(
			m_current_document->get_session());
		GtkTextBuffer* buffer = GTK_TEXT_BUFFER(
			m_current_document->get_text_buffer());

		InfSessionStatus status = inf_session_get_status(session);
		if(status == INF_SESSION_SYNCHRONIZING ||
		   !gtk_text_buffer_get_modified(buffer))
		{
			m_window.set_title(
				m_current_document->get_title() + " - Gobby");
		}
		else
		{
			m_window.set_title(
				"*" + m_current_document->get_title() +
				" - Gobby");
		}
	}
	else
	{
		m_window.set_title("Gobby");
	}
}
