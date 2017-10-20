/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
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

#include "operations/operation-open.hpp"

#include "core/noteplugin.hpp"
#include "util/i18n.hpp"

#include <glibmm/main.h>

#include <libinftextgtk/inf-text-gtk-buffer.h>
#include <gtksourceview/gtksource.h>

#include <cerrno>
#include <cstring> // memmove. Is there some C++ replacement for this?

namespace
{
	// These are the charsets that we try to convert a file from when
	// autodetecting the encoding.
	const char* get_autodetect_encoding(unsigned int index)
	{
		// Translators: This is the 8 bit encoding that is tried when
		// autodetecting a file's encoding.
		static const char* DEFAULT_8BIT_ENCODING = N_("ISO-8859-1");

		static const char* ENCODINGS[] = {
			"UTF-8",
			DEFAULT_8BIT_ENCODING,
			"UTF-16",
			"UCS-2",
			"UCS-4"
		};

		static const unsigned int N_ENCODINGS =
			sizeof(ENCODINGS)/sizeof(ENCODINGS[0]);

		if(index == 1) return Gobby::_(ENCODINGS[index]);
		if(index < N_ENCODINGS) return ENCODINGS[index];
		return NULL;
	}
}

Gobby::OperationOpen::OperationOpen(Operations& operations,
                                    const Preferences& preferences,
                                    InfBrowser* browser,
                                    const InfBrowserIter* parent,
                                    const std::string& name,
                                    const Glib::RefPtr<Gio::File>& file,
                                    const char* encoding):
	Operation(operations), m_preferences(preferences),
	m_name(name), m_file(file), m_parent(browser, parent),
	m_encoding_auto_detect_index(-1),
	m_eol_style(DocumentInfoStorage::EOL_CR), m_request(NULL),
	m_raw_pos(0), m_content(NULL),
	m_message_handle(get_status_bar().invalid_handle())
{
	if(encoding != NULL)
	{
		m_encoding = encoding;
	}
	else
	{
		m_encoding_auto_detect_index = 0;
		m_encoding = get_autodetect_encoding(0);
	}
}

Gobby::OperationOpen::~OperationOpen()
{
	// TODO: Cancel outstanding async operations?

	if(m_request != NULL)
	{
		g_signal_handlers_disconnect_by_func(
			G_OBJECT(m_request),
			(gpointer)G_CALLBACK(on_request_finished_static),
			this);
		g_object_unref(m_request);
	}

	if(m_content != NULL)
		g_object_unref(m_content);

	if(m_message_handle != get_status_bar().invalid_handle())
		get_status_bar().remove_message(m_message_handle);
}

void Gobby::OperationOpen::start()
{
	m_iconv.reset(new Glib::IConv("UTF-8", m_encoding));

	try
	{
		m_file->read_async(sigc::mem_fun(
			*this, &OperationOpen::on_file_read));

		m_message_handle = get_status_bar().add_info_message(
			Glib::ustring::compose(
				_("Opening document \"%1\"..."), m_file->get_uri()));

		m_parent.signal_node_removed().connect(
			sigc::mem_fun(
				*this, &OperationOpen::on_node_removed));

		m_content = GTK_TEXT_BUFFER(gtk_source_buffer_new(NULL));
	}
	catch(const Gio::Error& err)
	{
		error(err.what());
	}
}

void Gobby::OperationOpen::on_node_removed()
{
	error(_("The directory into which the new document "
	        "was supposed to be inserted has been removed"));
}

void Gobby::OperationOpen::on_file_read(
	const Glib::RefPtr<Gio::AsyncResult>& result)
{
	try
	{
		m_stream = m_file->read_finish(result);
		m_buffer.reset(new buffer);

		m_stream->read_async(
			m_buffer->buf, buffer::SIZE,
			sigc::mem_fun(*this, &OperationOpen::on_stream_read));
	}
	catch(const Glib::Exception& ex)
	{
		error(ex.what());
	}
}

void Gobby::OperationOpen::on_stream_read(
	const Glib::RefPtr<Gio::AsyncResult>& result)
{
	try
	{
		gssize size = m_stream->read_finish(result);

		// Close stream after reading, this signals the idle handler
		// that all data has been read from the file.
		if(size <= 0)
		{
			m_stream->close();
			m_stream.reset();
			m_buffer.reset(NULL);

			// If the idle handler is not connected, then we have
			// already processed all the data.
			if(!m_idle_connection.connected())
				read_finish();
		}
		else
		{
			m_raw_content.insert(m_raw_content.end(),
			                     m_buffer->buf,
			                     m_buffer->buf + size);

			// Process read data in an idle handler
			if(!m_idle_connection.connected())
			{
				m_idle_connection =
					Glib::signal_idle().connect(
						sigc::mem_fun(*this,
							&OperationOpen::
								on_idle));
			}

			m_stream->read_async(
				m_buffer->buf, buffer::SIZE,
			        sigc::mem_fun(
					*this,
					&OperationOpen::on_stream_read));
		}
	}
	catch(const Glib::Exception& ex)
	{
		error(ex.what());
	}
}

