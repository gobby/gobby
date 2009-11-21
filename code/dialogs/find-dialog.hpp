/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008, 2009 Armin Burgmeier <armin@arbur.net>
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

#include "core/folder.hpp"
#include "core/statusbar.hpp"
#include "core/sessionview.hpp"

#include <gtkmm/dialog.h>
#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/checkbutton.h>

namespace Gobby
{

class FindDialog: public Gtk::Dialog
{
public:
	typedef sigc::signal<void> SignalFindTextChanged;
	typedef sigc::signal<void> SignalReplaceTextChanged;

	FindDialog(Gtk::Window& parent, Folder& folder,
	           StatusBar& status_bar);
	~FindDialog();

	bool get_search_only() const;
	void set_search_only(bool search_only);

	Glib::ustring get_find_text() const;
	Glib::ustring get_replace_text() const;

	bool find_next();
	bool find_previous();

	SignalFindTextChanged signal_find_text_changed() const
	{
		return m_signal_find_text_changed;
	}

	SignalReplaceTextChanged signal_replace_text_changed() const
	{
		return m_signal_replace_text_changed;
	}
protected:
	enum SearchDirection {
		SEARCH_FORWARD,
		SEARCH_BACKWARD
	};

	virtual void on_show();
	virtual void on_response(int id);

	void on_document_changed(SessionView* view);
	void on_active_user_changed(InfUser* user);
	void on_find_text_changed();
	void on_replace_text_changed();

	SearchDirection get_direction() const;
	bool find();
	bool replace();
	bool replace_all();

	// Searches for an occurence with the provided options, selecting the
	// result, if any.
	bool find_and_select(const GtkTextIter* from,
	                     SearchDirection direction);

	// Searches for a occurence, beginning from from, and wrapping around
	// if the corresponding checkbutton is checked.
	bool find_wrap(const GtkTextIter* from,
	               SearchDirection direction,
	               GtkTextIter* match_start,
	               GtkTextIter* match_end);

	// Searches for an occurence in the given range
	bool find_range(const GtkTextIter* from,
	                const GtkTextIter* to,
	                SearchDirection direction,
	                GtkTextIter* match_start,
	                GtkTextIter* match_end);

	// Searches for an occurence, ignoring whole word only option
	bool find_range_once(const GtkTextIter* from,
	                     const GtkTextIter* to,
	                     SearchDirection direction,
	                     GtkTextIter* match_start,
	                     GtkTextIter* match_end);

	Folder& m_folder;
	StatusBar& m_status_bar;

	sigc::connection m_active_user_changed_connection;

	Gtk::VBox m_box_main;
	Gtk::Table m_table_entries;
	Gtk::Label m_label_find;
	Gtk::Label m_label_replace;
	Gtk::Entry m_entry_find;
	Gtk::Entry m_entry_replace;

	Gtk::CheckButton m_check_case;
	Gtk::CheckButton m_check_whole_word;
	Gtk::CheckButton m_check_backwards;
	Gtk::CheckButton m_check_wrap_around;

	Gtk::Button* m_button_replace;
	Gtk::Button* m_button_replace_all;

	SignalFindTextChanged m_signal_find_text_changed;
	SignalReplaceTextChanged m_signal_replace_text_changed;

private:
	void update_sensitivity();
};

}

#endif // _GOBBY_FINDDIALOG_HPP_
