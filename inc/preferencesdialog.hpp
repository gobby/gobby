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
#include "preferences.hpp"

namespace Gobby
{

class PreferencesDialog : public Gtk::Dialog
{
public:
	class Page : public Gtk::Frame
	{
	public:
		Page(const Preferences& preferences);

	protected:
		const Preferences& m_preferences;
	};

	class Editor : public Page
	{
	public:
		Editor(const Preferences& preferences);
		~Editor();

		unsigned int get_tab_width() const;
		bool get_tab_spaces() const;

		bool get_indentation_auto() const;
	protected:
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
		View(const Preferences& preferences);
		~View();

		bool get_wrap_text() const;
		bool get_wrap_words() const;

		bool get_linenum_display() const;
	protected:
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
		Appearance(const Preferences& preferences);
		~Appearance();

		// Fetch the font
	protected:
		Gtk::VBox m_box;

		Gtk::Frame m_frame_font;

		// Font chooser
	};

v v v v v v v
	class Security : public Page
	{
	public:
		Security(Config& config);
		~Security();

		// Fetch the key components
	protected:
		virtual void on_response(int response_id);

		Gtk::VBox m_box;

		Gtk::VBox m_box_key;
		// Display the current public key ID and let the user the
		// opportunity to regenerate it.
	};

	PreferencesDialog(Gtk::Window& parent, const Preferences& preferences);
*************
	class Security : public Page
	{
	public:
		Security(const Preferences& preferences);
		~Security();

		// Fetch the key components
	protected:
		Gtk::VBox m_box;

		Gtk::VBox m_box_key;
		// Display the current public key ID and let the user the
		// opportunity to regenerate it.
	};

	PreferencesDialog(Gtk::Window& parent, const Preferences& preferences);
^ ^ ^ ^ ^ ^ ^
	~PreferencesDialog();

	const Editor& editor() const;
	const View& view() const;
	const Appearance& appearance() const;

protected:
	Gtk::Notebook m_notebook;

	Editor m_page_editor;
	View m_page_view;
	Appearance m_page_appearance;
};

}

#endif // _GOBBY_PREFERENCESDIALOG_HPP_

