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

#ifndef _GOBBY_PREFERENCESDIALOG_HPP_
#define _GOBBY_PREFERENCESDIALOG_HPP_

#include <gtkmm/dialog.h>
#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/notebook.h>
#include "config.hpp"

namespace Gobby
{

class PreferencesDialog : public Gtk::Dialog
{
public:
	class Page : public Gtk::Frame
	{
		friend class PreferencesDialog;
	public:
		Page(Config& config);
		~Page();

	protected:
		virtual void on_response(int response_id) = 0;

		Config& m_config;
	};

	class Editor : public Page
	{
	public:
		Editor(Config& config);
		~Editor();

		unsigned int get_tab_width() const;
		bool get_tab_spaces() const;

		bool get_indentation_auto() const;
	protected:
		virtual void on_response(int response_id);

		Gtk::VBox m_box;
		Gtk::Frame m_frame_tab;
		Gtk::Frame m_frame_indentation;

		Gtk::VBox m_box_tab;
		Gtk::HBox m_box_tab_width;
		Gtk::Label m_lbl_tab_width;
		Gtk::SpinButton m_ent_tab_width;
		Gtk::CheckButton m_btn_tab_spaces;

		Gtk::VBox m_box_indentation;
		Gtk::CheckButton m_btn_indentation_auto;
	};

	class View : public Page
	{
	public:
		View(Config& config);
		~View();

		bool get_wrap_text() const;
		bool get_wrap_words() const;

		bool get_linenum_display() const;
	protected:
		virtual void on_response(int response_id);

		Gtk::VBox m_box;
		Gtk::Frame m_frame_wrap;
		Gtk::Frame m_frame_linenum;

		Gtk::VBox m_box_wrap;
		Gtk::CheckButton m_btn_wrap_text;
		Gtk::CheckButton m_btn_wrap_words;

		Gtk::VBox m_box_linenum;
		Gtk::CheckButton m_btn_linenum_display;
	};

	class Appearance : public Page
	{
	public:
		Appearance(Config& config);
		~Appearance();

		// Fetch the font
	protected:
		virtual void on_response(int response_id);

		Gtk::VBox m_box;

		Gtk::Frame m_frame_font;

		// Font chooser
	};

	PreferencesDialog(Gtk::Window& parent, Config& config);
	~PreferencesDialog();

	const Editor& editor() const;
	const View& view() const;
	const Appearance& appearance() const;

protected:
	Gtk::Notebook m_notebook;
	Editor m_page_editor;
	View m_page_view;
	Appearance m_page_appearance;

	Config& m_config;
};

}

#endif // _GOBBY_PREFERENCESDIALOG_HPP_


