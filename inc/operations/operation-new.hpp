/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 - 2008 0x539 dev group
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

#ifndef _GOBBY_OPERATIONS_OPERATIONNEW_HPP_
#define _GOBBY_OPERATIONS_OPERATIONNEW_HPP_

#include "operations/operations.hpp"

namespace Gobby
{

class OperationNew: public Operations::Operation
{
public:
	OperationNew(Operations& operations, InfcBrowser* browser,
	             InfcBrowserIter* parent, const Glib::ustring& name);

	virtual ~OperationNew();

protected:
	static void
	on_request_failed_static(InfcNodeRequest* request,
	                         const GError* error,
	                         gpointer user_data)
	{
		static_cast<OperationNew*>(user_data)->
			on_request_failed(error);
	}

	static void
	on_request_finished_static(InfcNodeRequest* request,
	                           InfcBrowserIter* iter,
	                           gpointer user_data)
	{
		static_cast<OperationNew*>(user_data)->
			on_request_finished(iter);
	}

	void on_request_failed(const GError* error);
	void on_request_finished(InfcBrowserIter* iter);

protected:
	Glib::ustring m_name;
	InfcNodeRequest* m_request;
	gulong m_finished_id;
	gulong m_failed_id;

	StatusBar::MessageHandle m_message_handle;
};

}

#endif // _GOBBY_OPERATIONS_OPERATIONNEW_HPP_
