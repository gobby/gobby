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

#ifndef _GOBBY_PREFERENCESDIALOG_HPP_
#define _GOBBY_PREFERENCESDIALOG_HPP_

#include "core/preferences.hpp"
#include "core/filechooser.hpp"
#include "core/certificatemanager.hpp"
#include "core/credentialsgenerator.hpp"
#include "core/huebutton.hpp"
#include "util/groupframe.hpp"

#include <gtkmm/dialog.h>
#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/notebook.h>
#include <gtkmm/alignment.h>
#include <gtkmm/scale.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/fontbutton.h>
#include <gtkmm/sizegroup.h>
#include <gtkmm/liststore.h>

namespace Gobby
{

template<typename OptionType>
class PreferencesComboBox: public Gtk::ComboBox
{
public:
	PreferencesComboBox(Preferences::Option<OptionType>& option):
		m_option(option), m_store(Gtk::ListStore::create(m_columns))
	{
		set_model(m_store);

		Gtk::CellRendererText* renderer =
			Gtk::manage(new Gtk::CellRendererText);
		pack_start(*renderer, true);
		add_attribute(renderer->property_text(), m_columns.text);
	}

	void add(const Glib::ustring& text, const OptionType& value)
	{
		Gtk::TreeIter iter = m_store->append();
		(*iter)[m_columns.text] = text;
		(*iter)[m_columns.value] = value;

		if(m_option == value)
			set_active(iter);
	}

private:
	class Columns: public Gtk::TreeModelColumnRecord
	{
	public:
		Gtk::TreeModelColumn<Glib::ustring> text;
		Gtk::TreeModelColumn<OptionType> value;

		Columns() { add(text); add(value); }
	};

	virtual void on_changed()
	{
		Gtk::ComboBox::on_changed();
		OptionType value = (*get_active())[m_columns.value];

		if(m_option != value)
			m_option = value;
	}

	Preferences::Option<OptionType>& m_option;

	Columns m_columns;
	Glib::RefPtr<Gtk::ListStore> m_store;
};

class PreferencesDialog : public Gtk::Dialog
{
public:
	// An object which keeps two values in sync with each other, using
	// notification signals of both objects.
	template<typename Type>
	class DuplexConnection
	{
	public:
		template<typename Sig1, typename Sig2>
		DuplexConnection(Sig1 sig1, Sig2 sig2)
		{
			m_conn1 = sig1.connect(sigc::mem_fun(*this, &DuplexConnection<Type>::changed1));
			m_conn2 = sig2.connect(sigc::mem_fun(*this, &DuplexConnection<Type>::changed2));
		}
		
		virtual ~DuplexConnection()
		{
			m_conn1.disconnect();
			m_conn2.disconnect();
		}

		void block()
		{
			m_conn1.block();
			m_conn2.block();
		}

		void unblock()
		{
			m_conn1.unblock();
			m_conn2.unblock();
		}
	private:
		void changed1()
		{
			m_conn2.block();
			if(get2() != get1())
				set2(get1());
			m_conn2.unblock();
		}

		void changed2()
		{
			m_conn1.block();
			if(get1() != get2())
				set1(get2());
			m_conn1.unblock();
		}

	protected:
		virtual Type get1() const = 0;
		virtual Type get2() const = 0;
		virtual void set1(const Type& t) = 0;
		virtual void set2(const Type& t) = 0;

	private:
		sigc::connection m_conn1;
		sigc::connection m_conn2;
	};

	class FontConnection: public DuplexConnection<Pango::FontDescription>
	{
	public:
		FontConnection(
			Gtk::FontButton& font_button,
		        Preferences::Option<Pango::FontDescription>& option)
		:
			DuplexConnection<Pango::FontDescription>(
				font_button.signal_font_set(),
				option.signal_changed()),
			m_font_button(font_button),
			m_option(option)
		{
		}
	protected:
		virtual Pango::FontDescription get1() const
		{
			return Pango::FontDescription(
				m_font_button.get_font_name());
		}

		virtual Pango::FontDescription get2() const
			{ return m_option.get(); }
		virtual void set1(const Pango::FontDescription& val)
			{ m_font_button.set_font_name(val.to_string()); }
		virtual void set2(const Pango::FontDescription& val)
			{ m_option.set(val); }

	private:
		Gtk::FontButton& m_font_button;
		Preferences::Option<Pango::FontDescription>& m_option;
	};

