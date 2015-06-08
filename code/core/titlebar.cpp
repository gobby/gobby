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

#include "core/titlebar.hpp"

Gobby::TitleBar::TitleBar(Gtk::Window& window, const Folder& folder):
	m_window(window), m_folder(folder), m_current_view(NULL)
{
	folder.signal_document_removed().connect(
		sigc::mem_fun(*this, &TitleBar::on_document_removed));
	folder.signal_document_changed().connect(
		sigc::mem_fun(*this, &TitleBar::on_document_changed));

	on_document_changed(folder.get_current_document());
}

Gobby::TitleBar::~TitleBar()
{
	on_document_changed(NULL);
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
