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

#ifndef _GOBBY_OPERATIONS_OPERATION_EXPORT_HTML_HPP_
#define _GOBBY_OPERATIONS_OPERATION_EXPORT_HTML_HPP_

#include "operations/operations.hpp"
#include "core/documentinfostorage.hpp"

#include <giomm/file.h>
#include <giomm/outputstream.h>

#include <ctime>

namespace Gobby
{

class OperationExportHtml: public Operations::Operation, public sigc::trackable
{
public:
	OperationExportHtml(Operations& operations, TextSessionView& view,
	                    const std::string& uri);

	virtual ~OperationExportHtml();

protected:
	void on_file_replace(const Glib::RefPtr<Gio::AsyncResult>& result);
	void on_stream_write(const Glib::RefPtr<Gio::AsyncResult>& result);

	void error(const Glib::ustring& message);

protected:
	const std::string m_xml;
	std::string::size_type m_index;

	Glib::RefPtr<Gio::File> m_file;
	Glib::RefPtr<Gio::OutputStream> m_stream;

	StatusBar::MessageHandle m_message_handle;
};

}

#endif // _GOBBY_OPERATIONS_OPERATION_EXPORT_HTML_HPP_
