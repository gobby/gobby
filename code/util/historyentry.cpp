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

#include "util/historyentry.hpp"

#include <giomm/file.h>
#include <giomm/asyncresult.h>

namespace
{
	Glib::ustring strip(const Glib::ustring& string)
	{
		gchar* dup = g_strdup(string.c_str());
		dup = g_strchug(dup);
		Glib::ustring retval(dup);
		g_free(dup);
		return retval;
	}

	bool strequal(const Glib::ustring& string1,
	              const Glib::ustring& string2)
	{
		gchar* casefold1 = g_utf8_casefold(string1.c_str(), -1);
		gchar* casefold2 = g_utf8_casefold(string2.c_str(), -1);
		int collate = g_utf8_collate(casefold1, casefold2);
		g_free(casefold1);
		g_free(casefold2);
		return collate == 0;
	}
}

// Load history asynchronously, to save startup time
class Gobby::History::Loader
{
public:
	Loader(History& history);

private:
	void close();

	void add(const std::string& str);
	void process(unsigned int size);

	void on_read(const Glib::RefPtr<Gio::AsyncResult>& result);
	void on_stream_read(const Glib::RefPtr<Gio::AsyncResult>& result);

	History& m_history;

	Glib::RefPtr<Gio::File> m_file;
	Glib::RefPtr<Gio::InputStream> m_stream;
	std::string m_line;

	static const unsigned int BUFFER_SIZE = 1024;
	char m_buffer[BUFFER_SIZE];
};

Gobby::History::Loader::Loader(History& history):
	m_history(history)
{
	m_file = Gio::File::create_for_path(m_history.m_history_file);
	m_file->read_async(sigc::mem_fun(*this, &Loader::on_read));
}

void Gobby::History::Loader::close()
{
	m_history.m_loader.reset(NULL);
}

void Gobby::History::Loader::add(const std::string& str)
{
	if(!str.empty())
	{
		Gtk::TreeIter iter = m_history.m_history->append();
		(*iter)[m_history.m_history_columns.text] = str;
	}
}

void Gobby::History::Loader::process(unsigned int size)
{
	const char* pos = m_buffer;
	const char* end = m_buffer + size;
	const char* next;

	const Gtk::TreeNodeChildren& children =
		m_history.m_history->children();
	while((next = std::find(pos, end, '\n')) != end)
	{
		if(!m_line.empty())
		{
			add(m_line + std::string(pos, next - pos));
			m_line.clear();
		}
		else
		{
			add(std::string(pos, next - pos));
		}

		if(children.size() == m_history.m_length)
			break;
	
		pos = next + 1;
	}

	if(children.size() == m_history.m_length)
	{
		close();
	}
	else
	{
		m_line.append(pos, end - pos);

		m_stream->read_async(
			m_buffer, BUFFER_SIZE,
			sigc::mem_fun(*this, &Loader::on_stream_read));
	}
}

void Gobby::History::Loader::on_read(
	const Glib::RefPtr<Gio::AsyncResult>& result)
{
	try
	{
		m_stream = m_file->read_finish(result);

		m_stream->read_async(
			m_buffer, BUFFER_SIZE,
			sigc::mem_fun(*this, &Loader::on_stream_read));
	}
	catch(const Glib::Exception& ex)
	{
		close();
	}
}

void Gobby::History::Loader::on_stream_read(
	const Glib::RefPtr<Gio::AsyncResult>& result)
{
	gssize size;

	try
	{
		size = m_stream->read_finish(result);

		// This should have caused an exception
		g_assert(size >= 0);

		if(size == 0)
		{
			// EOF
			if(!m_line.empty()) add(m_line);
			close();
		}
		else
		{
			process(size);
		}
	}
	catch(const Glib::Exception& ex)
	{
		close();
	}

}

Gobby::History::History(const std::string& history_file, unsigned int length):
	m_length(length),
	m_history(Gtk::ListStore::create(m_history_columns)),
	m_current(m_history->children().end()),
	m_history_file(history_file),
	m_loader(new Loader(*this))
{
}

Gobby::History::History(unsigned int length):
	m_length(length),
	m_history(Gtk::ListStore::create(m_history_columns)),
	m_current(m_history->children().end())
{
}

Gobby::History::~History()
{
	try
	{
		if(!m_history_file.empty())
		{
			Glib::RefPtr<Gio::File> file =
				Gio::File::create_for_path(m_history_file);
			Glib::RefPtr<Gio::OutputStream> stream =
				file->replace();

			const Gtk::TreeNodeChildren& children =
				m_history->children();
			for(Gtk::TreeIter iter = children.begin();
			    iter != children.end(); ++ iter)
			{
				const Glib::ustring& str =
					(*iter)[m_history_columns.text];

				gsize bytes_written;
				stream->write_all(str, bytes_written);
				g_assert(bytes_written == str.length());

				stream->write_all("\n", bytes_written);
				g_assert(bytes_written == 1);
			}
		}
	}
	catch(const Glib::Exception& error)
	{
		// Ignore
	}
}