bool Gobby::OperationOpen::on_idle()
{
	static const unsigned int CONVERT_BUFFER_SIZE = 1024;

	const char* inbuffer = &m_raw_content[m_raw_pos];
	char* inbuf = const_cast<char*>(inbuffer);
	gsize inbytes = m_raw_content.size() - m_raw_pos;
	char outbuffer[CONVERT_BUFFER_SIZE];
	gchar* outbuf = outbuffer;
	gsize outbytes = CONVERT_BUFFER_SIZE;

	/* iconv is defined as libiconv on Windows, or at least when using the
	 * binary packages from ftp.gnome.org. Therefore we can't propely
	 * call Glib::IConv::iconv. Therefore, we use the C API here. */
	const std::size_t result = g_iconv(m_iconv->gobj(),
		&inbuf, &inbytes, &outbuf, &outbytes);
	bool more_to_process = (inbytes != 0);

	if(result == static_cast<std::size_t>(-1))
	{
		if(errno == EILSEQ)
		{
			// Invalid text for the current encoding
			encoding_error();
			return false;
		}

		if(errno == EINVAL)
		{
			// If EINVAL is set, this means that an incomplete
			// multibyte sequence was at the end of the input.
			// We might have some more bytes, but those do not
			// make up a whole character, so we need to wait for
			// more input.
			if(!m_stream)
			{
				// However, if we already read all input, then
				// there is no more input to come. We
				// consider this an error since the file
				// should not end with an incomplete multibyte
				// sequence.
				encoding_error();
				return false;
			}
			else
			{
				// Otherwise, we need to wait for more data
				// to process.
				more_to_process = false;
			}
		}
	}

	m_raw_pos += (inbuf - inbuffer);

	// We now have outbuf - outbuffer bytes of valid UTF-8 in outbuffer.
	char* prev = outbuffer;
	char* pos;
	const char to_find[] = { '\r', '\n', '\0' };

	/* TODO: Write directly into the buffer here,
	 * instead of memmoving stuff. */
	while( (pos = std::find_first_of<char*>(prev, outbuf,
		to_find, to_find + sizeof(to_find))) != outbuf)
	{
		if(*pos == '\0')
		{
			// There is a nullbyte in the conversion. As normal
			// text files don't contain nullbytes, this only
			// occurs when converting for example a UTF-16 from
			// ISO-8859-1 to UTF-8 (note that the UTF-16 file is
			// valid ISO-8859-1, it just contains lots of
			// nullbytes). We therefore produce an error here.
			encoding_error();
			return false;
		}
		else
		{
			// We convert everything to '\n' as line separator,
			// but remember the current eol-style to correctly
			// save the document back to disk.
			prev = pos + 1;
			if(*pos == '\r' && prev != outbuf && *prev == '\n')
			{
				// CRLF style line break
				std::memmove(prev, prev + 1,
				             outbuf - prev - 1);
				m_eol_style = DocumentInfoStorage::EOL_CRLF;
				--outbuf;
			}
			else if(*pos == '\r')
			{
				*pos = '\n';
				m_eol_style = DocumentInfoStorage::EOL_CR;
			}
			else
			{
				m_eol_style = DocumentInfoStorage::EOL_LF;
			}
		}
	}

	GtkTextIter insert_iter;
	gtk_text_buffer_get_end_iter(m_content, &insert_iter);
	gtk_text_buffer_insert(m_content, &insert_iter, outbuffer,
	                       outbuf - outbuffer);

	// Done reading and converting the whole file
	if(!more_to_process && !m_stream)
		read_finish();

	return more_to_process;
}

