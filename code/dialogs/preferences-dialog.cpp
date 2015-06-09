/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
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

Gobby::PreferencesDialog::User::User(
	const Glib::RefPtr<Gtk::Builder>& builder,
	Preferences& preferences)
{
	builder->get_widget("user-name", m_ent_user_name);
	builder->get_widget_derived("user-color", m_btn_user_color);
	builder->get_widget("color-intensity", m_scl_user_alpha);
	builder->get_widget("remote-show-cursors", m_btn_remote_show_cursors);
	builder->get_widget("remote-show-selections",
	                    m_btn_remote_show_selections);
	builder->get_widget("remote-show-current-line",
	                    m_btn_remote_show_current_lines);
	builder->get_widget("remote-show-in-scrollbar",
	                    m_btn_remote_show_cursor_positions);
	builder->get_widget("grid-local-connections",
	                    m_grid_local_connections);
	builder->get_widget("remote-allow-edit",
	                    m_btn_local_allow_connections);
	builder->get_widget("require-password",
	                    m_btn_local_require_password);
	builder->get_widget("grid-password", m_grid_password);
	builder->get_widget("password-entry", m_ent_local_password);
	builder->get_widget("port-number", m_ent_local_port);
	builder->get_widget("remember-local-documents",
	                    m_btn_local_keep_documents);
	builder->get_widget("grid-local-documents-directory",
	                    m_grid_local_documents_directory);
	builder->get_widget("local-documents-directory",
	                    m_btn_local_documents_directory);

	m_conn_local_documents_directory.reset(new PathConnection(
		*m_btn_local_documents_directory,
		preferences.user.host_directory));

	m_btn_local_allow_connections->signal_toggled().connect(
		sigc::mem_fun(
			*this, &User::on_local_allow_connections_toggled));
	m_btn_local_require_password->signal_toggled().connect(
		sigc::mem_fun(
			*this, &User::on_local_require_password_toggled));
	m_btn_local_keep_documents->signal_toggled().connect(
		sigc::mem_fun(
			*this, &User::on_local_keep_documents_toggled));

	m_ent_user_name->set_text(preferences.user.name);
	connect_option(*m_ent_user_name, preferences.user.name);

	m_btn_user_color->set_hue(preferences.user.hue);
	m_btn_user_color->set_saturation(0.35);
	m_btn_user_color->set_value(1.0);
	connect_hue_option(*m_btn_user_color, preferences.user.hue);

	m_scl_user_alpha->set_range(0.0, 1.0);
	m_scl_user_alpha->set_increments(0.0025, 0.1);
	m_scl_user_alpha->set_value(preferences.user.alpha);
	connect_option(*m_scl_user_alpha, preferences.user.alpha);

	m_btn_remote_show_cursors->set_active(
		preferences.user.show_remote_cursors);
	connect_option(*m_btn_remote_show_cursors,
	               preferences.user.show_remote_cursors);

	m_btn_remote_show_selections->set_active(
		preferences.user.show_remote_selections);
	connect_option(*m_btn_remote_show_selections,
	               preferences.user.show_remote_selections);

	m_btn_remote_show_current_lines->set_active(
		preferences.user.show_remote_current_lines);
	connect_option(*m_btn_remote_show_current_lines,
	               preferences.user.show_remote_current_lines);

	m_btn_remote_show_cursor_positions->set_active(
		preferences.user.show_remote_cursor_positions);
	connect_option(*m_btn_remote_show_cursor_positions,
	               preferences.user.show_remote_cursor_positions);

	m_btn_local_allow_connections->set_active(
		preferences.user.allow_remote_access);
	connect_option(*m_btn_local_allow_connections,
	               preferences.user.allow_remote_access);

	m_btn_local_require_password->set_active(
		preferences.user.require_password);
	connect_option(*m_btn_local_require_password,
	               preferences.user.require_password);

	m_ent_local_password->set_text(
		static_cast<std::string>(preferences.user.password));
	connect_option(*m_ent_local_password,
	               preferences.user.password);

	m_ent_local_port->set_range(1, 65535);
	m_ent_local_port->set_value(preferences.user.port);
	m_ent_local_port->set_increments(1, 1);
	connect_option(*m_ent_local_port, preferences.user.port);

	m_btn_local_keep_documents->set_active(
		preferences.user.keep_local_documents);
	connect_option(*m_btn_local_keep_documents,
	               preferences.user.keep_local_documents);

	m_btn_local_documents_directory->set_filename(
		static_cast<std::string>(preferences.user.host_directory));

	// Initial sensitivity
	on_local_allow_connections_toggled();
	on_local_require_password_toggled();
	on_local_keep_documents_toggled();
}

