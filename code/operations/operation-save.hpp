/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_OPERATIONS_OPERATION_SAVE_HPP_
#define _GOBBY_OPERATIONS_OPERATION_SAVE_HPP_

#include "operations/operations.hpp"
#include "core/documentinfostorage.hpp"

#include <giomm/file.h>
#include <giomm/outputstream.h>

#include <ctime>

namespace Gobby
{

class OperationSave: public Operations::Operation, public sigc::trackable
{
public:
	OperationSave(Operations& operations, DocWindow& document,
	              Folder& folder, const std::string& uri,
	              const std::string& encoding,
	              DocumentInfoStorage::EolStyle eol_style);

	virtual ~OperationSave();

	// Note these can return NULL in case the document has been removed
	// in the meanwhile.
	DocWindow* get_document() { return m_document; }
	const DocWindow* get_document() const { return m_document; }

	std::time_t get_start_time() const { return m_start_time; }

protected:
	void on_document_removed(DocWindow& document);
	void on_file_replace(const Glib::RefPtr<Gio::AsyncResult>& result);
	void on_stream_write(const Glib::RefPtr<Gio::AsyncResult>& result);

	void attempt_next();
	void write_next();
	void error(const Glib::ustring& message);
protected:
	DocWindow* m_document;
	std::time_t m_start_time;

	typedef std::pair<gchar*, std::size_t> Line;
	std::list<Line> m_lines;
	std::list<Line>::iterator m_current_line;
	std::size_t m_current_line_index;

	std::string m_encoding;
	DocumentInfoStorage::EolStyle m_eol_style;
	std::string m_storage_key;
	Glib::IConv m_iconv;

	static const std::size_t BUFFER_SIZE = 1024;
	char m_buffer[BUFFER_SIZE];
	std::size_t m_buffer_size;
	std::size_t m_buffer_index;

	Glib::RefPtr<Gio::File> m_file;
	Glib::RefPtr<Gio::OutputStream> m_stream;

	StatusBar::MessageHandle m_message_handle;
};

}

#endif // _GOBBY_OPERATIONS_OPERATION_SAVE_HPP_
