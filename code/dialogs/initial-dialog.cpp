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

#include "dialogs/initial-dialog.hpp"

#include "core/credentialsgenerator.hpp"

#include "util/color.hpp"
#include "util/file.hpp"
#include "util/i18n.hpp"
#include "features.hpp"

#include <glibmm/markup.h>
#include <gtkmm/stock.h>
#include <gtkmm/alignment.h>

namespace
{
	Gtk::Widget* align_top(Gtk::Widget& widget)
	{
		Gtk::Alignment* alignment =
			new Gtk::Alignment(Gtk::ALIGN_CENTER,
			                   Gtk::ALIGN_START,
			                   1.0, 0.0);
		alignment->add(widget);
		alignment->show();
		return Gtk::manage(alignment);
	}

	Gtk::Widget* indent(Gtk::Widget& widget)
	{
		Gtk::Alignment* alignment = new Gtk::Alignment;
		alignment->set_padding(0, 0, 12, 0);
		alignment->add(widget);
		alignment->show();
		return Gtk::manage(alignment);
	}

	class InitialCertGenerator
	{
	public:
		InitialCertGenerator(Gobby::CertificateManager& cert_manager,
		                     Gobby::StatusBar& status_bar,
		                     const std::string& key_filename,
		                     const std::string& cert_filename):
			m_cert_manager(cert_manager),
			m_status_bar(status_bar),
			m_key_filename(key_filename),
			m_cert_filename(cert_filename),
			m_key(NULL),
			m_status_handle(m_status_bar.invalid_handle())
		{
			m_key_handle = Gobby::create_key(
				GNUTLS_PK_RSA,
				2048,
				sigc::mem_fun(
					*this,
					&InitialCertGenerator::
						on_key_generated));

			m_status_handle = m_status_bar.add_info_message(
				Gobby::_("Generating 2048-bit "
				         "RSA private key..."));
		}

		~InitialCertGenerator()
		{
			if(m_key != NULL)
				gnutls_x509_privkey_deinit(m_key);
			if(m_status_handle != m_status_bar.invalid_handle())
				m_status_bar.remove_message(m_status_handle);
		}

	private:
		void on_key_generated(const Gobby::KeyGeneratorHandle* handle,
		                      gnutls_x509_privkey_t key,
		                      const GError* error)
		{
			m_key_handle.reset(NULL);
			m_key = key;

			if(error != NULL)
			{
				m_status_bar.add_error_message(
					Gobby::_("Failed to generate "
					         "private key"),
					Glib::ustring::compose(
						Gobby::_("%1\n\nYou can try "
						         "again to create a "
						         "key in the "
						         "Security tab of "
						         "the preferences "
						         "dialog."),
						error->message));

				m_cert_manager.set_private_key(
					NULL, m_key_filename.c_str(), error);

				done();
				return;
			}

			m_crt_handle = Gobby::create_self_signed_certificate(
				m_key,
				sigc::mem_fun(
					*this,
					&InitialCertGenerator::
						on_cert_generated));
		}

		void on_cert_generated(
			const Gobby::CertificateGeneratorHandle* hndl,
			gnutls_x509_crt_t cert,
			const GError* error)
		{
			m_status_bar.remove_message(m_status_handle);
			m_status_handle = m_status_bar.invalid_handle();
			m_crt_handle.reset(NULL);

			m_cert_manager.set_private_key(
				m_key, m_key_filename.c_str(), NULL);
			m_key = NULL;

			if(error != NULL)
			{
				m_status_bar.add_error_message(
					Gobby::_("Failed to generate "
					         "self-signed certificate"),
					Glib::ustring::compose(
						Gobby::_("%1\n\nYou can try "
						         "again to create a "
						         "certificate in the "
						         "Security tab of "
						         "the preferences "
						         "dialog."),
						error->message));
			}
			else
			{
				// Note this needs to be allocated with
				// g_malloc, since it is directly passed and
				// freed by libinfinity
				gnutls_x509_crt_t* crt_array =
					static_cast<gnutls_x509_crt_t*>(
						g_malloc(sizeof(
							gnutls_x509_crt_t)));
				crt_array[0] = cert;

				m_cert_manager.set_certificates(
					crt_array, 1,
					m_cert_filename.c_str(), NULL);
			}

			done();
		}

		void done()
		{
			delete this;
		}

