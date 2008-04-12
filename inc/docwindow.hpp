/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 - 2008 0x539 dev group
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

#ifndef _GOBBY_DOCWINDOW_HPP_
#define _GOBBY_DOCWINDOW_HPP_

#include "preferences.hpp"
#include "features.hpp"

#include <gtkmm/box.h>
#include <gtkmm/paned.h>
#include <gtkmm/textiter.h>

#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcelanguagemanager.h>

#include <libinftext/inf-text-session.h>

namespace Gobby
{

class DocWindow: public Gtk::HPaned
{
public:
	typedef sigc::signal<void, GtkSourceLanguage*> SignalLanguageChanged;

	DocWindow(InfTextSession* session, const Glib::ustring& title,
	          const Preferences& preferences,
	          GtkSourceLanguageManager* manager);
	virtual ~DocWindow();

	const InfTextSession* get_session() const { return m_session; }
	InfTextSession* get_session() { return m_session; }
	const Glib::ustring& get_title() const { return m_title; }

	void get_cursor_position(unsigned int& row, unsigned int& col) const;
	void set_selection(const Gtk::TextIter& begin,
	                   const Gtk::TextIter& end);
	Glib::ustring get_selected_text() const;

	GtkSourceLanguage* get_language() const;
	void set_language(GtkSourceLanguage* language);

	GtkSourceView* get_text_view() { return m_view; }
	GtkSourceBuffer* get_text_buffer() { return m_buffer; }

	void set_info(const Glib::ustring& message);

	SignalLanguageChanged signal_language_changed() const {
		return m_signal_language_changed;
	}

protected:
	void on_tab_width_changed();
	void on_tab_spaces_changed();
	void on_auto_indent_changed();
	void on_homeend_smart_changed();

	void on_wrap_mode_changed();
	void on_linenum_display_changed();
	void on_curline_highlight_changed();
	void on_margin_display_changed();
	void on_margin_pos_changed();
	void on_bracket_highlight_changed();

	void on_font_changed();

	InfTextSession* m_session;
	Glib::ustring m_title;
	const Preferences& m_preferences;

	GtkSourceView* m_view;
	GtkSourceBuffer* m_buffer;

	Gtk::VBox m_info_box;

	SignalLanguageChanged m_signal_language_changed;
};

}

#endif // _GOBBY_DOCWINDOW_HPP_