	class PathConnection: public DuplexConnection<std::string>
	{
	public:
		PathConnection(Gtk::FileChooser& file_chooser,
		               Preferences::Option<std::string>& option):
			DuplexConnection<std::string>(
				file_chooser.signal_selection_changed(),
				option.signal_changed()),
			m_file_chooser(file_chooser),
			m_option(option)
		{
		}

	protected:
		virtual std::string get1() const
			{ return m_file_chooser.get_filename(); }
		virtual std::string get2() const
			{ return m_option.get(); }
		virtual void set1(const std::string& val)
			{ m_file_chooser.set_filename(val); }
		virtual void set2(const std::string& val)
			{ m_option.set(val); }

	private:
		Gtk::FileChooser& m_file_chooser;
		Preferences::Option<std::string>& m_option;
	};

	template<typename OptionType>
	class ComboColumns: public Gtk::TreeModelColumnRecord
	{
	};

	class Page: public Gtk::Frame
	{
	public:
		Page();
		void add(Gtk::Widget& widget, bool expand);

	protected:
		Gtk::VBox m_box;
	};

	class User: public Page
	{
	public:
		User(Gtk::Window& parent, Preferences& preferences);

	protected:
		void on_local_allow_connections_toggled();
		void on_local_require_password_toggled();
		void on_local_keep_documents_toggled();

		GroupFrame m_group_settings;
		GroupFrame m_group_remote;
		GroupFrame m_group_local;

		Gtk::HBox m_box_user_name;
		Gtk::Label m_lbl_user_name;
		Gtk::Entry m_ent_user_name;

		Gtk::HBox m_box_user_color;
		Gtk::Label m_lbl_user_color;
		HueButton m_btn_user_color;

		Gtk::HBox m_box_user_alpha;
		Gtk::Label m_lbl_user_alpha;
		Gtk::HScale m_scl_user_alpha;

		Gtk::CheckButton m_btn_remote_show_cursors;
		Gtk::CheckButton m_btn_remote_show_selections;
		Gtk::CheckButton m_btn_remote_show_current_lines;
		Gtk::CheckButton m_btn_remote_show_cursor_positions;

		Gtk::CheckButton m_btn_local_allow_connections;
		Gtk::CheckButton m_btn_local_require_password;
		Gtk::Label m_lbl_local_password;
		Gtk::Entry m_ent_local_password;
		Gtk::HBox m_box_local_password;
		Gtk::Label m_lbl_local_port;
		Gtk::SpinButton m_ent_local_port;
		Gtk::HBox m_box_local_port;
		Gtk::VBox m_box_local_connections;
		Gtk::CheckButton m_btn_local_keep_documents;
		Gtk::Label m_lbl_local_documents_directory;
		Gtk::FileChooserButton m_btn_local_documents_directory;
		Gtk::HBox m_box_local_documents_directory;

		Glib::RefPtr<Gtk::SizeGroup> m_size_group;
	};

	class Editor: public Page
	{
	public:
		Editor(Preferences& preferences);

	protected:
		void on_autosave_enabled_toggled();

		GroupFrame m_group_tab;
		GroupFrame m_group_indentation;
		GroupFrame m_group_homeend;
		GroupFrame m_group_saving;

		Gtk::HBox m_box_tab_width;
		Gtk::Label m_lbl_tab_width;
		Gtk::SpinButton m_ent_tab_width;
		Gtk::CheckButton m_btn_tab_spaces;

		Gtk::CheckButton m_btn_indentation_auto;

		Gtk::CheckButton m_btn_homeend_smart;

		Gtk::CheckButton m_btn_autosave_enabled;
		Gtk::HBox m_box_autosave_interval;
		Gtk::Label m_lbl_autosave_interval;
		Gtk::Label m_lbl_autosave_interval_suffix;
		Gtk::SpinButton m_ent_autosave_interval;
	};

	class View: public Page
	{
	public:
		View(Preferences& preferences);
		void set(Preferences::View& view) const;

	protected:
		void on_wrap_text_toggled();
		void on_margin_display_toggled();

		GroupFrame m_group_wrap;
		GroupFrame m_group_linenum;
		GroupFrame m_group_curline;
		GroupFrame m_group_margin;
		GroupFrame m_group_bracket;
		GroupFrame m_group_spaces;

		Gtk::CheckButton m_btn_wrap_text;
		Gtk::CheckButton m_btn_wrap_words;

		Gtk::CheckButton m_btn_linenum_display;

