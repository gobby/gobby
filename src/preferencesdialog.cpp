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

Gobby::PreferencesDialog::Editor::Editor(const Preferences& preferences,
                                         Gtk::Tooltips& tooltips)
 : Page(preferences), m_frame_tab(_("Tab Stops") ),
   m_frame_indentation(_("Indentation") ),
   m_frame_homeend(_("Home/End behaviour") ),
   m_lbl_tab_width(_("Tab width:"), Gtk::ALIGN_RIGHT),
   m_btn_tab_spaces(_("Insert spaces instead of tabs") ),
   m_btn_indentation_auto(_("Enable automatic indentation") ),
   m_btn_homeend_smart(_("Smart home/end") )
{
	unsigned int tab_width = preferences.editor.tab_width;
	bool tab_spaces = preferences.editor.tab_spaces;
	bool indentation_auto = preferences.editor.indentation_auto;
	bool homeend_smart = preferences.editor.homeend_smart;

	m_ent_tab_width.set_range(1, 8);
	m_ent_tab_width.set_value(tab_width);
	m_ent_tab_width.set_increments(1, 1);

	// TODO: Improve this description
	tooltips.set_tip(m_btn_homeend_smart,
		_("With this option enabled, Home/End keys move to first/last "
		  "character before going to the start/end of the line.") );

	m_box_tab_width.set_spacing(5);
	m_box_tab_width.pack_start(m_lbl_tab_width, Gtk::PACK_SHRINK);
	m_box_tab_width.pack_start(m_ent_tab_width, Gtk::PACK_EXPAND_WIDGET);

	m_btn_tab_spaces.set_active(tab_spaces);
	m_btn_indentation_auto.set_active(indentation_auto);
	m_btn_homeend_smart.set_active(homeend_smart);

	m_box_tab.set_spacing(5);
	m_box_tab.set_border_width(5);
	m_box_tab.pack_start(m_box_tab_width, Gtk::PACK_SHRINK);
	m_box_tab.pack_start(m_btn_tab_spaces, Gtk::PACK_SHRINK);

	m_box_indentation.set_spacing(5);
	m_box_indentation.set_border_width(5);
	m_box_indentation.pack_start(m_btn_indentation_auto, Gtk::PACK_SHRINK);

	m_box_homeend.set_spacing(5);
	m_box_homeend.set_border_width(5);
	m_box_homeend.pack_start(m_btn_homeend_smart, Gtk::PACK_SHRINK);

	m_frame_tab.add(m_box_tab);
	m_frame_indentation.add(m_box_indentation);
	m_frame_homeend.add(m_box_homeend);

	m_box.set_spacing(5);
	m_box.pack_start(m_frame_tab, Gtk::PACK_SHRINK);
	m_box.pack_start(m_frame_indentation, Gtk::PACK_SHRINK);
	m_box.pack_start(m_frame_homeend, Gtk::PACK_SHRINK);

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

bool Gobby::PreferencesDialog::Editor::get_homeend_smart() const
{
	return m_btn_homeend_smart.get_active();
}

Gobby::PreferencesDialog::View::View(const Preferences& preferences)
 : Page(preferences), m_frame_wrap(_("Text wrapping") ),
   m_frame_linenum(_("Line numbers") ),
   m_frame_curline(_("Current line") ),
   m_frame_margin(_("Right margin") ),
   m_frame_bracket(_("Bracket matching") ),
   m_btn_wrap_text(_("Enable text wrapping") ),
   m_btn_wrap_words(_("Do not split words over two lines") ),
   m_btn_linenum_display(_("Display line numbers") ),
   m_btn_curline_highlight(_("Highlight current line") ),
   m_btn_margin_display(_("Display right margin") ),
   m_lbl_margin_pos(_("Right margin at column:") ),
   m_btn_bracket_highlight(_("Highlight matching bracket") )
{
	bool wrap_text = preferences.view.wrap_text;
	bool wrap_words = preferences.view.wrap_words;
	bool linenum_display = preferences.view.linenum_display;
	bool curline_highlight = preferences.view.curline_highlight;
	bool margin_display = preferences.view.margin_display;
	unsigned int margin_pos = preferences.view.margin_pos;
	bool bracket_highlight = preferences.view.bracket_highlight;

	m_btn_margin_display.signal_toggled().connect(
		sigc::mem_fun(*this, &View::on_margin_display_toggled) );

	m_ent_margin_pos.set_range(1, 1024);
	m_ent_margin_pos.set_value(margin_pos);
	m_ent_margin_pos.set_increments(1, 16);

	m_btn_wrap_text.set_active(wrap_text);
	m_btn_wrap_words.set_active(!wrap_words);
	m_btn_linenum_display.set_active(linenum_display);
	m_btn_curline_highlight.set_active(curline_highlight);
	m_btn_margin_display.set_active(margin_display);
	m_btn_bracket_highlight.set_active(bracket_highlight);

	m_box_margin_pos.set_spacing(5);
	m_box_margin_pos.pack_start(m_lbl_margin_pos, Gtk::PACK_SHRINK);
	m_box_margin_pos.pack_start(m_ent_margin_pos, Gtk::PACK_EXPAND_WIDGET);
	m_box_wrap.set_spacing(5);
	m_box_wrap.set_border_width(5);
	m_box_wrap.pack_start(m_btn_wrap_text, Gtk::PACK_SHRINK);
	m_box_wrap.pack_start(m_btn_wrap_words, Gtk::PACK_SHRINK);

	m_box_linenum.set_spacing(5);
	m_box_linenum.set_border_width(5);
	m_box_linenum.pack_start(m_btn_linenum_display, Gtk::PACK_SHRINK);

	m_box_curline.set_spacing(5);
	m_box_curline.set_border_width(5);
	m_box_curline.pack_start(m_btn_curline_highlight, Gtk::PACK_SHRINK);

	m_box_margin.set_spacing(5);
	m_box_margin.set_border_width(5);
	m_box_margin.pack_start(m_btn_margin_display, Gtk::PACK_SHRINK);
	m_box_margin.pack_start(m_box_margin_pos, Gtk::PACK_SHRINK);

	m_box_bracket.set_spacing(5);
	m_box_bracket.set_border_width(5);
	m_box_bracket.pack_start(m_btn_bracket_highlight, Gtk::PACK_SHRINK);
	
	m_frame_wrap.add(m_box_wrap);
	m_frame_linenum.add(m_box_linenum);
	m_frame_curline.add(m_box_curline);
	m_frame_margin.add(m_box_margin);
	m_frame_bracket.add(m_box_bracket);

	m_box.set_spacing(5);
	m_box.pack_start(m_frame_wrap, Gtk::PACK_SHRINK);
	m_box.pack_start(m_frame_linenum, Gtk::PACK_SHRINK);
	m_box.pack_start(m_frame_curline, Gtk::PACK_SHRINK);
	m_box.pack_start(m_frame_margin, Gtk::PACK_SHRINK);
	m_box.pack_start(m_frame_bracket, Gtk::PACK_SHRINK);

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

bool Gobby::PreferencesDialog::View::get_curline_highlight() const
{
	return m_btn_curline_highlight.get_active();
}

bool Gobby::PreferencesDialog::View::get_margin_display() const
{
	return m_btn_margin_display.get_active();
}

unsigned int Gobby::PreferencesDialog::View::get_margin_pos() const
{
	return static_cast<unsigned int>(m_ent_margin_pos.get_value() );
}

bool Gobby::PreferencesDialog::View::get_bracket_highlight() const
{
	return m_btn_bracket_highlight.get_active();
}

void Gobby::PreferencesDialog::View::on_margin_display_toggled()
{
	m_box_margin_pos.set_sensitive(m_btn_margin_display.get_active() );
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
   m_page_editor(preferences, m_tooltips), m_page_view(preferences),
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

Gobby::Preferences Gobby::PreferencesDialog::preferences() const
{
	Preferences preferences;

	preferences.editor.tab_width = m_page_editor.get_tab_width();
	preferences.editor.tab_spaces = m_page_editor.get_tab_spaces();
	preferences.editor.indentation_auto =
		m_page_editor.get_indentation_auto();
	preferences.editor.homeend_smart = m_page_editor.get_homeend_smart();

	preferences.view.wrap_text = m_page_view.get_wrap_text();
	preferences.view.wrap_words = m_page_view.get_wrap_words();
	preferences.view.linenum_display = m_page_view.get_linenum_display();
	preferences.view.curline_highlight =
		m_page_view.get_curline_highlight();
	preferences.view.margin_display = m_page_view.get_margin_display();
	preferences.view.margin_pos = m_page_view.get_margin_pos();
	preferences.view.bracket_highlight =
		m_page_view.get_bracket_highlight();

	return preferences;
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

