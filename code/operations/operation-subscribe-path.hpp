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

#ifndef _GOBBY_OPERATIONS_OPERATIONSUBSCRIBEPATH_HPP_
#define _GOBBY_OPERATIONS_OPERATIONSUBSCRIBEPATH_HPP_

#include "operations/operations.hpp"
#include "core/browser.hpp"
#include "util/resolv.hpp"

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

	static void on_explore_finished_static(InfNodeRequest* request,
	                                       const InfBrowserIter* iter,
	                                       const GError* error,
	                                       gpointer user_data)
	{
		static_cast<OperationSubscribePath*>(user_data)->
			on_explore_finished(iter, error);
	}

	static void on_subscribe_finished_static(InfNodeRequest* request,
	                                         const InfBrowserIter* iter,
	                                         const GError* error,
	                                         gpointer user_data)
	{
		static_cast<OperationSubscribePath*>(user_data)->
			on_subscribe_finished(iter, error);
	}

	void on_resolv_done(const ResolvHandle* handle,
	                    const InfIpAddress* address,
	                    guint port,
	                    const std::string& hostname,
	                    unsigned int device_index);
	void on_resolv_error(const ResolvHandle* handle,
	                     const std::runtime_error& error,
	                     const std::string& hostname);

	void on_notify_status();
	void on_explore_finished(const InfBrowserIter* iter,
	                         const GError* error);
	void on_subscribe_finished(const InfBrowserIter* iter,
	                           const GError* error);

	InfBrowser* m_browser;
	std::string m_target;

	std::vector<std::string> m_path;
	std::vector<std::string>::size_type m_path_index;
	InfBrowserIter m_path_iter;

	InfNodeRequest* m_request;
	gulong m_notify_status_id;

	std::auto_ptr<ResolvHandle> m_resolve_handle;
	StatusBar::MessageHandle m_message_handle;
};

}

#endif // _GOBBY_OPERATIONS_OPERATIONSUBSCRIBEPATH_HPP_
