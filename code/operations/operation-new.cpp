/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
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
	InfRequest* request;

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
		if(!m_directory)
		{
			InfSessionProxy* proxy =
				inf_browser_get_session(m_browser, iter);
			g_assert(proxy != NULL);

			get_folder_manager().add_document(
				m_browser, iter, proxy, NULL);
		}

		finish();
	}
}
