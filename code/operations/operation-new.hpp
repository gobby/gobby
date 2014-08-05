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

#ifndef _GOBBY_OPERATIONS_OPERATIONNEW_HPP_
#define _GOBBY_OPERATIONS_OPERATIONNEW_HPP_

#include "operations/operations.hpp"

#include <libinfinity/common/inf-request-result.h>

namespace Gobby
{

class OperationNew: public Operations::Operation
{
public:
	OperationNew(Operations& operations, InfBrowser* browser,
	             const InfBrowserIter* parent, const Glib::ustring& name,
	             bool directory);

	virtual ~OperationNew();

	virtual void start();

protected:
	static void
	on_request_finished_static(InfRequest* request,
	                           const InfRequestResult* result,
	                           const GError* error,
	                           gpointer user_data)
	{
		const InfBrowserIter* iter;
		inf_request_result_get_add_node(result, NULL, NULL, &iter);

		static_cast<OperationNew*>(user_data)->
			on_request_finished(iter, error);
	}

	void on_request_finished(const InfBrowserIter* iter,
	                         const GError* error);

protected:
	InfBrowser* m_browser;
	const InfBrowserIter m_parent;
	Glib::ustring m_name;
	bool m_directory;
	InfRequest* m_request;

	StatusBar::MessageHandle m_message_handle;
};

}

#endif // _GOBBY_OPERATIONS_OPERATIONNEW_HPP_
