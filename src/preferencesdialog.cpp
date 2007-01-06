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

#include <gtkmm/stock.h>
#include "common.hpp"
#include "preferencesdialog.hpp"

Gobby::PreferencesDialog::Page::Page(const Preferences& preferences)
 : Gtk::Frame(), m_preferences(preferences)
{
	// Remove shadow - use the frame just as container
	set_shadow_type(Gtk::SHADOW_NONE);
}

Gobby::PreferencesDialog::Editor::Editor(const Preferences& preferences)
 : Page(preferences), m_frame_tab(_("Tab Stops") ),
   m_frame_indentation(_("Indentation") ),
   m_lbl_tab_width(_("Tab width:"), Gtk::ALIGN_RIGHT),
   m_btn_tab_spaces(_("Use tabs instead of spaces") ),
   m_btn_indentation_auto(_("Enable automatic indentation") )
{
	unsigned int tab_width = preferences.editor.tab_width;
	bool tab_spaces = preferences.editor.tab_spaces;
	bool indentation_auto = preferences.editor.indentation_auto;

	m_ent_tab_width.set_range(1, 8);
	m_ent_tab_width.set_value(tab_width);
	m_ent_tab_width.set_increments(1, 1);

	m_box_tab_width.set_spacing(5);
	m_box_tab_width.pack_start(m_lbl_tab_width, Gtk::PACK_SHRINK);
	m_box_tab_width.pack_start(m_ent_tab_width, Gtk::PACK_EXPAND_WIDGET);

	m_btn_tab_spaces.set_active(tab_spaces);
	m_btn_indentation_auto.set_active(indentation_auto);

	m_box_tab.set_spacing(5);
	m_box_tab.set_border_width(5);
	m_box_tab.pack_start(m_box_tab_width, Gtk::PACK_SHRINK);
	m_box_tab.pack_start(m_btn_tab_spaces, Gtk::PACK_SHRINK);

	m_box_indentation.set_spacing(5);
	m_box_indentation.set_border_width(5);
	m_box_indentation.pack_start(m_btn_indentation_auto, Gtk::PACK_SHRINK);

	m_frame_tab.add(m_box_tab);
	m_frame_indentation.add(m_box_indentation);

	m_box.set_spacing(5);
	m_box.pack_start(m_frame_tab, Gtk::PACK_SHRINK);
	m_box.pack_start(m_frame_indentation, Gtk::PACK_SHRINK);

	set_border_width(10);
	add(m_box);
}

Gobby::PreferencesDialog::Editor::~Editor()
{
}

unsigned int Gobby::PreferencesDialog::Editor::get_tab_width() const
{
	return static_cast<unsigned int>(m_ent_tab_width.get_value() );
}

bool Gobby::PreferencesDialog::Editor::get_tab_spaces() const
{
	return m_btn_tab_spaces.get_active();
}

bool Gobby::PreferencesDialog::Editor::get_indentation_auto() const
{
	return m_btn_indentation_auto.get_active();
}

Gobby::PreferencesDialog::View::View(const Preferences& preferences)
 : Page(preferences), m_frame_wrap(_("Text wrapping") ),
   m_frame_linenum(_("Line numbers") ),
   m_btn_wrap_text(_("Enable text wrapping") ),
   m_btn_wrap_words(_("Do not split words over two lines") ),
   m_btn_linenum_display(_("Display line numbers") )
{
	bool wrap_text = preferences.view.wrap_text;
	bool wrap_words = preferences.view.wrap_words;
	bool linenum_display = preferences.view.linenum_display;

	m_btn_wrap_text.set_active(wrap_text);
	m_btn_wrap_words.set_active(wrap_words);
	m_btn_linenum_display.set_active(linenum_display);

	m_box_wrap.set_spacing(5);
	m_box_wrap.set_border_width(5);
	m_box_wrap.pack_start(m_btn_wrap_text, Gtk::PACK_SHRINK);
	m_box_wrap.pack_start(m_btn_wrap_words, Gtk::PACK_SHRINK);
	m_box_linenum.set_spacing(5);
	m_box_linenum.set_border_width(5);
	m_box_linenum.pack_start(m_btn_linenum_display, Gtk::PACK_SHRINK);
	
	m_frame_wrap.add(m_box_wrap);
	m_frame_linenum.add(m_box_linenum);

	m_box.set_spacing(5);
	m_box.pack_start(m_frame_wrap, Gtk::PACK_SHRINK);
	m_box.pack_start(m_frame_linenum, Gtk::PACK_SHRINK);

	set_border_width(10);
	add(m_box);
}

Gobby::PreferencesDialog::View::~View()
{
}

bool Gobby::PreferencesDialog::View::get_wrap_text() const
{
	return m_btn_wrap_text.get_active();
}

bool Gobby::PreferencesDialog::View::get_wrap_words() const
{
	return !m_btn_wrap_words.get_active();
}

bool Gobby::PreferencesDialog::View::get_linenum_display() const
{
	return m_btn_linenum_display.get_active();
}

Gobby::PreferencesDialog::Appearance::Appearance(
	const Gobby::Preferences& preferences)
 : Page(preferences)
{
}

Gobby::PreferencesDialog::Appearance::~Appearance()
{
}

Gobby::PreferencesDialog::Security::Security(const Preferences& preferences)
 : Page(preferences)
{
}

Gobby::PreferencesDialog::Security::~Security()
{
}

Gobby::PreferencesDialog::PreferencesDialog(Gtk::Window& parent,
                                            const Preferences& preferences)
 : Gtk::Dialog(_("Preferences"), parent, true),
   m_page_editor(preferences), m_page_view(preferences),
   m_page_appearance(preferences)
{
	m_notebook.append_page(m_page_editor, _("Editor") );
	m_notebook.append_page(m_page_view, _("View") );
//	m_notebook.append_page(m_page_appearance, _("Appearance") );

	get_vbox()->set_spacing(5);
	get_vbox()->pack_start(m_notebook, Gtk::PACK_EXPAND_WIDGET);

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

	set_border_width(10);
	set_resizable(false);

	show_all();
}

Gobby::PreferencesDialog::~PreferencesDialog()
{
}

const Gobby::PreferencesDialog::Editor&
Gobby::PreferencesDialog::editor() const
{
	return m_page_editor;
}

const Gobby::PreferencesDialog::View&
Gobby::PreferencesDialog::view() const
{
	return m_page_view;
}

const Gobby::PreferencesDialog::Appearance&
Gobby::PreferencesDialog::appearance() const
{
	return m_page_appearance;
}

