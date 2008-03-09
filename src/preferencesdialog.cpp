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

#include <stdexcept>
#include <gtkmm/stock.h>
#include <gtkmm/messagedialog.h>

#include <obby/format_string.hpp>

#include "common.hpp"
#include "colorutil.hpp"
#include "preferencesdialog.hpp"

namespace
{
	using namespace Gobby;

	Gtk::ToolbarStyle rownum_to_toolstyle(int rownum)
	{
		switch(rownum)
		{
		case 0: return Gtk::TOOLBAR_TEXT;
		case 1: return Gtk::TOOLBAR_ICONS;
		case 3: return Gtk::TOOLBAR_BOTH_HORIZ;
		case 2: default: return Gtk::TOOLBAR_BOTH;
		}
	}

	Gtk::WrapMode
	wrap_mode_from_check_buttons(Gtk::CheckButton& char_button,
	                             Gtk::CheckButton& word_button)
	{
		if(!char_button.get_active())
			return Gtk::WRAP_NONE;
		else if(!word_button.get_active())
			return Gtk::WRAP_CHAR;
		else
			return Gtk::WRAP_WORD_CHAR;
	}

	Pango::FontDescription create_description(const Glib::ustring& str)
	{
		return Pango::FontDescription(str);
	}

	template<typename OptionType>
	void connect_option(Gtk::CheckButton& checkbutton,
	                    Preferences::Option<OptionType>& option)
	{
		checkbutton.signal_toggled().connect(
			sigc::compose(
				sigc::mem_fun(
					option,
					&Preferences::Option<OptionType>::set
				),
				sigc::mem_fun(
					checkbutton,
					&Gtk::CheckButton::get_active
				)
			)
		);
	}

	template<typename OptionType>
	void connect_option(Gtk::SpinButton& spinbutton,
	                    Preferences::Option<OptionType>& option)
	{
		spinbutton.signal_value_changed().connect(
			sigc::compose(
				sigc::mem_fun(
					option,
					&Preferences::Option<OptionType>::set
				),
				sigc::mem_fun(
					spinbutton,
					&Gtk::SpinButton::get_value
				)
			)
		);
	}

	template<typename OptionType>
	void connect_option(Gtk::Entry& entry,
	                    Preferences::Option<OptionType>& option)
	{
		entry.signal_changed().connect(
			sigc::compose(
				sigc::mem_fun(
					option,
					&Preferences::Option<OptionType>::set
				),
				sigc::mem_fun(
					entry,
					&Gtk::Entry::get_text
				)
			)
		);
	}

	void
	connect_toolbar_style_option(Gtk::ComboBox& box,
	                             Preferences::Option<Gtk::ToolbarStyle>&
	                             option)
	{
		box.signal_changed().connect(
			sigc::compose(
				sigc::mem_fun(
					option,
					&Preferences::Option<
						Gtk::ToolbarStyle>::set
				),
				sigc::compose(
					sigc::ptr_fun(rownum_to_toolstyle),
					sigc::mem_fun(
						box,
						&Gtk::ComboBox::
							get_active_row_number
					)
				)
			)
		);
	}

	void connect_wrap_option(Gtk::CheckButton& char_button,
	                         Gtk::CheckButton& word_button,
				 Preferences::Option<Gtk::WrapMode>& option)
	{
		sigc::slot<void> toggled_slot(
			sigc::compose(
				sigc::mem_fun(
					option,
					&Preferences::Option<
						Gtk::WrapMode>::set
				),
				sigc::bind(
					sigc::ptr_fun(
						wrap_mode_from_check_buttons),
					sigc::ref(char_button),
					sigc::ref(word_button)
				)
			)
		);

		char_button.signal_toggled().connect(toggled_slot);
		word_button.signal_toggled().connect(toggled_slot);
	}

	void connect_hue_option(Gtk::ColorButton& color_button,
	                        Preferences::Option<double>& option)
	{
		color_button.signal_color_set().connect(
			sigc::compose(
				sigc::mem_fun(
					option,
					&Preferences::Option<double>::set
				),
				sigc::compose(
					sigc::ptr_fun(&hue_from_gdk_color),
					sigc::mem_fun(
						color_button,
						&Gtk::ColorButton::get_color
					)
				)
			)
		);
	}

	void connect_path_option(Gtk::FileChooser& file_chooser,
	                         Preferences::Option<std::string>& option)
	{
		file_chooser.signal_selection_changed().connect(
			sigc::compose(
				sigc::mem_fun(
					option,
					&Preferences::Option<std::string>::set
				),
				sigc::mem_fun(
					file_chooser,
					&Gtk::FileChooser::get_filename
				)
			)
		);
	}

