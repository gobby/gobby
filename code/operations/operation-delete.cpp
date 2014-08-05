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

#include "operations/operation-delete.hpp"
#include "util/i18n.hpp"

Gobby::OperationDelete::OperationDelete(Operations& operations,
                                        InfBrowser* browser,
                                        const InfBrowserIter* iter):
	Operation(operations), m_browser(browser), m_iter(*iter),
	m_name(inf_browser_get_node_name(browser, iter)), m_request(NULL)
{
	g_object_ref(browser);
}

Gobby::OperationDelete::~OperationDelete()
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

void Gobby::OperationDelete::start()
{
	InfRequest* request = inf_browser_remove_node(
		m_browser,
		&m_iter,
		on_request_finished_static,
		this
	);

	// Object might be gone at this point, so check stack variable
	if(request != NULL)
	{
		m_request = request;

		// Note infc_browser_remove_node does not return a
		// new reference.
		g_object_ref(m_request);

		m_message_handle = get_status_bar().add_info_message(
			Glib::ustring::compose(
				_("Removing node \"%1\"..."), m_name));
	}
}

void Gobby::OperationDelete::on_request_finished(const GError* error)
{
	if(error)
	{
		get_status_bar().add_error_message(
			Glib::ustring::compose(
				_("Failed to delete node \"%1\""),
				m_name),
			error->message);

		fail();
	}
	else
	{
		finish();
	}
}