		Gobby::CertificateManager& m_cert_manager;
		Gobby::StatusBar& m_status_bar;
		const std::string m_key_filename;
		const std::string m_cert_filename;

		gnutls_x509_privkey_t m_key;
		std::auto_ptr<Gobby::KeyGeneratorHandle> m_key_handle;
		std::auto_ptr<Gobby::CertificateGeneratorHandle> m_crt_handle;
		Gobby::StatusBar::MessageHandle m_status_handle;
	};
}

Gobby::InitialDialog::InitialDialog(Gtk::Window& parent,
                                    StatusBar& status_bar,
                                    Preferences& preferences,
                                    CertificateManager& cert_manager,
                                    const IconManager& icon_manager):
	Gtk::Dialog("Gobby", parent),
	m_status_bar(status_bar),
	m_preferences(preferences),
	m_cert_manager(cert_manager),
	m_vbox(false, 12),
	m_color_button(_("Choose a user color"), *this),
	m_user_table(3, 2),
	m_remote_auth_external_table(2, 2)
{
	m_title.set_markup(
		"<span size=\"x-large\" weight=\"bold\">" +
		Glib::Markup::escape_text(_("Welcome to Gobby")) +
		"</span>");
	m_title.show();

	Gtk::IconSize::register_new("welcome", 128, 128);

	m_image.set_from_icon_name("gobby-0.5",
	                           Gtk::IconSize::from_name("welcome"));
	m_image.set_alignment(Gtk::ALIGN_CENTER, Gtk::ALIGN_START),
	m_image.show();

	m_intro.set_text(
		_("Before we start, a few options need to be configured. "
		  "You can later change them by choosing Edit ▸ Preferences "
		  "from the menu."));
	m_intro.set_width_chars(50);
	m_intro.set_line_wrap(true);
	m_intro.show();

	m_name_label.set_markup(
		"<b>" + Glib::Markup::escape_text(_("User Name")) + "</b>"
		"<small>\n\n" +
		Glib::Markup::escape_text(_("Your name as shown to "
		                            "other users.")) +
		"</small>");
	m_name_label.set_alignment(Gtk::ALIGN_START);
	m_name_label.set_line_wrap(true);
	m_name_label.set_width_chars(20);
	m_name_label.show();
	
	m_color_label.set_markup(
		"<b>" + Glib::Markup::escape_text(_("User Color")) + "</b>"
		"<small>\n\n" +
		Glib::Markup::escape_text(_("The color with which text you "
		                            "have written is branded.")) +
		"</small>");
	m_color_label.set_alignment(Gtk::ALIGN_START);
	m_color_label.set_line_wrap(true);
	m_color_label.set_width_chars(20);
	m_color_label.show();

	m_name_entry.set_text(preferences.user.name);
	m_name_entry.set_activates_default(true);
	m_name_entry.show();

	m_color_button.set_hue(preferences.user.hue);
	m_color_button.set_saturation(0.35);
	m_color_button.set_value(1.0);
	m_color_button.show();

	m_remote_label.set_markup(
		"<b>" +
		Glib::Markup::escape_text(_("Remote Connections")) +
		"</b>");
	m_remote_label.set_alignment(Gtk::ALIGN_START);
	m_remote_label.show();

	m_remote_allow_connections.set_label(
		_("Allow remote users to edit local documents"));
	m_remote_allow_connections.set_active(
		preferences.user.allow_remote_access);
	m_remote_allow_connections.show();

	m_remote_require_password.set_label(
		_("Ask remote users for a password")); 
	m_remote_require_password.set_active(
		preferences.user.require_password);
	m_remote_require_password.show();

	m_remote_password_label.set_label(_("Password:"));
	m_remote_password_label.show();

	m_remote_password_entry.set_visibility(false);
	m_remote_password_entry.set_text(
		static_cast<std::string>(preferences.user.password));
	m_remote_password_entry.show();

	m_remote_password_box.set_spacing(12);
	m_remote_password_box.pack_start(
		m_remote_password_label, Gtk::PACK_SHRINK);
	m_remote_password_box.pack_start(
		m_remote_password_entry, Gtk::PACK_EXPAND_WIDGET);
	m_remote_password_box.set_sensitive(
		preferences.user.require_password);
	m_remote_password_box.show();

	m_remote_auth_label.set_markup(
		"<b>" + Glib::Markup::escape_text("Authentication") + "</b>");
	m_remote_auth_label.set_alignment(Gtk::ALIGN_START);
	m_remote_auth_label.show();

	Gtk::RadioButton::Group group;
	m_remote_auth_none.set_label(
		_("No authentication (Not recommended)"));
	m_remote_auth_none.set_group(group);
	m_remote_auth_none.show();

	m_remote_auth_none_help.set_markup(
		"<small>" +
		Glib::Markup::escape_text(
			_("Don't authenticate ourselves to remote users. "
			  "Data transfer will be unencrypted.")) +
		"</small>");
	m_remote_auth_none_help.set_line_wrap(true);
	m_remote_auth_none_help.set_width_chars(30);
	m_remote_auth_none_help.set_alignment(Gtk::ALIGN_START);
	m_remote_auth_none_help.show();

	m_remote_auth_self.set_label(
		_("Create a self-signed certificate (Recommended)"));
	m_remote_auth_self.set_group(group);
	m_remote_auth_self.set_active(true);
	m_remote_auth_self.show();

	m_remote_auth_self_help.set_markup(
		"<small>" +
		Glib::Markup::escape_text(
			_("It may take a minute or two until remote users "
			  "can connect while the security certificate is "
			  "being generated.")) +
		"</small>");
	m_remote_auth_self_help.set_line_wrap(true);
	m_remote_auth_self_help.set_width_chars(30);
	m_remote_auth_self_help.set_alignment(Gtk::ALIGN_START);
	m_remote_auth_self_help.show();

	m_remote_auth_external.set_label(
		_("Use an existing certificate (Expert option)"));
	m_remote_auth_external.set_group(group);
	m_remote_auth_external.show();

	m_remote_auth_external_help.set_markup(
		"<small>" +
		Glib::Markup::escape_text(
			_("Use an existing private key and certificate for "
			  "authentication. The files must be in PEM "
			  "format.")) +
		"</small>");
	m_remote_auth_external_help.set_line_wrap(true);
	m_remote_auth_external_help.set_width_chars(30);
	m_remote_auth_external_help.set_alignment(Gtk::ALIGN_START);
	m_remote_auth_external_help.show();

	m_remote_auth_external_keyfile_label.set_label(_("Private key:"));
	m_remote_auth_external_keyfile_label.show();
	m_remote_auth_external_certfile_label.set_label(_("Certificate:"));
	m_remote_auth_external_certfile_label.show();
	m_remote_auth_external_keyfile.show();
	m_remote_auth_external_certfile.show();

	m_remote_auth_external_table.set_spacings(6);
	m_remote_auth_external_table.attach(
		m_remote_auth_external_keyfile_label, 0, 1, 0, 1,
		Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK);
	m_remote_auth_external_table.attach(
		m_remote_auth_external_keyfile, 1, 2, 0, 1,
		Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::SHRINK);
	m_remote_auth_external_table.attach(
		m_remote_auth_external_certfile_label, 0, 1, 1, 2,
		Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK);
	m_remote_auth_external_table.attach(
		m_remote_auth_external_certfile, 1, 2, 1, 2,
		Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::SHRINK);
	m_remote_auth_external_table.set_sensitive(false);
	m_remote_auth_external_table.show();

	m_remote_auth_box.set_spacing(6);
	m_remote_auth_box.pack_start(
		m_remote_auth_none, Gtk::PACK_SHRINK);
	m_remote_auth_box.pack_start(
		*indent(m_remote_auth_none_help), Gtk::PACK_SHRINK);
	m_remote_auth_box.pack_start(
		m_remote_auth_self, Gtk::PACK_SHRINK);
	m_remote_auth_box.pack_start(
		*indent(m_remote_auth_self_help), Gtk::PACK_SHRINK);
	m_remote_auth_box.pack_start(
		m_remote_auth_external, Gtk::PACK_SHRINK);
	m_remote_auth_box.pack_start(
		*indent(m_remote_auth_external_help), Gtk::PACK_SHRINK);
	m_remote_auth_box.pack_start(
		*indent(m_remote_auth_external_table), Gtk::PACK_SHRINK);
	m_remote_auth_box.show();

	m_remote_options_box.set_spacing(6);
	m_remote_options_box.pack_start(
		m_remote_require_password, Gtk::PACK_SHRINK);
	m_remote_options_box.pack_start(
		*indent(m_remote_password_box), Gtk::PACK_SHRINK);
	m_remote_options_box.pack_start(
		m_remote_auth_label, Gtk::PACK_SHRINK);
	m_remote_options_box.pack_start(
		*indent(m_remote_auth_box), Gtk::PACK_SHRINK);
	m_remote_options_box.set_sensitive(
		preferences.user.allow_remote_access);
	m_remote_options_box.show();

	m_remote_box.pack_start(m_remote_label, Gtk::PACK_SHRINK);
	m_remote_box.pack_start(m_remote_allow_connections, Gtk::PACK_SHRINK);
	m_remote_box.pack_start(*indent(m_remote_options_box),
	                        Gtk::PACK_SHRINK);
	m_remote_box.show();

	m_user_table.set_row_spacings(12);

	m_user_table.attach(m_image, 0, 2, 0, 1,
	                    Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK);
	m_user_table.attach(m_name_label, 0, 1, 1, 2,
	                    Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK);
	m_user_table.attach(*align_top(m_name_entry), 1, 2, 1, 2,
	                    Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::SHRINK);
	m_user_table.attach(m_color_label, 0, 1, 2, 3,
	                    Gtk::FILL | Gtk::SHRINK, Gtk::FILL | Gtk::SHRINK);
	m_user_table.attach(*align_top(m_color_button), 1, 2, 2, 3,
	                    Gtk::FILL | Gtk::EXPAND, Gtk::FILL | Gtk::SHRINK);
	m_user_table.show();

	m_main_box.set_spacing(24);
	m_main_box.pack_start(m_user_table, Gtk::PACK_SHRINK);
	m_main_box.pack_start(m_remote_box, Gtk::PACK_SHRINK);
	m_main_box.show();

	m_vbox.set_spacing(24);
	m_vbox.pack_start(m_intro, Gtk::PACK_SHRINK);
	m_vbox.pack_start(m_main_box, Gtk::PACK_EXPAND_WIDGET);
	m_vbox.show();

	m_topbox.pack_start(m_title, Gtk::PACK_SHRINK);
	m_topbox.pack_start(m_vbox, Gtk::PACK_EXPAND_WIDGET);
	m_topbox.set_spacing(24);
	m_topbox.set_border_width(12);
	m_topbox.show();

	m_remote_allow_connections.signal_toggled().connect(
		sigc::mem_fun(*this,
			&Gobby::InitialDialog::
				on_remote_allow_connections_toggled));
	m_remote_require_password.signal_toggled().connect(
		sigc::mem_fun(*this,
			&Gobby::InitialDialog::
				on_remote_require_password_toggled));
	m_remote_auth_external.signal_toggled().connect(
		sigc::mem_fun(*this,
			&Gobby::InitialDialog::
				on_remote_auth_external_toggled));

	get_vbox()->pack_start(m_topbox, Gtk::PACK_EXPAND_WIDGET);

	set_resizable(false);
	add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);
}

