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

#include "operations/operation-new.hpp"

#include "core/noteplugin.hpp"
#include "util/i18n.hpp"

Gobby::OperationNew::OperationNew(Operations& operations,
                                  InfcBrowser* browser,
                                  const InfcBrowserIter* parent,
                                  const Glib::ustring& name,
                                  bool directory):
	Operation(operations), m_name(name), m_directory(directory)
{
	if(directory)
	{
		m_request = infc_browser_add_subdirectory(browser, parent,
		                                          name.c_str());
	}
	else
	{
		m_request = infc_browser_add_note(browser, parent,
		                                  name.c_str(), Plugins::TEXT,
		                                  TRUE);
	}

	// Note infc_browser_add_note does not return a
	// new reference.
	g_object_ref(m_request);

	m_failed_id = g_signal_connect(
		G_OBJECT(m_request), "failed",
		G_CALLBACK(on_request_failed_static), this);
	m_finished_id = g_signal_connect(
		G_OBJECT(m_request), "finished",
		G_CALLBACK(on_request_finished_static), this);

	m_message_handle = get_status_bar().add_info_message(
		Glib::ustring::compose(
			directory ? _("Creating directory %1...")
			          : _("Creating document %1..."), name));
}

Gobby::OperationNew::~OperationNew()
{
	g_signal_handler_disconnect(G_OBJECT(m_request), m_finished_id);
	g_signal_handler_disconnect(G_OBJECT(m_request), m_failed_id);
	g_object_unref(m_request);

	get_status_bar().remove_message(m_message_handle);
}

void Gobby::OperationNew::on_request_failed(const GError* error)
{
	get_status_bar().add_error_message(
		Glib::ustring::compose(
			m_directory ? _("Failed to create directory %1")
			            : _("Failed to create document %1"),
			m_name),
		error->message);

	fail();
}

void Gobby::OperationNew::on_request_finished(InfcBrowserIter* iter)
{
	finish();
}
