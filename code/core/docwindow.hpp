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

#ifndef _GOBBY_DOCWINDOW_HPP_
#define _GOBBY_DOCWINDOW_HPP_

#include "core/userlist.hpp"
#include "core/preferences.hpp"
#include "util/closebutton.hpp"
#include "features.hpp"

#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/paned.h>
#include <gtkmm/textiter.h>

#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcelanguagemanager.h>

#include <libinftext/inf-text-session.h>
#include <libinftext/inf-text-user.h>

namespace Gobby
{

class DocWindow: public Gtk::HPaned
{
public:
	typedef sigc::signal<void, GtkSourceLanguage*> SignalLanguageChanged;
	typedef sigc::signal<void, InfTextUser*> SignalActiveUserChanged;

	DocWindow(InfTextSession* session, const Glib::ustring& title,
	          const Glib::ustring& path, const Glib::ustring& hostname,
	          const std::string& info_storage_key,
	          Preferences& preferences,
	          GtkSourceLanguageManager* manager);
	virtual ~DocWindow();

	const InfTextSession* get_session() const { return m_session; }
	InfTextSession* get_session() { return m_session; }
	const Glib::ustring& get_title() const { return m_title; }
	const Glib::ustring& get_path() const { return m_path; }
	const Glib::ustring& get_hostname() const { return m_hostname; }
	const std::string& get_info_storage_key() const
	{
		return m_info_storage_key;
	}

	void get_cursor_position(unsigned int& row, unsigned int& col) const;
	void set_selection(const GtkTextIter* begin,
	                   const GtkTextIter* end);
	Glib::ustring get_selected_text() const;
	void scroll_to_cursor_position(double within_margin);

	GtkSourceLanguage* get_language() const;
	void set_language(GtkSourceLanguage* language);

	InfTextUser* get_active_user() const;
	void set_active_user(InfTextUser* user);

	GtkSourceView* get_text_view() { return m_view; }
	GtkSourceBuffer* get_text_buffer() { return m_buffer; }

	void set_info(const Glib::ustring& info, bool closable);
	void unset_info();

	SignalLanguageChanged signal_language_changed() const
	{
		return m_signal_language_changed;
	}

	SignalActiveUserChanged signal_active_user_changed() const
	{
		return m_signal_active_user_changed;
	}

protected:
	virtual void on_size_allocate(Gtk::Allocation& allocation);

	void on_user_color_changed();

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

	void on_doc_userlist_width_changed();
	void on_pref_userlist_width_changed();

	bool on_query_tooltip(int x, int y, bool keyboard_mode,
	                      const Glib::RefPtr<Gtk::Tooltip>& tooltip);

	static gboolean on_query_tooltip_static(GtkWidget* widget, int x,
	                                        int y, gboolean keyboard_mode,
	                                        GtkTooltip* tooltip,
	                                        gpointer user_data)
	{
		return static_cast<DocWindow*>(user_data)->on_query_tooltip(
			x, y, keyboard_mode, Glib::wrap(tooltip, true));
	}

	InfTextSession* m_session;
	Glib::ustring m_title;
	Glib::ustring m_path;
	Glib::ustring m_hostname;
	std::string m_info_storage_key;
	Preferences& m_preferences;

	GtkSourceView* m_view;
	GtkSourceBuffer* m_buffer;
	UserList m_userlist;

	Gtk::Frame m_info_frame;
	Gtk::VBox m_info_box;
	Gtk::HBox m_info_close_button_box;
	CloseButton m_info_close_button;
	Gtk::Label m_info_label;

	SignalLanguageChanged m_signal_language_changed;
	SignalActiveUserChanged m_signal_active_user_changed;

	sigc::connection m_doc_userlist_width_changed_connection;
	sigc::connection m_pref_userlist_width_changed_connection;
};

}

#endif // _GOBBY_DOCWINDOW_HPP_
