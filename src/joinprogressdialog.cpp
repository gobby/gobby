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

#include <gtkmm/messagedialog.h>
#include <obby/format_string.hpp>
#include "common.hpp"
#include "buffer_wrapper.hpp"
#include "passworddialog.hpp"
#include "joinprogressdialog.hpp"

Gobby::JoinProgressDialog::JoinProgressDialog(Gtk::Window& parent,
                                              Config& config,
                                              const Glib::ustring& hostname,
                                              unsigned int port,
                                              const Glib::ustring& username,
                                              const Gdk::Color& color)
 : ProgressDialog(_("Joining obby session..."), parent), m_config(config),
   m_hostname(hostname), m_port(port), m_username(username), m_color(color)
{
	obby::format_string str("Connecting to %0...");
	str << hostname;
	set_status_text(str.str() );
}

Gobby::JoinProgressDialog::~JoinProgressDialog()
{
}

std::auto_ptr<obby::client_buffer> Gobby::JoinProgressDialog::get_buffer()
{
	return m_buffer;
}

void Gobby::JoinProgressDialog::on_thread()
{
}

void Gobby::JoinProgressDialog::on_done()
{
	// Call base function joining the thread
	ProgressDialog::on_done();

	try
	{
		// Create buffer
		m_buffer.reset(
			new ClientBuffer(
				*static_cast<Gtk::Window*>(get_parent()),
				m_hostname,
				m_port
			)
		);
	}
	catch(net6::error& e)
	{
		// Display error
		display_error(e.what() );
		// Return
		response(Gtk::RESPONSE_CANCEL);
		return;
	}

	m_buffer->welcome_event().connect(
		sigc::mem_fun(*this, &JoinProgressDialog::on_welcome) );
	m_buffer->login_failed_event().connect(
		sigc::mem_fun(*this, &JoinProgressDialog::on_login_failed) );
	m_buffer->global_password_event().connect(
		sigc::mem_fun(*this, &JoinProgressDialog::on_global_password) );
	m_buffer->user_password_event().connect(
		sigc::mem_fun(*this, &JoinProgressDialog::on_user_password) );
	m_buffer->sync_init_event().connect(
		sigc::mem_fun(*this, &JoinProgressDialog::on_sync_init) );
	m_buffer->sync_final_event().connect(
		sigc::mem_fun(*this, &JoinProgressDialog::on_sync_final) );
	m_buffer->close_event().connect(
		sigc::mem_fun(*this, &JoinProgressDialog::on_close) );

	// Wait for welcome packet
	set_status_text(_("Waiting for welcome packet...") );
	set_progress_fraction(1.0/4.0);
}

void Gobby::JoinProgressDialog::on_welcome()
{
	// TODO: Show key ID to user and allow him to deny connection
	// Got welcome packet, send login packet now
	unsigned int red = m_color.get_red() * 255 / 65535;
	unsigned int green = m_color.get_green() * 255 / 65535;
	unsigned int blue = m_color.get_blue() * 255 / 65535;

	m_buffer->login(m_username, red, green, blue);

	// Update status message
	set_status_text(_("Login packet sent, waiting for response...") );
	set_progress_fraction(2.0/4.0);
}

void Gobby::JoinProgressDialog::on_login_failed(obby::login::error error)
{
	// Display error message and cancel connect
	display_error(obby::login::errstring(error) );
	response(Gtk::RESPONSE_CANCEL);
}

bool Gobby::JoinProgressDialog::on_global_password(std::string& password)
{
	bool result = prompt_password(_("Enter session password"), password);
	if(result == false) response(Gtk::RESPONSE_CANCEL);
	return result;
}

bool Gobby::JoinProgressDialog::on_user_password(std::string& password)
{
	bool result = prompt_password(_("Enter user password"), password);
	if(result == false) response(Gtk::RESPONSE_CANCEL);
	return result;
}

void Gobby::JoinProgressDialog::on_sync_init(unsigned int count)
{
	// Update status
	set_status_text(_(
		"Logged in successfully, synchronising session...") );
	set_progress_fraction(3.0/4.0);
}

void Gobby::JoinProgressDialog::on_sync_final()
{
	// Done.
	response(Gtk::RESPONSE_OK);
}

void Gobby::JoinProgressDialog::on_close()
{
	// Connection closed by remote site
	display_error(_("Connection lost") );
	// Cancel login process
	response(Gtk::RESPONSE_CANCEL);
}

void Gobby::JoinProgressDialog::display_error(const Glib::ustring& message)
{
	Gtk::MessageDialog dlg(*this, message, false, Gtk::MESSAGE_ERROR,
	                       Gtk::BUTTONS_OK, true);
	dlg.run();
}

bool Gobby::JoinProgressDialog::prompt_password(const Glib::ustring& label,
                                                std::string& password)
{
	// Setup entry dialog
	PasswordDialog dlg(*this, label, true);

	// Run it
	if(dlg.run() == Gtk::RESPONSE_OK)
	{
		password = dlg.get_password();
		return true;
	}
	else
	{
		return false;
	}
}

