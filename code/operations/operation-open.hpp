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

#ifndef _GOBBY_OPERATIONS_OPERATION_OPEN_HPP_
#define _GOBBY_OPERATIONS_OPERATION_OPEN_HPP_

#include "operations/operations.hpp"
#include "core/documentinfostorage.hpp"
#include "core/nodewatch.hpp"

#include <giomm/file.h>
#include <giomm/inputstream.h>
#include <glibmm/convert.h>

#include <libinfinity/common/inf-request-result.h>

namespace Gobby
{

class OperationOpen: public Operations::Operation, public sigc::trackable
{
public:
	OperationOpen(Operations& operations, const Preferences& preferences,
	              InfBrowser* browser, const InfBrowserIter* parent,
	              const std::string& name, const std::string& uri,
	              const char* encoding /* NULL means auto-detect */);

	virtual ~OperationOpen();

	virtual void start();

protected:
	static void
	on_request_finished_static(InfRequest* request,
	                           const InfRequestResult* result,
	                           const GError* error,
	                           gpointer user_data)
	{
		const InfBrowserIter* iter;
		inf_request_result_get_add_node(result, NULL, NULL, &iter);

		static_cast<OperationOpen*>(user_data)->
			on_request_finished(iter, error);
	}

	void on_node_removed();

	void on_file_read(const Glib::RefPtr<Gio::AsyncResult>& result);
	void on_stream_read(const Glib::RefPtr<Gio::AsyncResult>& result);
	bool on_idle();

	void encoding_error();
	void read_finish();

	void on_request_finished(const InfBrowserIter* iter,
	                         const GError* error);

	void error(const Glib::ustring& message);
protected:
	const Preferences& m_preferences;
	const std::string m_name;
	const std::string m_uri;
	NodeWatch m_parent;

	int m_encoding_auto_detect_index;
	std::auto_ptr<Glib::IConv> m_iconv;
	std::string m_encoding;
	DocumentInfoStorage::EolStyle m_eol_style;

	struct buffer
	{
		static const unsigned int SIZE = 1024;
		char buf[SIZE];
	};

	Glib::RefPtr<Gio::File> m_file;
	Glib::RefPtr<Gio::InputStream> m_stream;
	std::auto_ptr<buffer> m_buffer;
	sigc::connection m_idle_connection;

	InfRequest* m_request;

	std::vector<char> m_raw_content;
	std::vector<char>::size_type m_raw_pos;
	GtkTextBuffer* m_content;

	StatusBar::MessageHandle m_message_handle;
};

}

#endif // _GOBBY_OPERATIONS_OPERATION_OPEN_HPP_
