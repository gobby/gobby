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

#ifndef _GOBBY_OPERATIONS_OPERATIONRENAME_HPP_
#define _GOBBY_OPERATIONS_OPERATIONRENAME_HPP_

#include "operations/operations.hpp"

namespace Gobby
{

class OperationRename: public Operations::Operation
{
public:
	OperationRename(Operations& operations, InfBrowser* browser,
	                const InfBrowserIter* iter, const Glib::ustring& new_name);

	virtual ~OperationRename();

	virtual void start();

protected:
	static void
	on_request_finished_static(InfRequest* request,
	                           const InfRequestResult* result,
	                           const GError* error,
	                           gpointer user_data)
	{
		static_cast<OperationRename*>(user_data)->
			on_request_finished(error);
	}

	void on_request_finished(const GError* error);

protected:
	InfBrowser* m_browser;
	const InfBrowserIter m_iter;
	const Glib::ustring m_name;
	const Glib::ustring m_new_name;

	InfRequest* m_request;
	StatusBar::MessageHandle m_message_handle;
};

}

#endif // _GOBBY_OPERATIONS_OPERATIONRENAME_HPP_