		Gtk::CheckButton m_btn_curline_highlight;

		Gtk::CheckButton m_btn_margin_display;
		Gtk::HBox m_box_margin_pos;
		Gtk::Label m_lbl_margin_pos;
		Gtk::SpinButton m_ent_margin_pos;

		Gtk::CheckButton m_btn_bracket_highlight;
		PreferencesComboBox<GtkSourceDrawSpacesFlags>
			m_cmb_spaces_display;
	};

	class Appearance: public Page
	{
	public:
		Appearance(Preferences& preferences);

	protected:
		class Columns: public Gtk::TreeModelColumnRecord
		{
		public:
			Gtk::TreeModelColumn<GtkSourceStyleScheme*> scheme;
			Gtk::TreeModelColumn<Glib::ustring> name;
			Gtk::TreeModelColumn<Glib::ustring> description;
			Columns()
			{
				add(scheme);
				add(name);
				add(description);
			}
		};

		void on_pref_font_changed();

		void on_scheme_changed(Preferences& preferences);

		GroupFrame m_group_toolbar;
		GroupFrame m_group_font;
		GroupFrame m_group_scheme;

		PreferencesComboBox<Gtk::ToolbarStyle> m_cmb_toolbar_style;
		Gtk::FontButton m_btn_font;
		FontConnection m_conn_font;

		Columns m_columns;
		Glib::RefPtr<Gtk::ListStore> m_list;
		Gtk::TreeView m_tree;
	};

	class Security: public Page
	{
	public:
		Security(Gtk::Window& parent,
		         FileChooser& file_chooser,
		         Preferences& preferences,
		         CertificateManager& cert_manager);
	
	protected:
		void set_file_error(Gtk::Label& label, const GError* error);

		void on_credentials_changed();
		void on_auth_cert_toggled();

		void on_create_key_clicked();
		void on_create_cert_clicked();

		void on_file_dialog_response_key(int response_id);
		void on_file_dialog_response_certificate(int response_id);
		
		void on_key_generated(const KeyGeneratorHandle* handle,
		                      gnutls_x509_privkey_t key,
		                      const GError* error,
		                      const std::string& filename);
		void on_cert_generated(const CertificateGeneratorHandle* hndl,
		                       gnutls_x509_crt_t cert,
		                       const GError* error,
		                       const std::string& filename);

		Preferences& m_preferences;
		FileChooser& m_file_chooser;
		Gtk::Window& m_parent;
		CertificateManager& m_cert_manager;

		GroupFrame m_group_trust_file;
		GroupFrame m_group_connection_policy;
		GroupFrame m_group_authentication;

		Gtk::FileChooserButton m_btn_path_trust_file;
		PathConnection m_conn_path_trust_file;
		Gtk::Label m_error_trust_file;
		PreferencesComboBox<InfXmppConnectionSecurityPolicy>
			m_cmb_connection_policy;

		Gtk::RadioButton m_btn_auth_none;
		Gtk::RadioButton m_btn_auth_cert;
		Gtk::Label m_lbl_key_file;
		Gtk::FileChooserButton m_btn_key_file;
		PathConnection m_conn_path_key_file;
		Gtk::Button m_btn_key_file_create;
		Gtk::HBox m_box_key_file;
		Gtk::Label m_error_key_file;
		Gtk::Label m_lbl_cert_file;
		Gtk::FileChooserButton m_btn_cert_file;
		PathConnection m_conn_path_cert_file;
		Gtk::Button m_btn_cert_file_create;
		Gtk::HBox m_box_cert_file;
		Gtk::Label m_error_cert_file;

		Glib::RefPtr<Gtk::SizeGroup> m_size_group;

		std::auto_ptr<KeyGeneratorHandle> m_key_generator_handle;
		std::auto_ptr<CertificateGeneratorHandle> m_cert_generator_handle;
		std::auto_ptr<FileChooser::Dialog> m_file_dialog;
	};

	PreferencesDialog(Gtk::Window& parent,
	                  FileChooser& file_chooser,
	                  Preferences& preferences,
	                  CertificateManager& cert_manager);

protected:
	virtual void on_response(int id);

	Preferences& m_preferences;

	Gtk::Notebook m_notebook;

	User m_page_user;
	Editor m_page_editor;
	View m_page_view;
	Appearance m_page_appearance;
	Security m_page_security;
};

}

#endif // _GOBBY_PREFERENCESDIALOG_HPP_

