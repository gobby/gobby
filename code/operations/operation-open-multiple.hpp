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
	typedef Operations::uri_list uri_list;

	OperationOpenMultiple(Operations& operations,
	                      const Preferences& preferences,
	                      InfBrowser* browser,
	                      const InfBrowserIter* parent,
	                      const uri_list& uris);

	virtual void start();

protected:
	struct Info
	{
		Glib::ustring uri;
		std::string name;
		const char* encoding;
	};

	typedef std::list<Info> info_list;

	void query(const info_list::iterator& info);

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