void Gobby::PreferencesDialog::User::on_local_allow_connections_toggled()
{
	m_grid_local_connections->set_sensitive(
		m_btn_local_allow_connections->get_active());
}

void Gobby::PreferencesDialog::User::on_local_require_password_toggled()
{
	m_grid_password->set_sensitive(
		m_btn_local_require_password->get_active());
}

void Gobby::PreferencesDialog::User::on_local_keep_documents_toggled()
{
	m_grid_local_documents_directory->set_sensitive(
		m_btn_local_keep_documents->get_active());
}

Gobby::PreferencesDialog::Editor::Editor(
	const Glib::RefPtr<Gtk::Builder>& builder,
	Preferences& preferences)
{
	builder->get_widget("tab-width", m_ent_tab_width);
	builder->get_widget("insert-spaces", m_btn_tab_spaces);
	builder->get_widget("automatic-indentation", m_btn_indentation_auto);
	builder->get_widget("smart-home-end", m_btn_homeend_smart);
	builder->get_widget("enable-autosave", m_btn_autosave_enabled);
	builder->get_widget("grid-autosave-interval",
	                    m_grid_autosave_interval);
	builder->get_widget("autosave-interval", m_ent_autosave_interval);

	const unsigned int tab_width = preferences.editor.tab_width;
	const bool tab_spaces = preferences.editor.tab_spaces;
	const bool indentation_auto = preferences.editor.indentation_auto;
	const bool homeend_smart = preferences.editor.homeend_smart;
	const unsigned int autosave_enabled =
		preferences.editor.autosave_enabled;
	const unsigned int autosave_interval =
		preferences.editor.autosave_interval;

	m_btn_autosave_enabled->signal_toggled().connect(
		sigc::mem_fun(*this, &Editor::on_autosave_enabled_toggled));

	m_ent_tab_width->set_range(1, 8);
	m_ent_tab_width->set_value(tab_width);
	m_ent_tab_width->set_increments(1, 1);
	connect_option(*m_ent_tab_width, preferences.editor.tab_width);

	m_btn_tab_spaces->set_active(tab_spaces);
	connect_option(*m_btn_tab_spaces, preferences.editor.tab_spaces);

	m_btn_indentation_auto->set_active(indentation_auto);
	connect_option(*m_btn_indentation_auto,
	               preferences.editor.indentation_auto);

	m_btn_homeend_smart->set_active(homeend_smart);
	connect_option(*m_btn_homeend_smart,
	               preferences.editor.homeend_smart);

	m_btn_autosave_enabled->set_active(autosave_enabled);
	connect_option(*m_btn_autosave_enabled,
	               preferences.editor.autosave_enabled);

	m_ent_autosave_interval->set_range(1, 60);
	m_ent_autosave_interval->set_value(autosave_interval);
	m_ent_autosave_interval->set_increments(1, 10);
	connect_option(*m_ent_autosave_interval,
	               preferences.editor.autosave_interval);

	// Initial sensitivity
	on_autosave_enabled_toggled();
}

void Gobby::PreferencesDialog::Editor::on_autosave_enabled_toggled()
{
	m_grid_autosave_interval->set_sensitive(
		m_btn_autosave_enabled->get_active());
}

