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

	virtual void start();

protected:
	void on_file_replace(const Glib::RefPtr<Gio::AsyncResult>& result);
	void on_stream_write(const Glib::RefPtr<Gio::AsyncResult>& result);

	void error(const Glib::ustring& message);

protected:
	const std::string m_title;
	const std::string m_uri;
	const std::string m_xml;
	std::string::size_type m_index;

	Glib::RefPtr<Gio::File> m_file;
	Glib::RefPtr<Gio::OutputStream> m_stream;

	StatusBar::MessageHandle m_message_handle;
};

}

#endif // _GOBBY_OPERATIONS_OPERATION_EXPORT_HTML_HPP_