void Gobby::InitialDialog::on_response(int id)
{
	m_preferences.user.name = m_name_entry.get_text();
	m_preferences.user.hue =
		hue_from_gdk_color(m_color_button.get_color());
	m_preferences.user.allow_remote_access =
		m_remote_allow_connections.get_active();
	m_preferences.user.require_password =
		m_remote_require_password.get_active();
	m_preferences.user.password =
		m_remote_password_entry.get_text();
	m_preferences.security.authentication_enabled =
		!m_remote_auth_none.get_active();
	if(m_remote_auth_self.get_active())
	{
		// The certificate generator takes care of its own:
		new InitialCertGenerator(
			m_cert_manager, m_status_bar,
			config_filename("key.pem"),
			config_filename("cert.pem"));
	}
	else
	{
		m_preferences.security.certificate_file =
			m_remote_auth_external_certfile.get_filename();
		m_preferences.security.key_file =
			m_remote_auth_external_keyfile.get_filename();
	}

	hide();
}

void Gobby::InitialDialog::on_remote_allow_connections_toggled()
{
	m_remote_options_box.set_sensitive(
		m_remote_allow_connections.get_active());
}

void Gobby::InitialDialog::on_remote_require_password_toggled()
{
	m_remote_password_box.set_sensitive(
		m_remote_require_password.get_active());
}

void Gobby::InitialDialog::on_remote_auth_external_toggled()
{
	m_remote_auth_external_table.set_sensitive(
		m_remote_auth_external.get_active());
}
