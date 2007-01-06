#include <cstring>
#include <cstdio>
#include <regex.h>

#include <gtkmm/messagedialog.h>
#include <gtksourceview/gtksourceiter.h>

#include "common.hpp"
#include "document.hpp"
#include "window.hpp"
#include "finddialog.hpp"


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

	// Begin at cursor's position
	Gtk::TextIter start_pos(doc->get_buffer()->get_insert()->get_iter());

	Gtk::TextIter match_start, match_end;
	bool result;

	// TODO: Ein try fuer alles?
	try
	{
		result = search(start_pos, match_start, match_end, NULL);
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
			restart_pos = doc->get_buffer()->begin();
		else
			restart_pos = doc->get_buffer()->end();

		try
		{
			result = search(
				restart_pos,
				match_start,
				match_end,
				&start_pos
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

bool FindDialog::search(Gtk::TextIter start_pos,
                        Gtk::TextIter& match_start,
                        Gtk::TextIter& match_end,
                        const Gtk::TextIter* limit)
{
	if (m_check_regex.get_active())
	{
		if (m_regex_changed)
			compile_regex();

		Glib::RefPtr<Gtk::TextBuffer> buf = start_pos.get_buffer();
		Gtk::TextIter my_limit(limit ? *limit : buf->end());

		// lots of hack: regexes do not support reverse searching, so
		//   we have to reverse the two-step search process manually
		if (m_radio_up.get_active())
		{
			if (limit)
			{
				start_pos = my_limit;
				my_limit = buf->end();
			}
			else
			{
				my_limit = start_pos;
				start_pos = buf->begin();
			}
		}

		Gtk::TextIter last_match_start = buf->end();
		Gtk::TextIter last_match_end;
		for (;;)
		{
			if (limit && *limit >= start_pos)
				break;
			if (!limit && start_pos == buf->end())
				break;

			Gtk::TextIter next_line = start_pos;
			next_line.forward_line();
			Glib::ustring line =
				start_pos.get_slice(next_line);

			// GETH NICHT bei backward suche?
			if (limit &&
			    limit->get_line() == start_pos.get_line())
				line.replace(limit->get_line_offset(),
				             1, '\0');

			std::pair<size_t, size_t> p;
			if (m_regex.find(line.c_str(), p,
			    start_pos.starts_line()
			    ? regex::match_options::NONE
			    : regex::match_options::NOT_BOL)
			    | (limit && limit->ends_line()
			       ? regex::match_options::NONE
			       : regex::match_options::NOT_EOL))
			{
				match_start = match_end = start_pos;
				match_start.set_line_index(p.first);
				match_end.set_line_index(p.second);
				if (m_radio_down.get_active())
				{
					return true;
				}
				else
				{
					last_match_start = match_start;
					last_match_end = match_end;
				}
			}

			start_pos = next_line;
		}

		if (last_match_start != buf->end())
		{
			match_start = last_match_start;
			match_end   = last_match_end;
			return true;
		}
	}
	else
	{
		GtkSourceSearchFlags flags = GTK_SOURCE_SEARCH_CASE_INSENSITIVE;
		if(m_check_case.get_active() )
			flags = GtkSourceSearchFlags(0);

		GtkTextIter match_start_gtk, match_end_gtk;
		gboolean (*search_func)(const GtkTextIter*, const gchar*,
		                        GtkSourceSearchFlags, GtkTextIter*,
		                        GtkTextIter*, const GtkTextIter*);

		search_func = gtk_source_iter_forward_search;
		if(m_radio_up.get_active() )
			search_func = gtk_source_iter_backward_search;

		const Glib::ustring& find_str = m_entry_find.get_text();
		while (search_func(start_pos.gobj(), find_str.c_str(), flags,
		                   &match_start_gtk, &match_end_gtk,
		                   limit ? limit->gobj() : 0))
		{
			match_start = Gtk::TextIter(&match_start_gtk);
			match_end   = Gtk::TextIter(&match_end_gtk);

			if (m_check_whole_word.get_active())
			{
				if (!match_start.starts_word() ||
				    !match_end.ends_word())
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
	}

	return false;
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

