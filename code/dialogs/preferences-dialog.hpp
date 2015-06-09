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

#include <gtkmm/dialog.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/scale.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/fontbutton.h>
#include <gtkmm/liststore.h>

namespace Gobby
{

template<typename OptionType>
class PreferencesComboBox: public Gtk::ComboBox
{
public:
	PreferencesComboBox(GtkComboBox* box,
	                    const Glib::RefPtr<Gtk::Builder>& builder):
		Gtk::ComboBox(box), m_option(NULL),
		m_store(Gtk::ListStore::create(m_columns))
	{
		set_model(m_store);

		Gtk::CellRendererText* renderer =
			Gtk::manage(new Gtk::CellRendererText);
		pack_start(*renderer, true);
		add_attribute(renderer->property_text(), m_columns.text);
	}

	void set_option(Preferences::Option<OptionType>& option)
	{
		m_option = &option;
	}

	void add(const Glib::ustring& text, const OptionType& value)
	{
		Gtk::TreeIter iter = m_store->append();
		(*iter)[m_columns.text] = text;
		(*iter)[m_columns.value] = value;

		if(*m_option == value)
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

		if(*m_option != value)
			*m_option = value;
	}

	Preferences::Option<OptionType>* m_option;

	Columns m_columns;
	Glib::RefPtr<Gtk::ListStore> m_store;
};

class PreferencesDialog : public Gtk::Dialog
{
private:
	PreferencesDialog(const Glib::RefPtr<Gtk::Builder>& builder,
	                  FileChooser& file_chooser,
	                  Preferences& preferences,
	                  CertificateManager& cert_manager);

public:
	static std::auto_ptr<PreferencesDialog> create(
		Gtk::Window& parent, FileChooser& file_chooser,
		Preferences& preferences, CertificateManager& cert_manager);

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

	// Note that old GTK+ versions do not emit the file-set signal
	// of Gtk::FileChooserButton when a new entry is chosen from
	// the combo box, but only from the dialog. The bug was fixed
	// only in GTK+ 3.10.0.
	//
	// Therefore, we are using the selection-changed signal
	// instead. However, when the button is constructed, the
	// signal is emitted "too often" since it is first emitted
	// with the home folder of the current user, and then again
	// with what was specified when set_filename() is called
	// to set the initial name. In the case of the local documents
	// directory, this causes a significant overhead and running
	// sessions being closed once the preferences dialog is opened.
	//
	// To work around this, we ignore all signal emissions until
	// the first signal emission with the initially set value occurs.
	// Once we depend on GTK+ 3.10 or greater, the workaround
	// can be removed and we can switch to signal_file_set().
	class PathConnection: public DuplexConnection<std::string>
	{
	public:
		PathConnection(Gtk::FileChooser& file_chooser,
		               Preferences::Option<std::string>& option):
			DuplexConnection<std::string>(
				file_chooser.signal_selection_changed(),
				option.signal_changed()),
			m_file_chooser(file_chooser),
			m_option(option),
			m_initial_value(option),
			m_seen_initial_value(file_chooser.get_filename() ==
			                     m_initial_value)
		{
		}

	protected:
		virtual std::string get1() const
			{ return m_file_chooser.get_filename(); }
		virtual std::string get2() const
		{
			if(m_seen_initial_value)
				return m_option.get();
			else
				return "";
		}

		virtual void set1(const std::string& val)
			{ m_file_chooser.set_filename(val); }
		virtual void set2(const std::string& val)
		{
			if(!m_seen_initial_value)
			{
				if(val == m_initial_value)
					m_seen_initial_value = true;
			}
			else
			{
				m_option.set(val);
			}
		}

	private:
		Gtk::FileChooser& m_file_chooser;
		Preferences::Option<std::string>& m_option;
		const std::string m_initial_value;
		bool m_seen_initial_value;
	};

	class User
	{
	public:
		User(const Glib::RefPtr<Gtk::Builder>& builder,
		     Preferences& preferences);

	protected:
		void on_local_allow_connections_toggled();
		void on_local_require_password_toggled();
		void on_local_keep_documents_toggled();

		Gtk::Entry* m_ent_user_name;
		HueButton* m_btn_user_color;

		Gtk::Scale* m_scl_user_alpha;

		Gtk::CheckButton* m_btn_remote_show_cursors;
		Gtk::CheckButton* m_btn_remote_show_selections;
		Gtk::CheckButton* m_btn_remote_show_current_lines;
		Gtk::CheckButton* m_btn_remote_show_cursor_positions;

