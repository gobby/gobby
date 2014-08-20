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

#ifndef _GOBBY_OPERATIONS_OPERATIONSUBSCRIBEPATH_HPP_
#define _GOBBY_OPERATIONS_OPERATIONSUBSCRIBEPATH_HPP_

#include "operations/operations.hpp"
#include "core/browser.hpp"

#include <libinfinity/common/inf-request-result.h>

namespace Gobby
{

class OperationSubscribePath: public Operations::Operation
{
public:
	OperationSubscribePath(Operations& operations,
	                       const std::string& uri);
	OperationSubscribePath(Operations& operations,
	                       InfBrowser* browser,
	                       const std::string& path);

	virtual ~OperationSubscribePath();

	virtual void start();

private:
	void start_without_browser();
	void start_with_browser();

	void explore();
	void descend();

	void make_explore_request();
	void make_subscribe_request();

	static void on_notify_status_static(GObject* object,
	                                    GParamSpec* pspec,
	                                    gpointer user_data)
	{
		static_cast<OperationSubscribePath*>(user_data)->
			on_notify_status();
	}

	static void on_browser_deleted_static(gpointer user_data,
	                                      GObject* where_the_object_was)
	{
		static_cast<OperationSubscribePath*>(user_data)->
			on_browser_deleted();
	}

	static void on_explore_finished_static(InfRequest* request,
	                                       const InfRequestResult* result,
	                                       const GError* error,
	                                       gpointer user_data)
	{
		static_cast<OperationSubscribePath*>(user_data)->
			on_explore_finished(error);
	}

	static void on_subscribe_finished_static(InfRequest* request,
	                                         const InfRequestResult* res,
	                                         const GError* error,
	                                         gpointer user_data)
	{
		const InfBrowserIter* iter;

		if(error == NULL)
		{
			inf_request_result_get_subscribe_session(
				res, NULL, &iter, NULL);
		}

		static_cast<OperationSubscribePath*>(user_data)->
			on_subscribe_finished(iter, error);
	}

	void on_notify_status();
	void on_browser_deleted();
	void on_explore_finished(const GError* error);
	void on_subscribe_finished(const InfBrowserIter* iter,
	                           const GError* error);

	InfBrowser* m_browser;
	std::string m_target;

	std::vector<std::string> m_path;
	std::vector<std::string>::size_type m_path_index;
	InfBrowserIter m_path_iter;

	InfRequest* m_request;
	gulong m_notify_status_id;

	StatusBar::MessageHandle m_message_handle;
};

}

#endif // _GOBBY_OPERATIONS_OPERATIONSUBSCRIBEPATH_HPP_
