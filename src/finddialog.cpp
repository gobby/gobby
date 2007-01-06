#include <cstring>
#include <cstdio>
#include <regex.h>

#include <gtkmm/messagedialog.h>
#include <gtksourceview/gtksourceiter.h>

#include "common.hpp"
#include "document.hpp"
#include "window.hpp"
#include "finddialog.hpp"

namespace
{
	typedef gboolean (*gtk_source_iter_search_func)(
		const GtkTextIter*,
		const gchar*,
        	GtkSourceSearchFlags,
		GtkTextIter*,
                GtkTextIter*,
		const GtkTextIter*
	);
}

namespace Gobby { // TODO: Remove this

FindDialog::FindDialog(Gobby::Window& parent):
	m_gobby(parent),
	m_label_find(_("Find what:"), Gtk::ALIGN_LEFT),
	m_label_replace(_("Replace with:"), Gtk::ALIGN_LEFT),
	m_check_whole_word(_("Match whole word only")),
	m_check_case(_("Match case")),
	m_check_regex(_("Match as regular expression")),
	m_frame_direction(_("Direction")),
	m_radio_up(m_group_direction, _("Up")),
	m_radio_down(m_group_direction, _("Down")),
	m_btn_find(Gtk::Stock::FIND),
	m_btn_replace(Gtk::Stock::FIND_AND_REPLACE),
	m_btn_replace_all(_("Replace _all") ),
	m_btn_close(Gtk::Stock::CLOSE),
	m_regex("")
{
	m_box_main.set_spacing(12);
	m_box_main.pack_start(m_box_left);
	m_box_main.pack_start(m_separator, Gtk::PACK_SHRINK);
	m_box_main.pack_start(m_box_btns, Gtk::PACK_SHRINK);
	add(m_box_main);

	m_box_left.pack_start(m_table_entries);
	m_box_left.pack_start(m_hbox);

	m_table_entries.set_spacings(5);
	m_table_entries.attach(m_label_find, 0, 1, 0, 1,
		Gtk::SHRINK | Gtk::FILL, Gtk::EXPAND);
	m_table_entries.attach(m_label_replace, 0, 1, 1, 2,
		Gtk::SHRINK | Gtk::FILL, Gtk::EXPAND);
	m_table_entries.attach(m_entry_find, 1, 2, 0, 1,
		Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND);
	m_table_entries.attach(m_entry_replace, 1, 2, 1, 2,
		Gtk::EXPAND | Gtk::FILL, Gtk::EXPAND);

	m_hbox.pack_start(m_box_options);
	m_hbox.pack_start(m_frame_direction, Gtk::PACK_SHRINK);

	m_box_options.pack_start(m_check_whole_word, Gtk::PACK_EXPAND_WIDGET);
	m_box_options.pack_start(m_check_case, Gtk::PACK_EXPAND_WIDGET);
	m_box_options.pack_start(m_check_regex, Gtk::PACK_EXPAND_WIDGET);

	m_frame_direction.add(m_box_direction);
	m_box_direction.set_border_width(4);
	m_box_direction.pack_start(m_radio_up, Gtk::PACK_EXPAND_WIDGET);
	m_box_direction.pack_start(m_radio_down, Gtk::PACK_EXPAND_WIDGET);

	m_box_btns.set_spacing(5);
	m_box_btns.pack_start(m_btn_find, Gtk::PACK_EXPAND_PADDING);
	m_box_btns.pack_start(m_btn_replace, Gtk::PACK_EXPAND_PADDING);
	m_box_btns.pack_start(m_btn_replace_all, Gtk::PACK_EXPAND_PADDING);
	m_box_btns.pack_start(m_btn_close, Gtk::PACK_EXPAND_PADDING);

	m_entry_find.signal_changed().connect(
		sigc::mem_fun(*this, &FindDialog::update_regex));
	m_check_case.signal_toggled().connect(
		sigc::mem_fun(*this, &FindDialog::update_regex));
	m_check_regex.signal_toggled().connect(
		sigc::mem_fun(*this, &FindDialog::update_regex));

	m_entry_find.signal_activate().connect(
		sigc::mem_fun(*this, &FindDialog::on_find));

	m_radio_down.set_active(true);

	m_btn_close.signal_clicked().connect(
		sigc::mem_fun(*this, &FindDialog::hide));

	m_btn_find.signal_clicked().connect(
		sigc::mem_fun(*this, &FindDialog::on_find));

	set_title(_("Search and replace"));
	set_border_width(16);

	set_skip_pager_hint(true);
	set_skip_taskbar_hint(true);

	set_resizable(false);
	set_transient_for(parent);
	show_all_children();
}

void FindDialog::set_search_only(bool search_only)
{
	void(Gtk::Widget::*show_func)();
	show_func = search_only ? &Gtk::Widget::hide : &Gtk::Widget::show;

	sigc::bind(show_func, sigc::ref(m_entry_replace) )();
	sigc::bind(show_func, sigc::ref(m_label_replace) )();
	sigc::bind(show_func, sigc::ref(m_btn_replace) )();
	sigc::bind(show_func, sigc::ref(m_btn_replace_all) )();
}

void FindDialog::on_find()
{
	Gobby::Document* doc = m_gobby.get_current_document();
	if (doc == NULL)
	{
		Gtk::MessageDialog dlg(
			*this,
			_("No document currently opened"),
			false,
			Gtk::MESSAGE_ERROR,
			Gtk::BUTTONS_OK,
			true
		);

		dlg.run();
		return;
	}

	Glib::ustring find_str = m_entry_find.get_text();
	Glib::RefPtr<Gtk::TextBuffer> buf = doc->get_buffer();
	Gtk::TextIter start_pos(buf->get_insert()->get_iter());

	Gtk::TextIter match_start, match_end;
	bool result;

	// TODO: Ein try fuer alles?
	try
	{
		result = search(start_pos, NULL, match_start, match_end);
	}
	catch (regex::compile_error& e)
	{
		Gtk::MessageDialog dlg(
			*this,
			e.what(),
			false,
			Gtk::MESSAGE_ERROR,
			Gtk::BUTTONS_OK,
			true
		);

		dlg.run();
		return;
	}

	if (result)
	{
		if(m_radio_down.get_active() )
			doc->set_selection(match_end, match_start);
		else
			doc->set_selection(match_start, match_end);

		return;
	}
	else
	{
		Gtk::TextIter restart_pos;
		if (m_radio_down.get_active())
			restart_pos = buf->begin();
		else
			restart_pos = buf->end();

		try
		{
			// Limit to search to: Normally the position where we
			// started.
			Gtk::TextIter* relimit = &start_pos;
			if(m_radio_down.get_active() )
			{
				start_pos.forward_chars(find_str.length() );
				if(start_pos == buf->end() )
					relimit = NULL;
			}

			result = search(
				restart_pos,
				relimit,
				match_start,
				match_end
			);
		}
		catch (regex::compile_error& e)
		{
			// Should not happen
			Gtk::MessageDialog dlg(
				*this,
				e.what(),
				false,
				Gtk::MESSAGE_ERROR,
				Gtk::BUTTONS_OK,
				true
			);

			dlg.run();
			return;
		}

		if (result)
		{
			if(m_radio_down.get_active() )
				doc->set_selection(match_end, match_start);
			else
				doc->set_selection(match_start, match_end);

			return;
		}
	}

	Gtk::MessageDialog dlg(
		*this,
		_("No match found"),
		false,
		Gtk::MESSAGE_INFO,
		Gtk::BUTTONS_OK,
		true
	);

	dlg.run();
}

bool FindDialog::search(const Gtk::TextIter& from,
                        const Gtk::TextIter* to,
                        Gtk::TextIter& match_start,
                        Gtk::TextIter& match_end)
{
	Gtk::TextIter start_pos(from);
	while(search_once(start_pos, to, match_start, match_end) )
	{
		if(m_check_whole_word.get_active() )
		{
			if(!match_start.starts_word() || !match_end.ends_word())
			{
				if(m_radio_down.get_active() )
					start_pos = match_end;
				else
					start_pos = match_start;

				continue;
			}
		}

		return true;
	}

	return false;
}

bool FindDialog::search_once(const Gtk::TextIter& from,
                             const Gtk::TextIter* to,
                             Gtk::TextIter& match_start,
                             Gtk::TextIter& match_end)
{
	if(m_check_regex.get_active() && m_regex_changed)
		compile_regex();

	if(m_check_regex.get_active() )
	{
		Glib::RefPtr<Gtk::TextBuffer> buf = from.get_buffer();

		Gtk::TextIter start_pos, limit;
		if(m_radio_up.get_active() )
		{
			limit = from;

			if(to == NULL)
				start_pos = buf->begin();
			else
				start_pos = *to;
		}
		else if(m_radio_down.get_active() )
		{
			start_pos = from;

			if(to == NULL)
				limit = buf->end();
			else
				limit = *to;
		}

		Gtk::TextIter begin = buf->end(), end = buf->end();
		Gtk::TextIter cur_line = start_pos, next_line = start_pos;
		for(;;)
		{
			next_line.forward_line();

			// Get current line of text
			Glib::ustring line = cur_line.get_slice(next_line);
			std::cout << "Text from " << cur_line.get_offset() << " to " << next_line.get_offset() << ": " << line << std::endl;

			// Trim trailing text after limit
			if(limit.get_line() == cur_line.get_line() )
			{
				if(!limit.ends_line() )
				{
					line.erase(
						limit.get_line_offset() -
						cur_line.get_line_offset()
					);
				}
			}

			regex::match_options options =
				regex::match_options::NONE;

			if(!cur_line.starts_line() )
				options |= regex::match_options::NOT_BOL;

			if(cur_line.get_line() == limit.get_line() &&
			   !limit.ends_line() )
				options |= regex::match_options::NOT_EOL;

			std::pair<std::size_t, std::size_t> pos;
			bool result = m_regex.find(
				line.c_str(),
				pos,
				options
			);

			if(result == true)
			{
				begin = end = cur_line;
				begin.set_line_index(
					begin.get_line_index() + pos.first
				);

				end.set_line_index(
					end.get_line_index() + pos.second
				);

				// Match after limit
				if(end > limit) break;

				// First match is result if searching forward
				if(m_radio_down.get_active() )
				{
					match_start = begin;
					match_end = end;

					return true;
				}
			}

			cur_line = next_line;
			if(cur_line > limit || cur_line == buf->end() )
				break;
		}

		if(m_radio_up.get_active() )
		{
			// No match for backward search
			if(begin == buf->end() && end == buf->end() )
				return false;

			match_start = begin;
			match_end = end;

			return true;
		}

		// No match for forward search
		return false;
	}
	else
	{
		GtkSourceSearchFlags flags = GtkSourceSearchFlags(0);
		if(!m_check_case.get_active() )
			flags = GTK_SOURCE_SEARCH_CASE_INSENSITIVE;

		gtk_source_iter_search_func search_func =
			gtk_source_iter_forward_search;

		if(m_radio_up.get_active() )
			search_func = gtk_source_iter_backward_search;

		Glib::ustring find_str = m_entry_find.get_text();
		GtkTextIter match_start_gtk, match_end_gtk;
		gboolean result = search_func(
			from.gobj(),
			find_str.c_str(),
			flags,
			&match_start_gtk,
			&match_end_gtk,
			to != NULL ? to->gobj() : NULL
		);

		if(result == TRUE)
		{
			match_start = Gtk::TextIter(&match_start_gtk);
			match_end = Gtk::TextIter(&match_end_gtk);

			return true;
		}

		return false;
	}
}

void FindDialog::update_regex()
{
	if (m_check_regex.get_active())
		m_regex_changed = true;
	else
		m_regex_changed = false;
}

void FindDialog::compile_regex()
{
	if (m_check_case.get_active())
	{
		m_regex.reset(
			m_entry_find.get_text().c_str(),
			regex::compile_options::EXTENDED
		);
	}
	else
	{
		m_regex.reset(
			m_entry_find.get_text().c_str(),
			regex::compile_options::EXTENDED |
			regex::compile_options::IGNORE_CASE
		);
	}

	m_regex_changed = false;
}

}

