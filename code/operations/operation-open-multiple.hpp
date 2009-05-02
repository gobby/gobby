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

#ifndef _GOBBY_OPERATIONS_OPERATION_OPEN_MULTIPLE_HPP_
#define _GOBBY_OPERATIONS_OPERATION_OPEN_MULTIPLE_HPP_

#include "operations/operations.hpp"
#include "core/nodewatch.hpp"

#include <giomm/file.h>

#include <list>

namespace Gobby
{

class OperationOpenMultiple:
	public Operations::Operation, public sigc::trackable
{
public:
	OperationOpenMultiple(Operations& operations,
	                      const Preferences& preferences,
	                      InfcBrowser* browser,
	                      const InfcBrowserIter* parent,
	                      unsigned int num_uris);

	void add_uri(const Glib::ustring& uri,
	             const char* name /* Autodetected if NULL */,
	             const char* encoding /* Autodetected if NULL */);

protected:
	struct Info
	{
		Glib::ustring uri;
		std::string name;
		const char* encoding;
	};

	typedef std::list<Info> info_list;

	void on_node_removed();
	void on_query_info(const Glib::RefPtr<Gio::AsyncResult>& result,
	                   const Glib::RefPtr<Gio::File>& file,
	                   const info_list::iterator& info);
	void on_finished(bool success, const info_list::iterator& info);

	void load_info(const info_list::iterator& iter);
	void single_error(const info_list::iterator& iter,
	                  const Glib::ustring& message);
	void fatal_error(const Glib::ustring& message);

	const Preferences& m_preferences;
	NodeWatch m_parent;
	unsigned int m_num_uris;

	info_list m_infos;
	OperationOpen* m_current;
};

}

#endif // _GOBBY_OPERATIONS_OPERATION_OPEN_MULTIPLE_HPP_