	void connect_font_option(Gtk::FontButton& button,
	                         Preferences::Option<Pango::FontDescription>&
	                         option)
	{
		button.signal_font_set().connect(
			sigc::compose(
				sigc::mem_fun(
					option,
					&Preferences::Option<
						Pango::FontDescription>::set
				),
				sigc::compose(
					sigc::ptr_fun(&create_description),
					sigc::mem_fun(
						button,
						&Gtk::FontButton::
							get_font_name
					)
				)
			)
		);
	}
}

Gobby::PreferencesDialog::Group::Group(const Glib::ustring& title):
	m_box(false, 6)
{
	Gtk::Label* title_label = Gtk::manage(new Gtk::Label);
	title_label->set_markup(
		"<b>" + Glib::Markup::escape_text(title) + "</b>");
	set_label_widget(*title_label);
	title_label->show();

	m_box.show();

	m_alignment.set_padding(6, 0, 12, 0);
	m_alignment.add(m_box);
	m_alignment.show();

	set_shadow_type(Gtk::SHADOW_NONE);
	Gtk::Frame::add(m_alignment);
}

void Gobby::PreferencesDialog::Group::add(Gtk::Widget& widget)
{
	m_box.pack_start(widget, Gtk::PACK_SHRINK);
}

Gobby::PreferencesDialog::Page::Page():
	Gtk::Frame(), m_box(false, 12)
{
	Gtk::Frame::add(m_box);
	m_box.show();

	// Remove shadow - use the frame just as container
	set_shadow_type(Gtk::SHADOW_NONE);
	set_border_width(12);
}

void Gobby::PreferencesDialog::Page::add(Gtk::Widget& widget)
{
	m_box.pack_start(widget, Gtk::PACK_SHRINK);
}