Glib::RefPtr<Gtk::ListStore> Gobby::History::get_store()
{
	return m_history;
}

const Gobby::History::Columns& Gobby::History::get_columns() const
{
	return m_history_columns;
}

bool Gobby::History::up(const Glib::ustring& current, Glib::ustring& entry)
{
	if(m_current == m_history->children().end())
	{
		m_current = m_history->children().begin();

		// No entries in list:
		if(m_current == m_history->children().end())
			return false;

		if(!current.empty())
			commit_noscroll(current);
	}
	else
	{
		++ m_current;
		if(m_current == m_history->children().end())
		{
			-- m_current;
			return false;
		}
	}

	entry = (*m_current)[m_history_columns.text];
	return true;
}

bool Gobby::History::down(const Glib::ustring& current, Glib::ustring& entry)
{
	if(m_current == m_history->children().end())
	{
		if(!current.empty())
		{
			entry.clear();
			commit_noscroll(current);
			return true;
		}
		else
		{
			return false;
		}
	}

	if(m_current == m_history->children().begin())
	{
		m_current = m_history->children().end();
		entry.clear();
		return true;
	}

	-- m_current;
	entry = (*m_current)[m_history_columns.text];
	return true;
}

void Gobby::History::commit(const Glib::ustring& str)
{
	commit_noscroll(str);

	m_current = m_history->children().end();
}

void Gobby::History::commit_noscroll(const Glib::ustring& str)
{
	const Glib::ustring stripped = strip(str);

	// Check if the entry already exists in the list, and if yes,
	// remove it.
	for(Gtk::TreeIter iter = m_history->children().begin();
	    iter != m_history->children().end(); ++iter)
	{
		const Glib::ustring& entry = (*iter)[m_history_columns.text];
		if(strequal(entry, stripped))
		{
			m_history->erase(iter);
			break;
		}
	}

	Gtk::TreeIter iter = m_history->prepend();
	(*iter)[m_history_columns.text] = stripped;

	while(m_history->children().size() > m_length)
	{
		iter = m_history->children().end();
		-- iter;

		m_history->erase(iter);
	}
}

Gobby::HistoryEntry::HistoryEntry(const std::string& history_file,
                                  unsigned int length):
	m_history(history_file, length)
{
}

Gobby::HistoryEntry::HistoryEntry(unsigned int length):
	m_history(length)
{
}

void Gobby::HistoryEntry::commit()
{
	m_history.commit(get_text());
}

bool Gobby::HistoryEntry::on_key_press_event(GdkEventKey* event)
{
	Glib::ustring entry;

	if(event->keyval == GDK_KEY_Up)
	{
		if(m_history.up(get_text(), entry))
			set_text(entry);

		return true;
	}

	if(event->keyval == GDK_KEY_Down)
	{
		if(m_history.down(get_text(), entry))
			set_text(entry);

		return true;
	}

	return Gtk::Entry::on_key_press_event(event);
}

Gobby::HistoryComboBox::HistoryComboBox(
	const Glib::RefPtr<Gtk::Builder>& builder,
	const char* id,
	const std::string& history_file,
	unsigned int length)
:
	Gtk::ComboBox(GTK_COMBO_BOX(
		gtk_builder_get_object(builder->gobj(), id))),
	m_history(history_file, length)
{
	set_model(m_history.get_store());
	set_entry_text_column(m_history.get_columns().text);

	get_entry()->signal_key_press_event().connect(
		sigc::mem_fun(
			*this, &HistoryComboBox::on_entry_key_press_event),
		false);
}

Gobby::HistoryComboBox::HistoryComboBox(const std::string& history_file,
                                        unsigned int length):
	Gtk::ComboBox(true), m_history(history_file, length)
{
	set_model(m_history.get_store());
	set_entry_text_column(m_history.get_columns().text);

	get_entry()->signal_key_press_event().connect(
		sigc::mem_fun(
			*this, &HistoryComboBox::on_entry_key_press_event),
		false);
}

Gobby::HistoryComboBox::HistoryComboBox(unsigned int length):
	Gtk::ComboBox(true), m_history(length)
{
	set_model(m_history.get_store());
	set_entry_text_column(m_history.get_columns().text);

	get_entry()->signal_key_press_event().connect(
		sigc::mem_fun(
			*this, &HistoryComboBox::on_entry_key_press_event),
		false);
}

void Gobby::HistoryComboBox::commit()
{
	m_history.commit(get_entry()->get_text());
}

Glib::RefPtr<Atk::Object> Gobby::HistoryComboBox::get_accessible()
{
	return get_entry()->get_accessible();
}

bool Gobby::HistoryComboBox::on_entry_key_press_event(GdkEventKey* event)
{
	Glib::ustring entry;

	if(event->keyval == GDK_KEY_Up)
	{
		if(m_history.up(get_entry()->get_text(), entry))
			get_entry()->set_text(entry);

		return true;
	}

	if(event->keyval == GDK_KEY_Down)
	{
		if(m_history.down(get_entry()->get_text(), entry))
			get_entry()->set_text(entry);

		return true;
	}

	return false;
}
