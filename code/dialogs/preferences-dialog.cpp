/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "dialogs/preferences-dialog.hpp"
#include "util/color.hpp"
#include "util/i18n.hpp"

#include <glibmm/markup.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/scrolledwindow.h>
#include <stdexcept>

#include <gtksourceview/gtksourcestyleschememanager.h>
#include <gtksourceview/gtksourcestylescheme.h>

#include <gnutls/x509.h>

namespace
{
	using namespace Gobby;

	Gtk::Widget* indent(Gtk::Widget& widget)
	{
		Gtk::Alignment* alignment = new Gtk::Alignment;
		alignment->set_padding(0, 0, 12, 0);
		alignment->add(widget);
		alignment->show();
		return Gtk::manage(alignment);
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
	void connect_option(Gtk::Scale& scale,
	                    Preferences::Option<OptionType>& option)
	{
		scale.signal_value_changed().connect(
			sigc::compose(
				sigc::mem_fun(
					option,
					&Preferences::Option<OptionType>::set
				),
				sigc::mem_fun(
					scale,
					&Gtk::Scale::get_value
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
		color_button.property_color().signal_changed().connect(
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

void Gobby::PreferencesDialog::Page::add(Gtk::Widget& widget, bool expand)
{
	m_box.pack_start(widget, expand ? Gtk::PACK_EXPAND_WIDGET : Gtk::PACK_SHRINK);
}

Gobby::PreferencesDialog::User::User(Gtk::Window& parent,
                                     Preferences& preferences):
	m_group_settings(_("Settings")),
	m_group_remote(_("Remote Users")),
	m_group_local(_("Local Documents")),
	m_box_user_name(false, 6),
	m_lbl_user_name(_("User name:"), Gtk::ALIGN_START),
	m_box_user_color(false, 6),
	m_lbl_user_color(_("User color:"), Gtk::ALIGN_START),
	m_box_user_alpha(false, 6),
	m_lbl_user_alpha(_("Color intensity:"), Gtk::ALIGN_START),
	m_scl_user_alpha(0.0, 1.0, 0.025),
	m_btn_user_color(_("Choose a new user color"), parent),
	m_btn_remote_show_cursors(_("Show cursors of remote users")),
	m_btn_remote_show_selections(_("Show selections of remote users")),
	m_btn_remote_show_current_lines(
		_("Highlight current line of remote users")),
	m_btn_remote_show_cursor_positions(
		_("Indicate cursor position of remote users "
		  "in the scrollbar")),
	m_btn_local_allow_connections(
		_("Allow remote users to edit local documents")),
	m_btn_local_require_password(
		_("Require remote users to enter a password")),
	m_lbl_local_password(_("Password:"), Gtk::ALIGN_START),
	m_box_local_password(false, 6),
	m_lbl_local_port(_("Port:"), Gtk::ALIGN_START),
	m_box_local_port(false, 6),
	m_box_local_connections(false, 6),
	m_btn_local_keep_documents(
		_("Remember local documents after Gobby restart")),
	m_lbl_local_documents_directory(_("Local documents directory:")),
	m_btn_local_documents_directory(
		Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER),
	m_conn_local_documents_directory(m_btn_local_documents_directory,
	                                 preferences.user.host_directory),
	m_box_local_documents_directory(false, 6),
	m_size_group(Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL))
{
	m_btn_local_allow_connections.signal_toggled().connect(
		sigc::mem_fun(
			*this, &User::on_local_allow_connections_toggled));
	m_btn_local_require_password.signal_toggled().connect(
		sigc::mem_fun(
			*this, &User::on_local_require_password_toggled));
	m_btn_local_keep_documents.signal_toggled().connect(
		sigc::mem_fun(
			*this, &User::on_local_keep_documents_toggled));

	m_lbl_user_name.show();
	m_ent_user_name.set_text(preferences.user.name);
	m_ent_user_name.show();
	connect_option(m_ent_user_name, preferences.user.name);

	m_box_user_name.pack_start(m_lbl_user_name, Gtk::PACK_SHRINK);
	m_box_user_name.pack_start(m_ent_user_name, Gtk::PACK_SHRINK);
	m_box_user_name.show();

	m_btn_user_color.set_hue(preferences.user.hue);
	m_btn_user_color.set_saturation(0.35);
	m_btn_user_color.set_value(1.0);
	m_btn_user_color.set_size_request(150, -1);
	m_lbl_user_color.show();
	m_btn_user_color.show();
	connect_hue_option(m_btn_user_color, preferences.user.hue);

	m_box_user_color.pack_start(m_lbl_user_color, Gtk::PACK_SHRINK);
	m_box_user_color.pack_start(
		m_btn_user_color, Gtk::PACK_SHRINK);
	m_box_user_color.show();

	m_lbl_user_alpha.show();
	m_scl_user_alpha.set_value(preferences.user.alpha);
	m_scl_user_alpha.set_show_fill_level(false);
	m_scl_user_alpha.set_draw_value(false);
	m_scl_user_alpha.show();
	connect_option(m_scl_user_alpha, preferences.user.alpha);

	m_box_user_alpha.pack_start(m_lbl_user_alpha, Gtk::PACK_SHRINK);
	m_box_user_alpha.pack_start(
		m_scl_user_alpha, Gtk::PACK_EXPAND_WIDGET);
	m_box_user_alpha.show();

	m_group_settings.add(m_box_user_name);
	m_group_settings.add(m_box_user_color);
	m_group_settings.add(m_box_user_alpha);

	m_size_group->add_widget(m_lbl_user_name);
	m_size_group->add_widget(m_lbl_user_color);
	m_size_group->add_widget(m_lbl_user_alpha);
	m_group_settings.show();

	m_btn_remote_show_cursors.set_active(
		preferences.user.show_remote_cursors);
	m_btn_remote_show_cursors.show();
	connect_option(m_btn_remote_show_cursors,
	               preferences.user.show_remote_cursors);

	m_btn_remote_show_selections.set_active(
		preferences.user.show_remote_selections);
	m_btn_remote_show_selections.show();
	connect_option(m_btn_remote_show_selections,
	               preferences.user.show_remote_selections);

	m_btn_remote_show_current_lines.set_active(
		preferences.user.show_remote_current_lines);
	m_btn_remote_show_current_lines.show();
	connect_option(m_btn_remote_show_current_lines,
	               preferences.user.show_remote_current_lines);

	m_btn_remote_show_cursor_positions.set_active(
		preferences.user.show_remote_cursor_positions);
	m_btn_remote_show_cursor_positions.show();
	connect_option(m_btn_remote_show_cursor_positions,
	               preferences.user.show_remote_cursor_positions);

	m_group_remote.add(m_btn_remote_show_cursors);
	m_group_remote.add(m_btn_remote_show_selections);
	m_group_remote.add(m_btn_remote_show_current_lines);
	m_group_remote.add(m_btn_remote_show_cursor_positions);
	m_group_remote.show();

	// TODO: Set mnemonics
	// m_lbl_autosave_interval.set_mnemonic_widget(m_ent_autosave_interval);

	m_btn_local_allow_connections.set_active(
		preferences.user.allow_remote_access);
	m_btn_local_allow_connections.show();
	connect_option(m_btn_local_allow_connections,
	               preferences.user.allow_remote_access);

	m_btn_local_require_password.set_active(
		preferences.user.require_password);
	m_btn_local_require_password.show();
	connect_option(m_btn_local_require_password,
	               preferences.user.require_password);

	m_lbl_local_password.show();
	m_ent_local_password.set_visibility(false);
	m_ent_local_password.set_text(
		static_cast<std::string>(preferences.user.password));
	m_ent_local_password.show();
	connect_option(m_ent_local_password,
	               preferences.user.password);

	m_box_local_password.pack_start(m_lbl_local_password,
	                                Gtk::PACK_SHRINK);
	m_box_local_password.pack_start(m_ent_local_password,
	                                Gtk::PACK_SHRINK);
	m_box_local_password.set_sensitive(preferences.user.require_password);
	m_box_local_password.show();

	m_lbl_local_port.show();
	m_ent_local_port.set_range(1, 65535);
	m_ent_local_port.set_value(preferences.user.port);
	m_ent_local_port.set_increments(1, 1);
	m_ent_local_port.show();
	connect_option(m_ent_local_port, preferences.user.port);

	m_box_local_port.pack_start(m_lbl_local_port,
	                            Gtk::PACK_SHRINK);
	m_box_local_port.pack_start(m_ent_local_port,
	                            Gtk::PACK_SHRINK);
	m_box_local_port.show();

	m_box_local_connections.pack_start(m_btn_local_require_password,
	                                   Gtk::PACK_SHRINK);
	m_box_local_connections.pack_start(*indent(m_box_local_password),
	                                   Gtk::PACK_SHRINK);
	m_box_local_connections.pack_start(m_box_local_port,
	                                   Gtk::PACK_SHRINK);
	m_box_local_connections.set_sensitive(
		preferences.user.allow_remote_access);
	m_box_local_connections.show();

	m_btn_local_keep_documents.set_active(
		preferences.user.keep_local_documents);
	m_btn_local_keep_documents.show();
	connect_option(m_btn_local_keep_documents,
	               preferences.user.keep_local_documents);

	m_lbl_local_documents_directory.show();
	m_btn_local_documents_directory.set_width_chars(20);
	m_btn_local_documents_directory.set_filename(
		static_cast<std::string>(preferences.user.host_directory));
	m_btn_local_documents_directory.show();

	m_box_local_documents_directory.pack_start(
		m_lbl_local_documents_directory,
		Gtk::PACK_SHRINK);
	m_box_local_documents_directory.pack_start(
		m_btn_local_documents_directory,
		Gtk::PACK_SHRINK);
	m_box_local_documents_directory.set_sensitive(
		preferences.user.keep_local_documents);
	m_box_local_documents_directory.show();

	m_group_local.add(m_btn_local_allow_connections);
	m_group_local.add(*indent(m_box_local_connections));
	m_group_local.add(m_btn_local_keep_documents);
	m_group_local.add(*indent(m_box_local_documents_directory));
	m_group_local.show();

	add(m_group_settings, false);
	add(m_group_remote, false);
	add(m_group_local, false);
}

void Gobby::PreferencesDialog::User::on_local_allow_connections_toggled()
{
	m_box_local_connections.set_sensitive(
		m_btn_local_allow_connections.get_active());
}

void Gobby::PreferencesDialog::User::on_local_require_password_toggled()
{
	m_box_local_password.set_sensitive(
		m_btn_local_require_password.get_active());
}

void Gobby::PreferencesDialog::User::on_local_keep_documents_toggled()
{
	m_box_local_documents_directory.set_sensitive(
		m_btn_local_keep_documents.get_active());
}

Gobby::PreferencesDialog::Editor::Editor(Preferences& preferences):
	m_group_tab(_("Tab Stops")),
	m_group_indentation(_("Indentation")),
	m_group_homeend(_("Home/End Behavior")),
	m_group_saving(_("File Saving")),
	m_lbl_tab_width(_("_Tab width:"), Gtk::ALIGN_END,
	                Gtk::ALIGN_CENTER, true),
	m_btn_tab_spaces(_("Insert _spaces instead of tabs"), true),
	m_btn_indentation_auto(_("Enable automatic _indentation"), true),
	m_btn_homeend_smart(_("Smart _home/end"), true),
	m_btn_autosave_enabled(_("Enable _automatic saving of documents"),
	                       true),
	m_lbl_autosave_interval(_("Autosave interval in _minutes:"), true)
{
	unsigned int tab_width = preferences.editor.tab_width;
	bool tab_spaces = preferences.editor.tab_spaces;
	bool indentation_auto = preferences.editor.indentation_auto;
	bool homeend_smart = preferences.editor.homeend_smart;
	unsigned int autosave_enabled = preferences.editor.autosave_enabled;
	unsigned int autosave_interval = preferences.editor.autosave_interval;

	m_btn_autosave_enabled.signal_toggled().connect(
		sigc::mem_fun(*this, &Editor::on_autosave_enabled_toggled));

	m_lbl_tab_width.set_mnemonic_widget(m_ent_tab_width);
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
	m_box_tab_width.pack_start(m_ent_tab_width, Gtk::PACK_SHRINK);
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

	m_btn_autosave_enabled.set_active(autosave_enabled);
	m_btn_autosave_enabled.show();
	connect_option(m_btn_autosave_enabled,
	               preferences.editor.autosave_enabled);

	m_lbl_autosave_interval.set_mnemonic_widget(m_ent_autosave_interval);
	m_lbl_autosave_interval.show();
	m_ent_autosave_interval.set_range(1,60);
	m_ent_autosave_interval.set_value(autosave_interval);
	m_ent_autosave_interval.set_increments(1,10);
	m_ent_autosave_interval.show();
	connect_option(m_ent_autosave_interval,
	               preferences.editor.autosave_interval);

	m_box_autosave_interval.set_spacing(6);
	m_box_autosave_interval.pack_start(m_lbl_autosave_interval,
	                                   Gtk::PACK_SHRINK);
	m_box_autosave_interval.pack_start(m_ent_autosave_interval,
	                                   Gtk::PACK_SHRINK);
	m_box_autosave_interval.set_sensitive(autosave_enabled);
	m_box_autosave_interval.show();

	m_group_tab.add(m_box_tab_width);
	m_group_tab.add(m_btn_tab_spaces);
	m_group_tab.show();

	m_group_indentation.add(m_btn_indentation_auto);
	m_group_indentation.show();

	m_group_homeend.add(m_btn_homeend_smart);
	m_group_homeend.show();

	m_group_saving.add(m_btn_autosave_enabled);
	m_group_saving.add(*indent(m_box_autosave_interval));
	m_group_saving.show();

	add(m_group_tab, false);
	add(m_group_indentation, false);
	add(m_group_homeend, false);
	add(m_group_saving, false);
}

void Gobby::PreferencesDialog::Editor::on_autosave_enabled_toggled()
{
	m_box_autosave_interval.set_sensitive(
		m_btn_autosave_enabled.get_active());
}

Gobby::PreferencesDialog::View::View(Preferences& preferences):
	m_group_wrap(_("Text Wrapping") ),
	m_group_linenum(_("Line Numbers") ),
	m_group_curline(_("Current Line") ),
	m_group_margin(_("Right Margin") ),
	m_group_bracket(_("Bracket Matching") ),
	m_group_spaces(_("Whitespace Display") ),
	m_btn_wrap_text(_("Enable text wrapping") ),
	m_btn_wrap_words(_("Do not split words over two lines") ),
	m_btn_linenum_display(_("Display line numbers") ),
	m_btn_curline_highlight(_("Highlight current line") ),
	m_btn_margin_display(_("Display right margin") ),
	m_lbl_margin_pos(_("Right margin at column:") ),
	m_btn_bracket_highlight(_("Highlight matching bracket") ),
	m_cmb_spaces_display(preferences.view.whitespace_display)
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

	m_lbl_margin_pos.show();
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

	// TODO: When we require a higher version of GtkSourceView, then
	// we should add GTK_SOURCE_DRAW_SPACES_NBSP here.
	const int SOURCE_DRAW_SPACES = GTK_SOURCE_DRAW_SPACES_SPACE;

	m_cmb_spaces_display.add(
		_("Display no whitespace"),
		static_cast<GtkSourceDrawSpacesFlags>(0));
	m_cmb_spaces_display.add(
		_("Display spaces"),
		static_cast<GtkSourceDrawSpacesFlags>(
			SOURCE_DRAW_SPACES));
	m_cmb_spaces_display.add(
		_("Display tabs"),
		static_cast<GtkSourceDrawSpacesFlags>(
			GTK_SOURCE_DRAW_SPACES_TAB));
	m_cmb_spaces_display.add(
		_("Display tabs and spaces"),
		static_cast<GtkSourceDrawSpacesFlags>(
			SOURCE_DRAW_SPACES | GTK_SOURCE_DRAW_SPACES_TAB));
	m_cmb_spaces_display.show();

	m_box_margin_pos.set_spacing(6);
	m_box_margin_pos.set_sensitive(margin_display);
	m_box_margin_pos.pack_start(m_lbl_margin_pos, Gtk::PACK_SHRINK);
	m_box_margin_pos.pack_start(*indent(m_ent_margin_pos),
	                            Gtk::PACK_SHRINK);
	m_box_margin_pos.show();

	m_group_wrap.add(m_btn_wrap_text);
	m_group_wrap.add(*indent(m_btn_wrap_words));
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

	m_group_spaces.add(m_cmb_spaces_display);
	m_group_spaces.show();

	add(m_group_wrap, false);
	add(m_group_linenum, false);
	add(m_group_curline, false);
	add(m_group_margin, false);
	add(m_group_bracket, false);
	add(m_group_spaces, false);
}

void Gobby::PreferencesDialog::View::on_wrap_text_toggled()
{
	m_btn_wrap_words.set_sensitive(m_btn_wrap_text.get_active());
}

void Gobby::PreferencesDialog::View::on_margin_display_toggled()
{
	m_box_margin_pos.set_sensitive(m_btn_margin_display.get_active());
}

Gobby::PreferencesDialog::Appearance::Appearance(Preferences& preferences):
	m_group_toolbar(_("Toolbar") ),
	m_group_font(_("Font") ),
	m_group_scheme(_("Color Scheme")),
	m_cmb_toolbar_style(preferences.appearance.toolbar_style),
	m_conn_font(m_btn_font, preferences.appearance.font),
	m_list(Gtk::ListStore::create(m_columns)),
	m_tree(m_list)
{
	const Pango::FontDescription& font = preferences.appearance.font;

	m_cmb_toolbar_style.add(_("Show text only"),
	                        Gtk::TOOLBAR_TEXT);
	m_cmb_toolbar_style.add(_("Show icons only"),
	                        Gtk::TOOLBAR_ICONS);
	m_cmb_toolbar_style.add(_("Show both icons and text"),
	                        Gtk::TOOLBAR_BOTH );
	m_cmb_toolbar_style.add(_("Show text besides icons"),
	                        Gtk::TOOLBAR_BOTH_HORIZ );
	m_cmb_toolbar_style.show();

	m_conn_font.block();
	m_btn_font.set_font_name(font.to_string());
	m_btn_font.show();
	m_conn_font.unblock();

	m_group_toolbar.add(m_cmb_toolbar_style);
	m_group_toolbar.show();

	m_group_font.add(m_btn_font);
	m_group_font.show();

	Gtk::TreeViewColumn column_name;
	Gtk::CellRendererText renderer_name;
	column_name.pack_start(renderer_name, false);
	column_name.add_attribute(renderer_name.property_text(), m_columns.name);

	m_tree.append_column(column_name);//"Scheme Name", m_columns.name);
	m_tree.append_column("Scheme description", m_columns.description);

	Pango::AttrList list;
	Pango::Attribute attr(Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD));
	list.insert(attr);
	renderer_name.property_attributes() = list;

	m_tree.set_headers_visible(false);
	m_tree.show();

	Gtk::ScrolledWindow* scroll = Gtk::manage(new Gtk::ScrolledWindow);
	scroll->set_shadow_type(Gtk::SHADOW_IN);
	scroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	scroll->add(m_tree);
	scroll->show();

	m_group_scheme.add(*scroll);
	m_group_scheme.show();

	GtkSourceStyleSchemeManager* manager = gtk_source_style_scheme_manager_get_default();
	const gchar* const* ids = gtk_source_style_scheme_manager_get_scheme_ids(manager);

	Glib::ustring current_scheme = preferences.appearance.scheme_id;

	for (const gchar* const* id = ids; *id != NULL; ++id)
	{
		GtkSourceStyleScheme* scheme = gtk_source_style_scheme_manager_get_scheme(manager, *id);
		const gchar* name = gtk_source_style_scheme_get_name(scheme);
		const gchar* desc = gtk_source_style_scheme_get_description(scheme);

		Gtk::TreeIter iter = m_list->append();
		(*iter)[m_columns.name] = name;
		(*iter)[m_columns.description] = desc;
		(*iter)[m_columns.scheme] = scheme;

		if (current_scheme == gtk_source_style_scheme_get_id(scheme))
			m_tree.get_selection()->select(iter);
	}

	m_tree.get_selection()->signal_changed().connect(
		sigc::bind(
			sigc::mem_fun(*this, &Appearance::on_scheme_changed),
			sigc::ref(preferences)));

	m_list->set_sort_column(m_columns.name, Gtk::SORT_ASCENDING);

	add(m_group_toolbar, false);
	add(m_group_font, false);
	add(m_group_scheme, true);
}

void Gobby::PreferencesDialog::Appearance::on_scheme_changed(Preferences& preferences)
{
	Gtk::TreeIter iter = m_tree.get_selection()->get_selected();
	GtkSourceStyleScheme* scheme = (*iter)[m_columns.scheme];
	preferences.appearance.scheme_id = gtk_source_style_scheme_get_id(scheme);
}

Gobby::PreferencesDialog::Security::Security(Gtk::Window& parent,
                                             FileChooser& file_chooser,
                                             Preferences& preferences,
                                             CertificateManager& cert_manager):
	m_preferences(preferences), m_parent(parent), m_file_chooser(file_chooser),
	m_cert_manager(cert_manager),
	m_group_trust_file(_("Trusted CAs")),
	m_group_connection_policy(_("Secure Connection")),
	m_group_authentication(_("Authentication")),
	m_btn_use_system_trust(_("Trust this computer's default CAs")),
	m_lbl_trust_file(_("Additionally Trusted CAs:")),
	m_btn_path_trust_file(_("Select Additionally Trusted CAs")),
	m_conn_path_trust_file(m_btn_path_trust_file,
	                       preferences.security.trusted_cas),
	m_box_trust_file(false, 6),
	m_error_trust_file("", Gtk::ALIGN_START),
	m_cmb_connection_policy(preferences.security.policy),
	m_btn_auth_none(_("None")),
	m_btn_auth_cert(_("Use a certificate")),
	m_lbl_key_file(_("Private key:"), Gtk::ALIGN_START),
	m_btn_key_file(_("Select a private key file")),
	m_conn_path_key_file(m_btn_key_file, preferences.security.key_file),
	m_btn_key_file_create(_("Create New...")),
	m_box_key_file(false, 6),
	m_error_key_file("", Gtk::ALIGN_START),
	m_lbl_cert_file(_("Certificate:"), Gtk::ALIGN_START),
	m_btn_cert_file(_("Select a certificate file")),
	m_conn_path_cert_file(m_btn_cert_file,
	                      preferences.security.certificate_file),
	m_btn_cert_file_create(_("Create New...")),
	m_box_cert_file(false, 6),
	m_error_cert_file("", Gtk::ALIGN_START),
	m_size_group(Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL)),
	m_key_generator_handle(NULL), m_cert_generator_handle(NULL)
{
	m_cert_manager.signal_credentials_changed().connect(
		sigc::mem_fun(*this, &Security::on_credentials_changed));
	m_btn_auth_cert.signal_toggled().connect(
		sigc::mem_fun(*this, &Security::on_auth_cert_toggled));

	m_btn_use_system_trust.set_active(
		m_preferences.security.use_system_trust);
	m_btn_use_system_trust.signal_toggled().connect(
		sigc::mem_fun(*this, &Security::on_use_system_trust_toggled));
	m_btn_use_system_trust.show();

	m_lbl_trust_file.show();
	m_btn_path_trust_file.set_width_chars(20);
	const std::string& trust_file = preferences.security.trusted_cas;
	if(!trust_file.empty())
		m_btn_path_trust_file.set_filename(trust_file);
	m_btn_path_trust_file.show();

	m_box_trust_file.pack_start(m_lbl_trust_file, Gtk::PACK_SHRINK);
	m_box_trust_file.pack_start(m_btn_path_trust_file,
	                            Gtk::PACK_EXPAND_WIDGET);
	m_box_trust_file.show();

	m_group_trust_file.add(m_btn_use_system_trust);
	m_group_trust_file.add(m_box_trust_file);
	m_group_trust_file.add(m_error_trust_file);
	m_group_trust_file.show();

/*	m_cmb_connection_policy.add(
		_("Never use a secure connection"),
		INF_XMPP_CONNECTION_SECURITY_ONLY_UNSECURED);*/
	m_cmb_connection_policy.add(
		_("Use TLS if possible"),
		INF_XMPP_CONNECTION_SECURITY_BOTH_PREFER_TLS);
	m_cmb_connection_policy.add(
		_("Always use TLS"),
		INF_XMPP_CONNECTION_SECURITY_ONLY_TLS);
	m_cmb_connection_policy.show();
	m_group_connection_policy.add(m_cmb_connection_policy);
	m_group_connection_policy.show();

	Gtk::RadioButton::Group group;
	m_btn_auth_none.set_group(group);
	m_btn_auth_none.show();
	m_btn_auth_cert.set_group(group);
	m_btn_auth_cert.show();
	if(preferences.security.authentication_enabled)
		m_btn_auth_cert.set_active(true);
	else
		m_btn_auth_none.set_active(true);
	connect_option(m_btn_auth_cert,
	               preferences.security.authentication_enabled);

	m_lbl_key_file.show();
	m_btn_key_file.set_width_chars(20);
	const std::string& key_file = preferences.security.key_file;
	if(!key_file.empty())
	{
		m_conn_path_key_file.block();
		m_btn_key_file.set_filename(key_file);
		m_conn_path_key_file.unblock();
	}
	m_btn_key_file.show();

	m_btn_key_file_create.signal_clicked().connect(
		sigc::mem_fun(*this, &Security::on_create_key_clicked));
	m_btn_key_file_create.show();

	m_box_key_file.pack_start(m_lbl_key_file, Gtk::PACK_SHRINK);
	m_box_key_file.pack_start(m_btn_key_file, Gtk::PACK_SHRINK);
	m_box_key_file.pack_start(m_btn_key_file_create, Gtk::PACK_SHRINK);
	m_box_key_file.set_sensitive(
		preferences.security.authentication_enabled);
	m_box_key_file.show();

	m_lbl_cert_file.show();
	m_btn_cert_file.set_width_chars(20);
	const std::string& cert_file = preferences.security.certificate_file;
	if(!cert_file.empty())
	{
		m_conn_path_cert_file.block();
		m_btn_cert_file.set_filename(cert_file);
		m_conn_path_cert_file.unblock();
	}
	m_btn_cert_file.show();

	m_btn_cert_file_create.signal_clicked().connect(
		sigc::mem_fun(*this, &Security::on_create_cert_clicked));
	m_btn_cert_file_create.show();

	m_box_cert_file.pack_start(m_lbl_cert_file, Gtk::PACK_SHRINK);
	m_box_cert_file.pack_start(m_btn_cert_file, Gtk::PACK_SHRINK);
	m_box_cert_file.pack_start(m_btn_cert_file_create, Gtk::PACK_SHRINK);
	m_box_cert_file.set_sensitive(
		preferences.security.authentication_enabled);
	m_box_cert_file.show();

	m_size_group->add_widget(m_lbl_key_file);
	m_size_group->add_widget(m_lbl_cert_file);

	m_group_authentication.add(m_btn_auth_none);
	m_group_authentication.add(m_btn_auth_cert);
	m_group_authentication.add(*indent(m_box_key_file));
	m_group_authentication.add(*indent(m_error_key_file));
	m_group_authentication.add(*indent(m_box_cert_file));
	m_group_authentication.add(*indent(m_error_cert_file));
	m_group_authentication.show();

	on_credentials_changed();

	add(m_group_trust_file, false);
	add(m_group_connection_policy, false);
	add(m_group_authentication, false);
}

void Gobby::PreferencesDialog::Security::set_file_error(Gtk::Label& label,
                                                        const GError* error)
{
	if(error != NULL)
	{
		label.set_markup(
			//"<span style='color: red;'>" +
			"<span foreground='red'>" +
			std::string(_("Error reading file:")) + " " +
			Glib::Markup::escape_text(error->message) +
			"</span>");
		label.show();
	}
	else
	{
		label.hide();
	}
}

void Gobby::PreferencesDialog::Security::on_use_system_trust_toggled()
{
	m_preferences.security.use_system_trust =
		m_btn_use_system_trust.get_active();
}

void Gobby::PreferencesDialog::Security::on_credentials_changed()
{
	set_file_error(m_error_trust_file,
	               m_cert_manager.get_trust_error());

	if(m_key_generator_handle.get() == NULL)
	{
		set_file_error(m_error_key_file,
		               m_cert_manager.get_key_error());
	}
	else
	{
		m_error_key_file.set_text(
			_("2048-bit RSA private key is being "
			  "generated, please wait..."));
		m_error_key_file.show();
	}

	if(m_cert_generator_handle.get() == NULL)
		set_file_error(m_error_cert_file,
		               m_cert_manager.get_certificate_error());

	const bool operation_in_progress =
		m_key_generator_handle.get() != NULL ||
		m_cert_generator_handle.get() != NULL;

	m_btn_key_file.set_sensitive(!operation_in_progress);
	m_btn_key_file_create.set_sensitive(!operation_in_progress);
	m_btn_cert_file.set_sensitive(!operation_in_progress);

	m_btn_cert_file_create.set_sensitive(
		!operation_in_progress &&
		m_cert_manager.get_private_key() != NULL);
}

void Gobby::PreferencesDialog::Security::on_auth_cert_toggled()
{
	m_box_key_file.set_sensitive(m_btn_auth_cert.get_active());
	m_box_cert_file.set_sensitive(m_btn_auth_cert.get_active());
}

void Gobby::PreferencesDialog::Security::on_create_key_clicked()
{
	m_file_dialog.reset(new FileChooser::Dialog(
		m_file_chooser, m_parent,
		_("Select a location for the generated key"),
		Gtk::FILE_CHOOSER_ACTION_SAVE));

	const std::string& key_file = m_preferences.security.key_file;
	if(!key_file.empty())
		m_file_dialog->set_filename(key_file);

	m_file_dialog->signal_response().connect(
		sigc::mem_fun(
			*this, &Security::on_file_dialog_response_key));

	m_file_dialog->present();
}

void Gobby::PreferencesDialog::Security::on_create_cert_clicked()
{
	m_file_dialog.reset(new FileChooser::Dialog(
		m_file_chooser, m_parent,
		_("Select a location for the generated certificate"),
		Gtk::FILE_CHOOSER_ACTION_SAVE));

	const std::string& certificate_file =
		m_preferences.security.certificate_file;
	if(!certificate_file.empty())
		m_file_dialog->set_filename(certificate_file);

	m_file_dialog->signal_response().connect(
		sigc::mem_fun(
			*this,
			&Security::on_file_dialog_response_certificate));

	m_file_dialog->present();
}

void Gobby::PreferencesDialog::Security::on_file_dialog_response_key(
	int response_id)
{
	const std::string filename = m_file_dialog->get_filename();

	m_file_dialog.reset(NULL);

	if(response_id == Gtk::RESPONSE_ACCEPT)
	{
		m_key_generator_handle = create_key(
			GNUTLS_PK_RSA,
			2048,
			sigc::bind(
				sigc::mem_fun(
					*this,
					&Security::on_key_generated),
				filename));

		on_credentials_changed();
	}
}

void Gobby::PreferencesDialog::Security::on_file_dialog_response_certificate(
	int response_id)
{
	const std::string filename = m_file_dialog->get_filename();

	m_file_dialog.reset(NULL);

	if(response_id == Gtk::RESPONSE_ACCEPT &&
	   m_cert_manager.get_private_key() != NULL)
	{
		m_cert_generator_handle = create_self_signed_certificate(
			m_cert_manager.get_private_key(),
			sigc::bind(
				sigc::mem_fun(
					*this,
					&Security::on_cert_generated),
				filename));

		on_credentials_changed();
	}
}

void Gobby::PreferencesDialog::Security::on_key_generated(
	const KeyGeneratorHandle* handle,
	gnutls_x509_privkey_t key,
	const GError* error,
	const std::string& filename)
{
	m_key_generator_handle.reset(NULL);

	m_cert_manager.set_private_key(key, filename.c_str(), error);
}

void Gobby::PreferencesDialog::Security::on_cert_generated(
	const CertificateGeneratorHandle* hndl,
	gnutls_x509_crt_t cert,
	const GError* error,
	const std::string& filename)
{
	m_cert_generator_handle.reset(NULL);

	if(cert != NULL)
	{
		// Note this needs to be allocated with g_malloc, since it
		// is directly passed and freed by libinfinity
		gnutls_x509_crt_t* crt_array =
			static_cast<gnutls_x509_crt_t*>(
				g_malloc(sizeof(gnutls_x509_crt_t)));
		crt_array[0] = cert;

		m_cert_manager.set_certificates(
			crt_array, 1, filename.c_str(), error);
	}
	else
	{
		m_cert_manager.set_certificates(
			NULL, 0, filename.c_str(), error);
	}
}

Gobby::PreferencesDialog::PreferencesDialog(Gtk::Window& parent,
                                            FileChooser& file_chooser,
                                            Preferences& preferences,
	                                    CertificateManager& cert_manager):
	Gtk::Dialog(_("Preferences"), parent), m_preferences(preferences),
	m_page_user(*this, preferences), m_page_editor(preferences),
	m_page_view(preferences), m_page_appearance(preferences),
	m_page_security(*this, file_chooser, preferences, cert_manager)
{
	m_notebook.append_page(m_page_user, _("User"));
	m_notebook.append_page(m_page_editor, _("Editor"));
	m_notebook.append_page(m_page_view, _("View"));
	m_notebook.append_page(m_page_appearance, _("Appearance"));
	m_notebook.append_page(m_page_security, _("Security"));

	m_page_user.show();
	m_page_editor.show();
	m_page_view.show();
	m_page_appearance.show();
	m_page_security.show();

	get_vbox()->set_spacing(6);
	get_vbox()->pack_start(m_notebook, Gtk::PACK_EXPAND_WIDGET);
	m_notebook.show();

	add_button(_("_Close"), Gtk::RESPONSE_CLOSE);

	set_border_width(12);
	set_resizable(false);
}

void Gobby::PreferencesDialog::on_response(int id)
{
	hide();
}
