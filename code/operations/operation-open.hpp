/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2011 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_OPERATIONS_OPERATION_OPEN_HPP_
#define _GOBBY_OPERATIONS_OPERATION_OPEN_HPP_

#include "operations/operations.hpp"
#include "core/documentinfostorage.hpp"
#include "core/nodewatch.hpp"

#include <giomm/file.h>
#include <giomm/inputstream.h>

namespace Gobby
{

class OperationOpen: public Operations::Operation, public sigc::trackable
{
public:
	OperationOpen(Operations& operations, const Preferences& preferences,
	              InfcBrowser* browser, const InfcBrowserIter* parent,
	              const std::string& name, const std::string& uri,
		      const char* encoding /* NULL means auto-detect */);

	virtual ~OperationOpen();

protected:
	static void
	on_request_failed_static(InfcNodeRequest* request,
	                         const GError* error,
	                         gpointer user_data)
	{
		static_cast<OperationOpen*>(user_data)->
			on_request_failed(error);
	}

	static void
	on_request_finished_static(InfcNodeRequest* request,
	                           InfcBrowserIter* iter,
	                           gpointer user_data)
	{
		static_cast<OperationOpen*>(user_data)->
			on_request_finished(iter);
	}

	void on_node_removed();

	void on_file_read(const Glib::RefPtr<Gio::AsyncResult>& result);
	void on_stream_read(const Glib::RefPtr<Gio::AsyncResult>& result);
	bool on_idle();

	void encoding_error();
	void read_finish();

	void on_request_failed(const GError* error);
	void on_request_finished(InfcBrowserIter* iter);

	void error(const Glib::ustring& message);
protected:
	const Preferences& m_preferences;
	std::string m_name;
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

	InfcNodeRequest* m_request;
	gulong m_finished_id;
	gulong m_failed_id;

	std::vector<char> m_raw_content;
	std::vector<char>::size_type m_raw_pos;
	GtkTextBuffer* m_content;

	StatusBar::MessageHandle m_message_handle;
};

}

#endif // _GOBBY_OPERATIONS_OPERATION_OPEN_HPP_