Gobby::PreferencesDialog::View::View(
	const Glib::RefPtr<Gtk::Builder>& builder,Preferences& preferences)
{
	builder->get_widget("enable-wrapping", m_btn_wrap_text);
	builder->get_widget("do-not-split-words", m_btn_wrap_words);
	builder->get_widget("display-line-numbers", m_btn_linenum_display);
	builder->get_widget("highlight-current-line",
	                    m_btn_curline_highlight);
	builder->get_widget("display-right-margin", m_btn_margin_display);
	builder->get_widget("grid-margin-position", m_grid_margin_pos);
	builder->get_widget("right-margin-column", m_ent_margin_pos);
	builder->get_widget("highlight-matching-brackets",
	                    m_btn_bracket_highlight);
	builder->get_widget_derived("display-whitespace",
	                            m_cmb_spaces_display);

	const Gtk::WrapMode mode = preferences.view.wrap_mode;
	const bool linenum_display = preferences.view.linenum_display;
	const bool curline_highlight = preferences.view.curline_highlight;
	const bool margin_display = preferences.view.margin_display;
	const unsigned int margin_pos = preferences.view.margin_pos;
	const bool bracket_highlight = preferences.view.bracket_highlight;

	m_btn_margin_display->signal_toggled().connect(
		sigc::mem_fun(*this, &View::on_margin_display_toggled));
	m_btn_wrap_text->signal_toggled().connect(
		sigc::mem_fun(*this, &View::on_wrap_text_toggled));

	m_ent_margin_pos->set_range(1, 1024);
	m_ent_margin_pos->set_value(margin_pos);
	m_ent_margin_pos->set_increments(1, 16);
	connect_option(*m_ent_margin_pos, preferences.view.margin_pos);

	m_btn_wrap_text->set_active(mode != Gtk::WRAP_NONE);
	m_btn_wrap_words->set_active(mode == Gtk::WRAP_WORD_CHAR);
	connect_wrap_option(*m_btn_wrap_text, *m_btn_wrap_words,
                            preferences.view.wrap_mode);

	m_btn_linenum_display->set_active(linenum_display);
	connect_option(*m_btn_linenum_display,
	               preferences.view.linenum_display);

	m_btn_curline_highlight->set_active(curline_highlight);
	connect_option(*m_btn_curline_highlight,
	               preferences.view.curline_highlight);

	m_btn_margin_display->set_active(margin_display);
	connect_option(*m_btn_margin_display,
	               preferences.view.margin_display);

	m_btn_bracket_highlight->set_active(bracket_highlight);
	connect_option(*m_btn_bracket_highlight,
	               preferences.view.bracket_highlight);

	// TODO: When we require a higher version of GtkSourceView, then
	// we should add GTK_SOURCE_DRAW_SPACES_NBSP here.
	const int SOURCE_DRAW_SPACES = GTK_SOURCE_DRAW_SPACES_SPACE;

	m_cmb_spaces_display->set_option(preferences.view.whitespace_display);
	m_cmb_spaces_display->add(
		_("Display no whitespace"),
		static_cast<GtkSourceDrawSpacesFlags>(0));
	m_cmb_spaces_display->add(
		_("Display spaces"),
		static_cast<GtkSourceDrawSpacesFlags>(
			SOURCE_DRAW_SPACES));
	m_cmb_spaces_display->add(
		_("Display tabs"),
		static_cast<GtkSourceDrawSpacesFlags>(
			GTK_SOURCE_DRAW_SPACES_TAB));
	m_cmb_spaces_display->add(
		_("Display tabs and spaces"),
		static_cast<GtkSourceDrawSpacesFlags>(
			SOURCE_DRAW_SPACES | GTK_SOURCE_DRAW_SPACES_TAB));

	// Initial sensitivity
	on_wrap_text_toggled();
	on_margin_display_toggled();
}

