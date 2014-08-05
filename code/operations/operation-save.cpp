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

#include "operations/operation-save.hpp"

#include "util/i18n.hpp"

#include <cassert>
#include <cerrno>

Gobby::OperationSave::OperationSave(Operations& operations,
                                    TextSessionView& view,
                                    const std::string& uri,
                                    const std::string& encoding,
                                    DocumentInfoStorage::EolStyle eol_style):
	Operation(operations), m_uri(uri), m_view(&view),
	m_start_time(std::time(NULL)), m_current_line_index(0),
	m_encoding(encoding), m_eol_style(eol_style),
	m_storage_key(view.get_info_storage_key()),
	m_iconv(encoding.c_str(), "UTF-8"),
	m_buffer_size(0), m_buffer_index(0)
{
	const Folder& folder = get_folder_manager().get_text_folder();
	folder.signal_document_removed().connect(
		sigc::mem_fun(*this, &OperationSave::on_document_removed));

	// Load content so that the session can go on while saving
	GtkTextBuffer* buffer = GTK_TEXT_BUFFER(view.get_text_buffer());
	GtkTextIter prev;
	GtkTextIter pos;
	GtkTextIter old_pos;

	gtk_text_buffer_get_start_iter(buffer, &prev);
	pos = prev;

	if(!gtk_text_iter_ends_line(&pos))
		gtk_text_iter_forward_to_line_end(&pos);

	do
	{
		Line line;
		line.first =
			gtk_text_buffer_get_text(buffer, &prev, &pos, TRUE);
		line.second = gtk_text_iter_get_line_index(&pos);
		m_lines.push_back(line);

		//if(gtk_text_iter_is_end(&prev))
		//	break;

		old_pos = pos;
		gtk_text_iter_forward_line(&prev);
		gtk_text_iter_forward_to_line_end(&pos);
	} while(!gtk_text_iter_equal(&pos, &old_pos));

	m_current_line = m_lines.begin();
}

Gobby::OperationSave::~OperationSave()
{
	// TODO: Cancel outstanding async operations?

	for(std::list<Line>::iterator iter = m_lines.begin();
	    iter != m_lines.end(); ++ iter)
	{
		g_free(iter->first);
	}

	get_status_bar().remove_message(m_message_handle);

	// Reset file explicitely before closing stream so that, on failure,
	// existing files are not overriden with the temporary files we
	// actually wrote to, at least for local files.
	m_file.reset();
}

void Gobby::OperationSave::start()
{
	m_file = Gio::File::create_for_uri(m_uri);
	m_file->replace_async(sigc::mem_fun(*this,
	                                   &OperationSave::on_file_replace));

	m_message_handle = get_status_bar().add_info_message(
		Glib::ustring::compose(
			_("Saving document \"%1\" to \"%2\"..."),
			m_view->get_title(), m_uri));
}

void Gobby::OperationSave::on_document_removed(SessionView& view)
{
	// We keep the document to unset the modified flag when the operation
	// is complete, however, if the document is removed in the meanwhile,
	// then we don't need to care anymore.
	if(m_view == &view)
		m_view = NULL;
}

void Gobby::OperationSave::on_file_replace(
	const Glib::RefPtr<Gio::AsyncResult>& result)
{
	try
	{
		m_stream = m_file->replace_finish(result);
		attempt_next();
	}
	catch(const Glib::Exception& ex)
	{
		error(ex.what());
	}
}

