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

Gobby::InitialDialog::InitialDialog(
	GtkDialog* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
:
	Gtk::Dialog(cobject), m_status_bar(NULL), m_preferences(NULL),
	m_cert_manager(NULL)
{
	builder->get_widget("name-entry", m_name_entry);
	builder->get_widget_derived("color-button", m_color_button);

	builder->get_widget("allow-remote-connections",
	                    m_remote_allow_connections);
	builder->get_widget("ask-password", m_remote_require_password);
	builder->get_widget("password", m_remote_password_entry);
	builder->get_widget("create-self-signed", m_remote_auth_self);
	builder->get_widget("use-existing-certificate",
	                    m_remote_auth_external);
	builder->get_widget("private-key-file",
	                    m_remote_auth_external_keyfile);
	builder->get_widget("certificate-file",
	                    m_remote_auth_external_certfile);

	builder->get_widget("remote-connections-grid",
	                    m_remote_connections_grid);
	builder->get_widget("password-grid",
	                    m_password_grid);
	builder->get_widget("certificate-grid",
	                    m_certificate_grid);

	m_remote_allow_connections->signal_toggled().connect(
		sigc::mem_fun(*this,
			&Gobby::InitialDialog::
				on_remote_allow_connections_toggled));
	m_remote_require_password->signal_toggled().connect(
		sigc::mem_fun(*this,
			&Gobby::InitialDialog::
				on_remote_require_password_toggled));
	m_remote_auth_external->signal_toggled().connect(
		sigc::mem_fun(*this,
			&Gobby::InitialDialog::
				on_remote_auth_external_toggled));

	add_button(_("_Close"), Gtk::RESPONSE_CLOSE);
}

std::auto_ptr<Gobby::InitialDialog>
Gobby::InitialDialog::create(Gtk::Window& parent,
                             StatusBar& status_bar,
                             Preferences& preferences,
                             CertificateManager& cert_manager)
{
	Glib::RefPtr<Gtk::Builder> builder =
		Gtk::Builder::create_from_resource(
			"/de/0x539/gobby/ui/initial-dialog.ui");

	InitialDialog* dialog_ptr;
	builder->get_widget_derived("InitialDialog", dialog_ptr);
	std::auto_ptr<InitialDialog> dialog(dialog_ptr);

	dialog->set_transient_for(parent);

	dialog->m_status_bar = &status_bar;
	dialog->m_preferences = &preferences;
	dialog->m_cert_manager = &cert_manager;

	// Set initial values
	dialog->m_name_entry->set_text(preferences.user.name);
	dialog->m_color_button->set_hue(preferences.user.hue);
	dialog->m_remote_allow_connections->set_active(
		preferences.user.allow_remote_access);
	dialog->m_remote_require_password->set_active(
		preferences.user.require_password);
	dialog->m_remote_password_entry->set_text(
		static_cast<std::string>(preferences.user.password));

	// Set initial sensitivity
	dialog->on_remote_allow_connections_toggled();
	dialog->on_remote_require_password_toggled();
	dialog->on_remote_auth_external_toggled();

	return dialog;
}

void Gobby::InitialDialog::on_response(int id)
{
	m_preferences->user.name = m_name_entry->get_text();
	m_preferences->user.hue =
		hue_from_gdk_color(m_color_button->get_color());
	m_preferences->user.allow_remote_access =
		m_remote_allow_connections->get_active();
	m_preferences->user.require_password =
		m_remote_require_password->get_active();
	m_preferences->user.password =
		m_remote_password_entry->get_text();
	m_preferences->security.authentication_enabled = true;
	if(m_remote_auth_self->get_active())
	{
		// The certificate generator takes care of its own:
		new InitialCertGenerator(
			*m_cert_manager, *m_status_bar,
			config_filename("key.pem"),
			config_filename("cert.pem"));
	}
	else
	{
		m_preferences->security.certificate_file =
			m_remote_auth_external_certfile->get_filename();
		m_preferences->security.key_file =
			m_remote_auth_external_keyfile->get_filename();
	}

	hide();
}

void Gobby::InitialDialog::on_remote_allow_connections_toggled()
{
	m_remote_connections_grid->set_sensitive(
		m_remote_allow_connections->get_active());
}

void Gobby::InitialDialog::on_remote_require_password_toggled()
{
	m_password_grid->set_sensitive(
		m_remote_require_password->get_active());
}

void Gobby::InitialDialog::on_remote_auth_external_toggled()
{
	m_certificate_grid->set_sensitive(
		m_remote_auth_external->get_active());
}