void Gobby::PreferencesDialog::View::on_wrap_text_toggled()
{
	m_btn_wrap_words->set_sensitive(m_btn_wrap_text->get_active());
}

void Gobby::PreferencesDialog::View::on_margin_display_toggled()
{
	m_grid_margin_pos->set_sensitive(m_btn_margin_display->get_active());
}

Gobby::PreferencesDialog::Appearance::Appearance(
	const Glib::RefPtr<Gtk::Builder>& builder,
	Preferences& preferences)
:
	m_scheme_list(Gtk::ListStore::create(m_scheme_columns))
{
	builder->get_widget_derived("toolbar-style", m_cmb_toolbar_style);
	builder->get_widget("font", m_btn_font);
	builder->get_widget("color-scheme-treeview", m_scheme_tree);

	const Pango::FontDescription& font = preferences.appearance.font;

	m_cmb_toolbar_style->set_option(preferences.appearance.toolbar_style);
	m_cmb_toolbar_style->add(_("Show text only"),
	                         Gtk::TOOLBAR_TEXT);
	m_cmb_toolbar_style->add(_("Show icons only"),
	                         Gtk::TOOLBAR_ICONS);
	m_cmb_toolbar_style->add(_("Show both icons and text"),
	                         Gtk::TOOLBAR_BOTH );
	m_cmb_toolbar_style->add(_("Show text besides icons"),
	                         Gtk::TOOLBAR_BOTH_HORIZ );

	m_btn_font->set_font_name(font.to_string());
	m_conn_font.reset(new FontConnection(
		*m_btn_font, preferences.appearance.font));

	Gtk::TreeViewColumn column_name;
	Gtk::CellRendererText renderer_name;
	column_name.pack_start(renderer_name, false);
	column_name.add_attribute(renderer_name.property_text(),
	                          m_scheme_columns.name);

	Pango::AttrList list;
	Pango::Attribute attr(Pango::Attribute::create_attr_weight(
		Pango::WEIGHT_BOLD));
	list.insert(attr);
	renderer_name.property_attributes() = list;

	m_scheme_tree->append_column(column_name);
	m_scheme_tree->append_column(_("Scheme Description"),
	                             m_scheme_columns.description);
	m_scheme_tree->set_model(m_scheme_list);

	// Populate scheme list
	GtkSourceStyleSchemeManager* manager =
		gtk_source_style_scheme_manager_get_default();
	const gchar* const* ids =
		gtk_source_style_scheme_manager_get_scheme_ids(manager);

	const Glib::ustring current_scheme = preferences.appearance.scheme_id;

	for (const gchar* const* id = ids; *id != NULL; ++id)
	{
		GtkSourceStyleScheme* scheme =
			gtk_source_style_scheme_manager_get_scheme(
				manager, *id);
		const gchar* name =
			gtk_source_style_scheme_get_name(scheme);
		const gchar* desc =
			gtk_source_style_scheme_get_description(scheme);

		Gtk::TreeIter iter = m_scheme_list->append();
		(*iter)[m_scheme_columns.name] = name;
		(*iter)[m_scheme_columns.description] = desc;
		(*iter)[m_scheme_columns.scheme] = scheme;

		if (current_scheme == gtk_source_style_scheme_get_id(scheme))
			m_scheme_tree->get_selection()->select(iter);
	}

	m_scheme_tree->get_selection()->signal_changed().connect(
		sigc::bind(
			sigc::mem_fun(*this, &Appearance::on_scheme_changed),
			sigc::ref(preferences)));

	m_scheme_list->set_sort_column(m_scheme_columns.name,
	                               Gtk::SORT_ASCENDING);
}

void Gobby::PreferencesDialog::Appearance::on_scheme_changed(
	Preferences& preferences)
{
	Gtk::TreeIter iter = m_scheme_tree->get_selection()->get_selected();
	GtkSourceStyleScheme* scheme = (*iter)[m_scheme_columns.scheme];

	preferences.appearance.scheme_id =
		gtk_source_style_scheme_get_id(scheme);
}