		Gtk::Grid* m_grid_local_connections;
		Gtk::CheckButton* m_btn_local_allow_connections;
		Gtk::Grid* m_grid_password;
		Gtk::CheckButton* m_btn_local_require_password;
		Gtk::Entry* m_ent_local_password;
		Gtk::SpinButton* m_ent_local_port;
		Gtk::CheckButton* m_btn_local_keep_documents;
		Gtk::Grid* m_grid_local_documents_directory;
		Gtk::FileChooserButton* m_btn_local_documents_directory;
		std::auto_ptr<PathConnection>
			m_conn_local_documents_directory;
	};

	class Editor
	{
	public:
		Editor(const Glib::RefPtr<Gtk::Builder>& builder,
		       Preferences& preferences);

	protected:
		void on_autosave_enabled_toggled();

		Gtk::SpinButton* m_ent_tab_width;
		Gtk::CheckButton* m_btn_tab_spaces;

		Gtk::CheckButton* m_btn_indentation_auto;

		Gtk::CheckButton* m_btn_homeend_smart;

		Gtk::CheckButton* m_btn_autosave_enabled;
		Gtk::Grid* m_grid_autosave_interval;
		Gtk::SpinButton* m_ent_autosave_interval;
	};

	class View
	{
	public:
		View(const Glib::RefPtr<Gtk::Builder>& builder,
		     Preferences& preferences);

	protected:
		void on_wrap_text_toggled();
		void on_margin_display_toggled();

		Gtk::CheckButton* m_btn_wrap_text;
		Gtk::CheckButton* m_btn_wrap_words;

		Gtk::CheckButton* m_btn_linenum_display;

		Gtk::CheckButton* m_btn_curline_highlight;

		Gtk::CheckButton* m_btn_margin_display;
		Gtk::Grid* m_grid_margin_pos;
		Gtk::SpinButton* m_ent_margin_pos;

		Gtk::CheckButton* m_btn_bracket_highlight;
		PreferencesComboBox<GtkSourceDrawSpacesFlags>*
			m_cmb_spaces_display;
	};

	class Appearance
	{
	public:
		Appearance(const Glib::RefPtr<Gtk::Builder>& builder,
		           Preferences& preferences);

	protected:
		class SchemeColumns: public Gtk::TreeModelColumnRecord
		{
		public:
			Gtk::TreeModelColumn<GtkSourceStyleScheme*> scheme;
			Gtk::TreeModelColumn<Glib::ustring> name;
			Gtk::TreeModelColumn<Glib::ustring> description;
			SchemeColumns()
			{
				add(scheme);
				add(name);
				add(description);
			}
		};

		void on_pref_font_changed();

		void on_scheme_changed(Preferences& preferences);

		PreferencesComboBox<Gtk::ToolbarStyle>* m_cmb_toolbar_style;
		Gtk::FontButton* m_btn_font;
		std::auto_ptr<FontConnection> m_conn_font;

		SchemeColumns m_scheme_columns;
		Glib::RefPtr<Gtk::ListStore> m_scheme_list;
		Gtk::TreeView* m_scheme_tree;
	};

	class Security
	{
	public:
		Security(const Glib::RefPtr<Gtk::Builder>& builder,
		         FileChooser& file_chooser,
		         Preferences& preferences,
		         CertificateManager& cert_manager);
	
	protected:
		void set_file_error(Gtk::Label& label, const GError* error);
		Gtk::Window& get_toplevel(Gtk::Widget& widget);

		void on_use_system_trust_toggled();

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
		CertificateManager& m_cert_manager;

		Gtk::CheckButton* m_btn_use_system_trust;
		Gtk::FileChooserButton* m_btn_path_trust_file;
		std::auto_ptr<PathConnection> m_conn_path_trust_file;
		Gtk::Label* m_error_trust_file;

		PreferencesComboBox<InfXmppConnectionSecurityPolicy>*
			m_cmb_connection_policy;

		Gtk::RadioButton* m_btn_auth_none;
		Gtk::RadioButton* m_btn_auth_cert;
		Gtk::Grid* m_grid_auth_cert;

		Gtk::FileChooserButton* m_btn_key_file;
		std::auto_ptr<PathConnection> m_conn_path_key_file;
		Gtk::Button* m_btn_key_file_create;
		Gtk::Label* m_error_key_file;

		Gtk::FileChooserButton* m_btn_cert_file;
		std::auto_ptr<PathConnection> m_conn_path_cert_file;
		Gtk::Button* m_btn_cert_file_create;
		Gtk::Label* m_error_cert_file;

		std::auto_ptr<KeyGeneratorHandle> m_key_generator_handle;
		std::auto_ptr<CertificateGeneratorHandle>
			m_cert_generator_handle;
		std::auto_ptr<FileChooser::Dialog> m_file_dialog;
	};

protected:
	virtual void on_response(int id);

	User m_page_user;
	Editor m_page_editor;
	View m_page_view;
	Appearance m_page_appearance;
	Security m_page_security;
};

}

#endif // _GOBBY_PREFERENCESDIALOG_HPP_