void Gobby::OperationOpen::encoding_error()
{
	if(m_encoding_auto_detect_index == -1)
	{
		error(_("The file contains data not in the "
		        "specified encoding"));
	}
	else
	{
		++ m_encoding_auto_detect_index;
		const char* next_encoding = get_autodetect_encoding(
			m_encoding_auto_detect_index);

		if(next_encoding == NULL)
		{
			error(_("The file either contains data in an unknown "
			        "encoding, or it contains binary data."));
		}
		else
		{
			// Delete current content:
			GtkTextIter start_iter, end_iter;
			gtk_text_buffer_get_start_iter(m_content,
			                               &start_iter);
			gtk_text_buffer_get_end_iter(m_content,
			                             &end_iter);
			gtk_text_buffer_delete(m_content, &start_iter,
			                       &end_iter);
			
			m_raw_pos = 0;

			m_encoding = next_encoding;
			m_iconv.reset(new Glib::IConv("UTf-8", m_encoding));

			// Read again, try with next encoding.
			m_idle_connection = Glib::signal_idle().connect(
				sigc::mem_fun(*this,
				              &OperationOpen::on_idle));
		}
	}
}

void Gobby::OperationOpen::read_finish()
{
	// If the last character is a newline character, remove it.
	GtkTextIter end_iter, test_iter;
	gtk_text_buffer_get_end_iter(m_content, &end_iter);
	test_iter = end_iter;
	if(gtk_text_iter_backward_char(&test_iter))
	{
		if(gtk_text_iter_get_char(&test_iter) == '\n')
		{
			gtk_text_buffer_delete(
				m_content, &test_iter, &end_iter);
		}
	}

	gtk_text_buffer_set_modified(m_content, FALSE);

	GtkTextIter insert_iter;
	GtkTextMark* insert = gtk_text_buffer_get_insert(m_content);
	gtk_text_buffer_get_iter_at_mark(m_content, &insert_iter, insert);

	InfUser* user = INF_USER(g_object_new(
		INF_TEXT_TYPE_USER,
		"id", 1,
		"flags", INF_USER_LOCAL,
		"name", m_preferences.user.name.get().c_str(),
		/* The user is made active when the user
		 * switches to the document. */
		"status", INF_USER_INACTIVE,
		"hue", m_preferences.user.hue.get(),
		"caret-position", gtk_text_iter_get_offset(&insert_iter),
		static_cast<void*>(NULL)));

	InfUserTable* user_table = inf_user_table_new();
	inf_user_table_add_user(user_table, user);
	g_object_unref(user);

	InfTextGtkBuffer* text_gtk_buffer =
		inf_text_gtk_buffer_new(m_content, user_table);
	g_object_unref(user_table);

	ConnectionManager& connection_manager =
		get_browser().get_connection_manager();
	InfCommunicationManager* communication_manager =
		connection_manager.get_communication_manager();

	InfBrowser* browser = m_parent.get_browser();

	InfIo* io;
	g_object_get(G_OBJECT(browser), "io", &io, NULL);

	InfTextSession* session = inf_text_session_new_with_user_table(
		communication_manager, INF_TEXT_BUFFER(text_gtk_buffer), io,
		user_table, INF_SESSION_RUNNING, NULL, NULL);

	g_object_unref(io);
	g_object_unref(text_gtk_buffer);

	InfRequest* request = inf_browser_add_note(
		m_parent.get_browser(), m_parent.get_browser_iter(),
		m_name.c_str(), "InfText", NULL,
		INF_SESSION(session), TRUE,
		on_request_finished_static, this);
	g_object_unref(session);

	if(request != NULL)
	{
		m_request = request;
		g_object_ref(m_request);

		// TODO: We can remove the node watch here, but need to have
		// the browser available in on_request_finished then. Maybe
		// just disconnect the signal, or bind it.
	}
}

void Gobby::OperationOpen::on_request_finished(const InfBrowserIter* iter,
                                               const GError* error)
{
	if(error != NULL)
	{
		OperationOpen::error(error->message);
	}
	else
	{
		// Store document info so that we know where we loaded the
		// file from, so we don't have to ask the user where to store
		// it when s/he wants to save it again.
		DocumentInfoStorage::Info info;
		info.uri = m_file->get_uri();
		info.encoding = m_encoding;
		info.eol_style = m_eol_style;

		get_info_storage().set_info(
			m_parent.get_browser(),
			iter,
			info);

		InfSessionProxy* proxy =
			inf_browser_get_session(m_parent.get_browser(), iter);
		g_assert(proxy != NULL);

		get_folder_manager().add_document(
			m_parent.get_browser(), iter, proxy, NULL);

		finish();
	}
}

void Gobby::OperationOpen::error(const Glib::ustring& message)
{
	get_status_bar().add_error_message(
		Glib::ustring::compose(
			_("Failed to open document \"%1\""),
			m_file->get_uri()),
		message);

	fail();
}