Gobby::PreferencesDialog::Security::Security(
	const Glib::RefPtr<Gtk::Builder>& builder,
	FileChooser& file_chooser, Preferences& preferences,
	CertificateManager& cert_manager)
:
	m_preferences(preferences),
	m_file_chooser(file_chooser),
	m_cert_manager(cert_manager)
{
	builder->get_widget("trust-default-cas", m_btn_use_system_trust);
	builder->get_widget("additionally-trusted-cas", m_btn_path_trust_file);
	builder->get_widget("ca-error-message", m_error_trust_file);
	builder->get_widget_derived("secure-connection",
	                            m_cmb_connection_policy);
	builder->get_widget("authentication-none", m_btn_auth_none);
	builder->get_widget("authentication-certificate", m_btn_auth_cert);
	builder->get_widget("grid-auth-certificate", m_grid_auth_cert);

	builder->get_widget("private-key-file", m_btn_key_file);
	builder->get_widget("create-private-key", m_btn_key_file_create);
	builder->get_widget("key-error-message", m_error_key_file);

	builder->get_widget("certificate-file", m_btn_cert_file);
	builder->get_widget("create-certificate", m_btn_cert_file_create);
	builder->get_widget("cert-error-message", m_error_cert_file);

	m_cert_manager.signal_credentials_changed().connect(
		sigc::mem_fun(*this, &Security::on_credentials_changed));
	m_btn_auth_cert->signal_toggled().connect(
		sigc::mem_fun(*this, &Security::on_auth_cert_toggled));

	m_btn_use_system_trust->set_active(
		m_preferences.security.use_system_trust);
	m_btn_use_system_trust->signal_toggled().connect(
		sigc::mem_fun(*this, &Security::on_use_system_trust_toggled));

	const std::string& trust_file = preferences.security.trusted_cas;
	if(!trust_file.empty())
		m_btn_path_trust_file->set_filename(trust_file);
	m_conn_path_trust_file.reset(new PathConnection(
		*m_btn_path_trust_file, preferences.security.trusted_cas));

	m_cmb_connection_policy->set_option(preferences.security.policy);
/*	m_cmb_connection_policy->add(
		_("Never use a secure connection"),
		INF_XMPP_CONNECTION_SECURITY_ONLY_UNSECURED);*/
	m_cmb_connection_policy->add(
		_("Use TLS if possible"),
		INF_XMPP_CONNECTION_SECURITY_BOTH_PREFER_TLS);
	m_cmb_connection_policy->add(
		_("Always use TLS"),
		INF_XMPP_CONNECTION_SECURITY_ONLY_TLS);

	if(preferences.security.authentication_enabled)
		m_btn_auth_cert->set_active(true);
	else
		m_btn_auth_none->set_active(true);
	connect_option(*m_btn_auth_cert,
	               preferences.security.authentication_enabled);

	const std::string& key_file = preferences.security.key_file;
	if(!key_file.empty())
		m_btn_key_file->set_filename(key_file);
	m_conn_path_key_file.reset(new PathConnection(
		*m_btn_key_file, preferences.security.key_file));
	m_btn_key_file_create->signal_clicked().connect(
		sigc::mem_fun(*this, &Security::on_create_key_clicked));

	const std::string& cert_file = preferences.security.certificate_file;
	if(!cert_file.empty())
		m_btn_cert_file->set_filename(cert_file);
	m_conn_path_cert_file.reset(new PathConnection(
		*m_btn_cert_file, preferences.security.certificate_file));
	m_btn_cert_file_create->signal_clicked().connect(
		sigc::mem_fun(*this, &Security::on_create_cert_clicked));

	// Initial sensitivity and content
	on_auth_cert_toggled();
	on_credentials_changed();
}