Gobby::PreferencesDialog::User::User(Preferences& preferences):
	m_group_settings(_("Settings")),
	m_group_paths(_("Paths")),
	m_box_user_name(false, 6),
	m_lbl_user_name(_("User name:")),
	m_box_user_color(false, 6),
	m_lbl_user_color(_("User color:")),
	m_box_path_host_directory(false, 6),
	m_btn_path_host_directory(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
	m_lbl_path_host_directory(_("Host directory:"))
{
	m_lbl_user_name.show();
	m_ent_user_name.set_text(preferences.user.name);
	m_ent_user_name.show();
	connect_option(m_ent_user_name, preferences.user.name);

	m_box_user_name.pack_start(m_lbl_user_name, Gtk::PACK_SHRINK);
	m_box_user_name.pack_start(m_ent_user_name, Gtk::PACK_EXPAND_WIDGET);
	m_box_user_name.show();

	Gdk::Color color;
	color.set_hsv(preferences.user.hue * 360, 0.8, 1.0);
	m_lbl_user_color.show();
	m_btn_user_color.set_color(color);
	m_btn_user_color.show();
	connect_hue_option(m_btn_user_color, preferences.user.hue);

	m_box_user_color.pack_start(m_lbl_user_color, Gtk::PACK_SHRINK);
	m_box_user_color.pack_start(
		m_btn_user_color, Gtk::PACK_EXPAND_WIDGET);
	m_box_user_color.show();

	m_group_settings.add(m_box_user_name);
	m_group_settings.add(m_box_user_color);
	m_group_settings.show();

	m_lbl_path_host_directory.show();
	m_btn_path_host_directory.set_current_folder(
		static_cast<const std::string&>(
			preferences.user.host_directory));
	m_btn_path_host_directory.show();
	connect_path_option(m_btn_path_host_directory,
	                    preferences.user.host_directory);

	m_box_path_host_directory.set_tooltip_text(
		_("The directory into which locally hosted sessions "
		  "are permanently stored"));
	m_box_path_host_directory.pack_start(
		m_lbl_path_host_directory, Gtk::PACK_SHRINK);
	m_box_path_host_directory.pack_start(
		m_btn_path_host_directory, Gtk::PACK_EXPAND_WIDGET);
	m_box_path_host_directory.show();

	m_group_paths.add(m_box_path_host_directory);
	m_group_paths.show();

	add(m_group_settings);
	add(m_group_paths);
}

Gobby::PreferencesDialog::Editor::Editor(Preferences& preferences):
	m_group_tab(_("Tab Stops") ),
	m_group_indentation(_("Indentation") ),
	m_group_homeend(_("Home/End behaviour") ),
	m_lbl_tab_width(_("Tab width:"), Gtk::ALIGN_RIGHT),
	m_btn_tab_spaces(_("Insert spaces instead of tabs") ),
	m_btn_indentation_auto(_("Enable automatic indentation") ),
	m_btn_homeend_smart(_("Smart home/end") )
{
	unsigned int tab_width = preferences.editor.tab_width;
	bool tab_spaces = preferences.editor.tab_spaces;
	bool indentation_auto = preferences.editor.indentation_auto;
	bool homeend_smart = preferences.editor.homeend_smart;

	m_lbl_tab_width.show();
	m_ent_tab_width.set_range(1, 8);
	m_ent_tab_width.set_value(tab_width);
	m_ent_tab_width.set_increments(1, 1);
	m_ent_tab_width.show();
	connect_option(m_ent_tab_width, preferences.editor.tab_width);

	m_btn_homeend_smart.set_tooltip_text(
		_("With this option enabled, Home/End keys move to "
		  "first/last character before going to the start/end of the "
		  "line.")
	);

	m_box_tab_width.set_spacing(6);
	m_box_tab_width.pack_start(m_lbl_tab_width, Gtk::PACK_SHRINK);
	m_box_tab_width.pack_start(m_ent_tab_width, Gtk::PACK_EXPAND_WIDGET);
	m_box_tab_width.show();

	m_btn_tab_spaces.set_active(tab_spaces);
	m_btn_tab_spaces.show();
	connect_option(m_btn_tab_spaces, preferences.editor.tab_spaces);
	m_btn_indentation_auto.set_active(indentation_auto);
	m_btn_indentation_auto.show();
	connect_option(m_btn_indentation_auto,
	               preferences.editor.indentation_auto);
	m_btn_homeend_smart.set_active(homeend_smart);
	m_btn_homeend_smart.show();
	connect_option(m_btn_homeend_smart, preferences.editor.homeend_smart);

	m_group_tab.add(m_box_tab_width);
	m_group_tab.add(m_btn_tab_spaces);
	m_group_tab.show();

	m_group_indentation.add(m_btn_indentation_auto);
	m_group_indentation.show();

	m_group_homeend.add(m_btn_homeend_smart);
	m_group_homeend.show();

	add(m_group_tab);
	add(m_group_indentation);
	add(m_group_homeend);
}

Gobby::PreferencesDialog::View::View(Preferences& preferences):
	m_group_wrap(_("Text wrapping") ),
	m_group_linenum(_("Line numbers") ),
	m_group_curline(_("Current line") ),
	m_group_margin(_("Right margin") ),
	m_group_bracket(_("Bracket matching") ),
	m_btn_wrap_text(_("Enable text wrapping") ),
	m_btn_wrap_words(_("Do not split words over two lines") ),
	m_btn_linenum_display(_("Display line numbers") ),
	m_btn_curline_highlight(_("Highlight current line") ),
	m_btn_margin_display(_("Display right margin") ),
	m_lbl_margin_pos(_("Right margin at column:") ),
	m_btn_bracket_highlight(_("Highlight matching bracket") )
{
	Gtk::WrapMode mode = preferences.view.wrap_mode;
	bool linenum_display = preferences.view.linenum_display;
	bool curline_highlight = preferences.view.curline_highlight;
	bool margin_display = preferences.view.margin_display;
	unsigned int margin_pos = preferences.view.margin_pos;
	bool bracket_highlight = preferences.view.bracket_highlight;

	m_btn_margin_display.signal_toggled().connect(
		sigc::mem_fun(*this, &View::on_margin_display_toggled));
	m_btn_wrap_text.signal_toggled().connect(
		sigc::mem_fun(*this, &View::on_wrap_text_toggled));

	m_ent_margin_pos.set_range(1, 1024);
	m_ent_margin_pos.set_value(margin_pos);
	m_ent_margin_pos.set_increments(1, 16);
	m_ent_margin_pos.show();
	connect_option(m_ent_margin_pos, preferences.view.margin_pos);

	m_btn_wrap_text.set_active(mode != Gtk::WRAP_NONE);
	m_btn_wrap_text.show();
	m_btn_wrap_words.set_active(mode == Gtk::WRAP_WORD_CHAR);
	m_btn_wrap_words.set_sensitive(mode != Gtk::WRAP_NONE);
	m_btn_wrap_words.show();
	connect_wrap_option(m_btn_wrap_text,
	                    m_btn_wrap_words,
                            preferences.view.wrap_mode);
	m_btn_linenum_display.set_active(linenum_display);
	m_btn_linenum_display.show();
	connect_option(m_btn_linenum_display,
	               preferences.view.linenum_display);
	m_btn_curline_highlight.set_active(curline_highlight);
	m_btn_curline_highlight.show();
	connect_option(m_btn_curline_highlight,
	               preferences.view.curline_highlight);
	m_btn_margin_display.set_active(margin_display);
	m_btn_margin_display.show();
	connect_option(m_btn_margin_display,
	               preferences.view.margin_display);
	m_btn_bracket_highlight.set_active(bracket_highlight);
	m_btn_bracket_highlight.show();
	connect_option(m_btn_bracket_highlight,
	               preferences.view.bracket_highlight);

	m_box_margin_pos.set_spacing(6);
	m_box_margin_pos.set_sensitive(margin_display);
	m_box_margin_pos.pack_start(m_lbl_margin_pos, Gtk::PACK_SHRINK);
	m_box_margin_pos.pack_start(m_ent_margin_pos, Gtk::PACK_EXPAND_WIDGET);
	m_box_margin_pos.show();

	m_group_wrap.add(m_btn_wrap_text);
	m_group_wrap.add(m_btn_wrap_words);
	m_group_wrap.show();

	m_group_linenum.add(m_btn_linenum_display);
	m_group_linenum.show();

	m_group_curline.add(m_btn_curline_highlight);
	m_group_curline.show();

	m_group_margin.add(m_btn_margin_display);
	m_group_margin.add(m_box_margin_pos);
	m_group_margin.show();

	m_group_bracket.add(m_btn_bracket_highlight);
	m_group_bracket.show();
	
	add(m_group_wrap);
	add(m_group_linenum);
	add(m_group_curline);
	add(m_group_margin);
	add(m_group_bracket);
}

void Gobby::PreferencesDialog::View::on_wrap_text_toggled()
{
	m_btn_wrap_words.set_sensitive(m_btn_wrap_text.get_active());
}

void Gobby::PreferencesDialog::View::on_margin_display_toggled()
{
	m_box_margin_pos.set_sensitive(m_btn_margin_display.get_active());
}

Gobby::PreferencesDialog::Appearance::
	Appearance(Gobby::Preferences& preferences):
	m_group_toolbar(_("Toolbar") ),
	m_group_font(_("Font") )
{
	Gtk::ToolbarStyle style = preferences.appearance.toolbar_style;
	const Pango::FontDescription& font = preferences.appearance.font;

	m_cmb_toolbar_style.append_text(_("Show text only") );
	m_cmb_toolbar_style.append_text(_("Show icons only") );
	m_cmb_toolbar_style.append_text(_("Show both icons and text") );
	m_cmb_toolbar_style.append_text(_("Show text besides icons") );
	m_cmb_toolbar_style.show();
	connect_toolbar_style_option(m_cmb_toolbar_style,
	                             preferences.appearance.toolbar_style);

	switch(style)
	{
	case Gtk::TOOLBAR_TEXT: m_cmb_toolbar_style.set_active(0); break;
	case Gtk::TOOLBAR_ICONS: m_cmb_toolbar_style.set_active(1); break;
	case Gtk::TOOLBAR_BOTH: m_cmb_toolbar_style.set_active(2); break;
	case Gtk::TOOLBAR_BOTH_HORIZ: m_cmb_toolbar_style.set_active(3); break;
	default: break; // Avoids compiler warnings
	}

	m_btn_font.set_font_name(font.to_string());
	m_btn_font.show();
	connect_font_option(m_btn_font, preferences.appearance.font);

	m_group_toolbar.add(m_cmb_toolbar_style);
	m_group_toolbar.show();

	m_group_font.add(m_btn_font);
	m_group_font.show();

	add(m_group_toolbar);
	add(m_group_font);
}

Gobby::PreferencesDialog::PreferencesDialog(Gtk::Window& parent,
                                            Preferences& preferences)
 : Gtk::Dialog(_("Preferences"), parent),
   m_preferences(preferences),
   m_page_user(preferences),
   m_page_editor(preferences),
   m_page_view(preferences),
   m_page_appearance(preferences)
{
	m_notebook.append_page(m_page_user, _("User"));
	m_notebook.append_page(m_page_editor, _("Editor"));
	m_notebook.append_page(m_page_view, _("View"));
	m_notebook.append_page(m_page_appearance, _("Appearance"));

	m_page_user.show();
	m_page_editor.show();
	m_page_view.show();
	m_page_appearance.show();

	get_vbox()->set_spacing(6);
	get_vbox()->pack_start(m_notebook, Gtk::PACK_EXPAND_WIDGET);
	m_notebook.show();

	add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);

	set_border_width(12);
}

void Gobby::PreferencesDialog::on_response(int id)
{
	hide();
}
