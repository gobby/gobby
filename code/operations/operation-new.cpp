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

#include "operations/operation-new.hpp"

#include "core/noteplugin.hpp"
#include "util/i18n.hpp"

Gobby::OperationNew::OperationNew(Operations& operations,
                                  InfBrowser* browser,
                                  const InfBrowserIter* parent,
                                  const Glib::ustring& name,
                                  bool directory):
	Operation(operations), m_request(NULL), m_browser(browser),
	m_parent(*parent), m_name(name), m_directory(directory)
{
	g_object_ref(browser);
}

Gobby::OperationNew::~OperationNew()
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

void Gobby::OperationNew::start()
{
	InfNodeRequest* request;

	if(m_directory)
	{
		request = inf_browser_add_subdirectory(
			m_browser, &m_parent, m_name.c_str(), NULL,
			on_request_finished_static, this);
	}
	else
	{
		request = inf_browser_add_note(
			m_browser, &m_parent, m_name.c_str(),
			"InfText", NULL, NULL, TRUE,
			on_request_finished_static, this);
	}

	if(request != NULL)
	{
		m_request = request;
		g_object_ref(m_request);

		m_message_handle = get_status_bar().add_info_message(
			Glib::ustring::compose(
				m_directory ?
					_("Creating directory \"%1\"...") :
					_("Creating document \"%1\"..."),
				m_name));
  	}
}

void Gobby::OperationNew::on_request_finished(const InfBrowserIter* iter,
                                              const GError* error)
{
	if(error)
	{
		const Glib::ustring prefix = Glib::ustring::compose(
			m_directory ?
				_("Failed to create directory \"%1\""):
				_("Failed to create document \"%1\""),
			m_name);

		get_status_bar().add_error_message(prefix, error->message);

		fail();
	}
	else
	{
		InfSessionProxy* proxy =
			inf_browser_get_session(m_browser, iter);
		g_assert(proxy != NULL);

		get_folder_manager().add_document(m_browser, iter, proxy);
		finish();
	}
}