void Gobby::PreferencesDialog::Security::set_file_error(Gtk::Label& label,
                                                        const GError* error)
{
	if(error != NULL)
	{
		label.set_text(Glib::ustring::compose(
			_("Error reading file: %1"), error->message));
		label.show();
	}
	else
	{
		label.hide();
	}
}

Gtk::Window&
Gobby::PreferencesDialog::Security::get_toplevel(Gtk::Widget& widget)
{
	Gtk::Window* parent = NULL;
	Gtk::Widget* toplevel_widget = widget.get_toplevel();
	if(gtk_widget_is_toplevel(toplevel_widget->gobj()))
		parent = dynamic_cast<Gtk::Window*>(toplevel_widget);

	g_assert(parent != NULL);
	return *parent;
}

void Gobby::PreferencesDialog::Security::on_use_system_trust_toggled()
{
	m_preferences.security.use_system_trust =
		m_btn_use_system_trust->get_active();
}

void Gobby::PreferencesDialog::Security::on_credentials_changed()
{
	set_file_error(*m_error_trust_file,
	               m_cert_manager.get_trust_error());

	if(m_key_generator_handle.get() == NULL)
	{
		set_file_error(*m_error_key_file,
		               m_cert_manager.get_key_error());
	}
	else
	{
		m_error_key_file->set_text(
			_("2048-bit RSA private key is being "
			  "generated, please wait..."));
		m_error_key_file->show();
	}

	if(m_cert_generator_handle.get() == NULL)
		set_file_error(*m_error_cert_file,
		               m_cert_manager.get_certificate_error());

	const bool operation_in_progress =
		m_key_generator_handle.get() != NULL ||
		m_cert_generator_handle.get() != NULL;

	m_btn_key_file->set_sensitive(!operation_in_progress);
	m_btn_key_file_create->set_sensitive(!operation_in_progress);
	m_btn_cert_file->set_sensitive(!operation_in_progress);
	m_btn_cert_file_create->set_sensitive(
		!operation_in_progress &&
		m_cert_manager.get_private_key() != NULL);
}

void Gobby::PreferencesDialog::Security::on_auth_cert_toggled()
{
	m_grid_auth_cert->set_sensitive(m_btn_auth_cert->get_active());
}

void Gobby::PreferencesDialog::Security::on_create_key_clicked()
{
	m_file_dialog.reset(new FileChooser::Dialog(
		m_file_chooser, get_toplevel(*m_btn_key_file_create),
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
		m_file_chooser, get_toplevel(*m_btn_cert_file_create),
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

Gobby::PreferencesDialog::PreferencesDialog(
	const Glib::RefPtr<Gtk::Builder>& builder,
	FileChooser& file_chooser,
	Preferences& preferences,
	CertificateManager& cert_manager
):
	Gtk::Dialog(GTK_DIALOG(gtk_builder_get_object(builder->gobj(),
	                                              "PreferencesDialog"))),
	m_page_user(builder, preferences),
	m_page_editor(builder, preferences),
	m_page_view(builder, preferences),
	m_page_appearance(builder, preferences),
	m_page_security(builder, file_chooser, preferences, cert_manager)
{
	add_button(_("_Close"), Gtk::RESPONSE_CLOSE);
}

std::auto_ptr<Gobby::PreferencesDialog>
Gobby::PreferencesDialog::create(Gtk::Window& parent,
                                 FileChooser& file_chooser,
                                 Preferences& preferences,
	                         CertificateManager& cert_manager)
{
	Glib::RefPtr<Gtk::Builder> builder =
		Gtk::Builder::create_from_resource(
			"/de/0x539/gobby/ui/preferences-dialog.ui");

	std::auto_ptr<PreferencesDialog> dialog(
		new PreferencesDialog(builder, file_chooser,
		                      preferences, cert_manager));

	dialog->set_transient_for(parent);
	return dialog;
}

void Gobby::PreferencesDialog::on_response(int id)
{
	hide();
}
