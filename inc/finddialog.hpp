/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
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

#ifndef _GOBBY_FINDDIALOG_HPP_
#define _GOBBY_FINDDIALOG_HPP_

#include "defaultdialog.hpp"
#include "regex.hpp"

#include <gtkmm.h>

namespace Gobby
{

class Window;

class FindDialog : public Gtk::Window
{
public:
	FindDialog(Gobby::Window& parent);

	void set_search_only(bool search_only);
protected:
	// virtual void on_response(int response_id);
	void on_find();

	bool find_in_line(Glib::ustring line,
	                  std::pair<size_t, size_t>& pos);

	bool search(const Gtk::TextIter& from,
	            const Gtk::TextIter* to,
	            Gtk::TextIter& match_start,
	            Gtk::TextIter& match_end);

	/** Searches for an occurence in the document, not looking at
	 * whole word stuff.
	 */
	bool search_once(const Gtk::TextIter& from,
	                 const Gtk::TextIter* to,
	                 Gtk::TextIter& match_start,
	                 Gtk::TextIter& match_end);

	void update_regex();
	void compile_regex();

	Gobby::Window& m_gobby;

	Gtk::HBox m_box_main;
	Gtk::VBox m_box_left;
	Gtk::VSeparator m_separator;
	Gtk::VBox m_box_btns;

	Gtk::Table m_table_entries;

	Gtk::Label m_label_find;
	Gtk::Label m_label_replace;

	Gtk::Entry m_entry_find;
	Gtk::Entry m_entry_replace;

	Gtk::HBox m_hbox;
	Gtk::VBox m_box_options;
	Gtk::CheckButton m_check_whole_word;
	Gtk::CheckButton m_check_case;
	Gtk::CheckButton m_check_regex;

	Gtk::Frame m_frame_direction;
	Gtk::VBox m_box_direction;
	Gtk::RadioButtonGroup m_group_direction;
	Gtk::RadioButton m_radio_up;
	Gtk::RadioButton m_radio_down;

	Gtk::Button m_btn_find;
	Gtk::Button m_btn_replace;
	Gtk::Button m_btn_replace_all;
	Gtk::Button m_btn_close;

	regex m_regex;
	bool m_regex_changed;
};

}

#endif // _GOBBY_JOINDIALOG_HPP_
