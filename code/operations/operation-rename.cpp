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

#include "operations/operation-rename.hpp"
#include "util/i18n.hpp"

Gobby::OperationRename::OperationRename(Operations& operations,
                                        InfBrowser* browser,
                                        const InfBrowserIter* iter,
					const Glib::ustring& new_name):
	Operation(operations), m_browser(browser), m_iter(*iter),
	m_name(inf_browser_get_node_name(browser, iter)),
	m_new_name(new_name), m_request(NULL)
{
	g_object_ref(browser);
}

Gobby::OperationRename::~OperationRename()
{
	if(m_request != NULL)
	{
		g_signal_handlers_disconnect_by_func(
			G_OBJECT(m_request),
			(gpointer)G_CALLBACK(on_request_finished_static),
			this);
		g_object_unref(m_request);

		get_status_bar().remove_message(m_message_handle);
	}

	g_object_unref(m_browser);
}

void Gobby::OperationRename::start()
{
	InfRequest* request = inf_browser_rename_node(
		m_browser,
		&m_iter,
		new_name.c_str(),
		on_request_finished_static,
		this
	);

	// Object might be gone at this point, so check stack variable
	if(request != NULL)
	{
		m_request = request;

		// Note infc_browser_rename_node does not return a
		// new reference.
		g_object_ref(m_request);

		m_message_handle = get_status_bar().add_info_message(
			Glib::ustring::compose(
				_("Renaming node \"%1\"..."), m_name));
	}
}

void Gobby::OperationRename::on_request_finished(const GError* error)
{
	if(error)
	{
		get_status_bar().add_error_message(
			Glib::ustring::compose(
				_("Failed to rename node \"%1\""),
				m_name),
			error->message);

		fail();
	}
	else
	{
		finish();
	}
}
