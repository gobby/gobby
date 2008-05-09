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

#ifndef _GOBBY_PREFERENCESDIALOG_HPP_
#define _GOBBY_PREFERENCESDIALOG_HPP_

#include "core/preferences.hpp"

#include <gtkmm/dialog.h>
#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/notebook.h>
#include <gtkmm/alignment.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/fontbutton.h>
#include <gtkmm/colorbutton.h>

namespace Gobby
{

class PreferencesDialog : public Gtk::Dialog
{
public:
	class Group: public Gtk::Frame
	{
	public:
		Group(const Glib::ustring& title);
		void add(Gtk::Widget& widget);

	protected:
		Gtk::Alignment m_alignment;
		Gtk::VBox m_box;
	};

	class Page: public Gtk::Frame
	{
	public:
		Page();
		void add(Gtk::Widget& widget);

	protected:
		Gtk::VBox m_box;
	};

	class User: public Page
	{
	public:
		User(Preferences& preferences);

	protected:
		Group m_group_settings;
		Group m_group_paths;

		Gtk::HBox m_box_user_name;
		Gtk::Label m_lbl_user_name;
		Gtk::Entry m_ent_user_name;

		Gtk::HBox m_box_user_color;
		Gtk::Label m_lbl_user_color;
		/* TODO: Use an own color chooser to only choose hue */
		Gtk::ColorButton m_btn_user_color;

		Gtk::HBox m_box_path_host_directory;
		Gtk::Label m_lbl_path_host_directory;
		Gtk::FileChooserButton m_btn_path_host_directory;
	};

	class Editor: public Page
	{
	public:
		Editor(Preferences& preferences);

	protected:
		Group m_group_tab;
		Group m_group_indentation;
		Group m_group_homeend;

		Gtk::HBox m_box_tab_width;
		Gtk::Label m_lbl_tab_width;
		Gtk::SpinButton m_ent_tab_width;
		Gtk::CheckButton m_btn_tab_spaces;

		Gtk::CheckButton m_btn_indentation_auto;

		Gtk::CheckButton m_btn_homeend_smart;
	};

	class View: public Page
	{
	public:
		View(Preferences& preferences);
		void set(Preferences::View& view) const;

	protected:
		void on_wrap_text_toggled();
		void on_margin_display_toggled();

		Group m_group_wrap;
		Group m_group_linenum;
		Group m_group_curline;
		Group m_group_margin;
		Group m_group_bracket;

		Gtk::CheckButton m_btn_wrap_text;
		Gtk::CheckButton m_btn_wrap_words;

		Gtk::CheckButton m_btn_linenum_display;

		Gtk::CheckButton m_btn_curline_highlight;

		Gtk::CheckButton m_btn_margin_display;
		Gtk::HBox m_box_margin_pos;
		Gtk::Label m_lbl_margin_pos;
		Gtk::SpinButton m_ent_margin_pos;

		Gtk::CheckButton m_btn_bracket_highlight;
	};

	class Appearance: public Page
	{
	public:
		Appearance(Preferences& preferences);

	protected:
		Group m_group_toolbar;
		Group m_group_font;

		Gtk::ComboBoxText m_cmb_toolbar_style;

		Gtk::FontButton m_btn_font;
	};

	PreferencesDialog(Gtk::Window& parent,
	                  Preferences& preferences);

protected:
	virtual void on_response(int id);

	Preferences& m_preferences;

	Gtk::Notebook m_notebook;

	User m_page_user;
	Editor m_page_editor;
	View m_page_view;
	Appearance m_page_appearance;
};

}

#endif // _GOBBY_PREFERENCESDIALOG_HPP_

