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

#ifndef _GOBBY_OPERATIONS_OPERATION_SAVE_HPP_
#define _GOBBY_OPERATIONS_OPERATION_SAVE_HPP_

#include "operations/operations.hpp"
#include "core/documentinfostorage.hpp"

#include <giomm/file.h>
#include <giomm/outputstream.h>
#include <glibmm/convert.h>

#include <ctime>

namespace Gobby
{

class OperationSave: public Operations::Operation, public sigc::trackable
{
public:
	// TODO: This should maybe just take a text buffer to save, not a
	// textsessionview.
	OperationSave(Operations& operations, TextSessionView& view,
	              const Glib::RefPtr<Gio::File>& file,
	              const std::string& encoding,
	              DocumentInfoStorage::EolStyle eol_style);

	virtual ~OperationSave();

	virtual void start();

	// Note these can return NULL in case the view has been closed
	// in the meanwhile.
	TextSessionView* get_view() { return m_view; }
	const TextSessionView* get_view() const { return m_view; }

	std::time_t get_start_time() const { return m_start_time; }

protected:
	void on_document_removed(SessionView& view);
	void on_file_replace(const Glib::RefPtr<Gio::AsyncResult>& result);
	void on_stream_write(const Glib::RefPtr<Gio::AsyncResult>& result);

	void attempt_next();
	void write_next();
	void error(const Glib::ustring& message);
protected:
	const Glib::RefPtr<Gio::File> m_file;
	TextSessionView* m_view;
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

	Glib::RefPtr<Gio::OutputStream> m_stream;

	StatusBar::MessageHandle m_message_handle;
};

}

#endif // _GOBBY_OPERATIONS_OPERATION_SAVE_HPP_