void Gobby::OperationSave::attempt_next()
{
	bool done;

	if(m_current_line == m_lines.end())
	{
		done = true;
	}
	else
	{
		// Always save a newline at the end, except the document
		// is empty.
		assert(!m_lines.empty());
		std::list<Line>::const_iterator iter = m_lines.begin();
		++iter;

		if(iter == m_lines.end() && m_lines.front().first[0] == '\0')
			done = true;
		else
			done = false;
	}

	if(done)
	{
		DocumentInfoStorage::Info info;
		info.uri = m_file->get_uri();
		info.encoding = m_encoding;
		info.eol_style = m_eol_style;
		get_info_storage().set_info(m_storage_key, info);

		m_stream->close();

		if(m_view != NULL)
		{
			// TODO: Don't unset modified flag if the document has
			// changed in the meanwhile, but set
			// buffer-modified-time in algorithm.
			gtk_text_buffer_set_modified(
				GTK_TEXT_BUFFER(m_view->get_text_buffer()),
				FALSE);
		}

		finish();
	}
	else
	{
		write_next();
	}
}

void Gobby::OperationSave::write_next()
{
	gchar* inbuf;
	gsize inlen;
	char newlinebuf[2] = { '\r', '\n' };

	if(m_current_line_index < m_current_line->second)
	{
		inbuf = m_current_line->first + m_current_line_index;
		inlen = m_current_line->second - m_current_line_index;
	}
	else
	{
		// Write newline
		switch(m_eol_style)
		{
		case DocumentInfoStorage::EOL_CR:
			inbuf = newlinebuf + 0;
			inlen = 1;
			break;
		case DocumentInfoStorage::EOL_LF:
			inbuf = newlinebuf + 1;
			inlen = 1;
			break;
		case DocumentInfoStorage::EOL_CRLF:
			inbuf = newlinebuf + 0;
			inlen = 2;
			break;
		default:
			g_assert_not_reached();
			break;
		}
	}

	gchar* outbuf = m_buffer;
	gsize outlen = BUFFER_SIZE;

	gchar* preserve_inbuf = inbuf;

	/* iconv is defined as libiconv on Windows, or at least when using the
	 * binary packages from ftp.gnome.org. Therefore we can't properly
	 * call Glib::IConv::iconv. Therefore, we use the C API here. */
	std::size_t retval = g_iconv(
		m_iconv.gobj(), &inbuf, &inlen, &outbuf, &outlen);

	if(retval == static_cast<std::size_t>(-1))
	{
		g_assert(errno != EILSEQ);
		// E2BIG and EINVAL are fully OK here.
	}
	else if(retval > 0)
	{
		error(_("The document contains one or more characters that "
		        "cannot be encoded in the specified character "
		        "coding."));
		return;
	}

	// Advance bytes read.
	m_current_line_index += inbuf - preserve_inbuf;
	m_buffer_size = BUFFER_SIZE - outlen;
	m_buffer_index = 0;

	g_assert(m_buffer_size > 0);

	if(m_current_line_index > m_current_line->second)
	{
		// Converted whole line:
		g_free(m_current_line->first);
		m_current_line = m_lines.erase(m_current_line);
		m_current_line_index = 0;
	}

	m_stream->write_async(m_buffer, m_buffer_size,
	                      sigc::mem_fun(*this,
			                    &OperationSave::on_stream_write));
}

void Gobby::OperationSave::on_stream_write(
	const Glib::RefPtr<Gio::AsyncResult>& result)
{
	try
	{
		gssize size = m_stream->write_finish(result);
		// On size < 0 an exception should have been thrown.
		g_assert(size >= 0);

		m_buffer_index += size;
		if(m_buffer_index < m_buffer_size)
		{
			// Write next chunk
			m_stream->write_async(
				m_buffer + m_buffer_index,
				m_buffer_size - m_buffer_index,
				sigc::mem_fun(
					*this,
					&OperationSave::on_stream_write));
		}
		else
		{
			// Go on with next part of line and/or next line
			attempt_next();
		}
	}
	catch(const Glib::Exception& ex)
	{
		error(ex.what());
	}
}

void Gobby::OperationSave::error(const Glib::ustring& message)
{
	get_status_bar().add_error_message(
		Glib::ustring::compose(_("Failed to save document \"%1\""),
		                         m_file->get_uri()),
		message);

	fail();
}
