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
#include "io/buffer_wrapper.hpp"
#include "common.hpp"
#include "passworddialog.hpp"
#include "joinprogressdialog.hpp"

Gobby::JoinProgressDialog::JoinProgressDialog(Gtk::Window& parent,
                                              Config& config,
                                              const Glib::ustring& hostname,
                                              unsigned int port,
                                              const Glib::ustring& username,
                                              const Gdk::Color& color)
 : ProgressDialog(_("Joining obby session..."), parent), m_config(config),
   m_hostname(hostname), m_port(port), m_username(username), m_color(color),
   m_got_done(false), m_got_welcome(false)
{
	obby::format_string str("Connecting to %0%...");
	str << hostname;
	set_status_text(str.str() );
}

std::auto_ptr<obby::client_buffer> Gobby::JoinProgressDialog::get_buffer()
{
	return m_buffer;
}

void Gobby::JoinProgressDialog::on_thread(Thread& thread)
{
	// Get initial data from dialog
	lock(thread);

#ifdef WIN32
	Gtk::Window& parent = m_parent;
#endif
	// Connection data
	Glib::ustring hostname = m_hostname; // Remote host name
	unsigned int port = m_port; // TCP port number

	// Dialog may be closed now
	unlock(thread);

	std::auto_ptr<obby::client_buffer> buffer; // Resulting buffer
	Glib::ustring error; // Error message

	// Establish connection
	try
	{
		buffer.reset(
			new obby::io::client_buffer(
#ifdef WIN32
				parent
#endif
			)
		);

		// Install signal handlers (notice that these get called within
		// the main thread)
		buffer->welcome_event().connect(
			sigc::mem_fun(*this, &JoinProgressDialog::on_welcome) );

		buffer->login_failed_event().connect(
			sigc::mem_fun(
				*this,
				&JoinProgressDialog::on_login_failed
			)
		);

		buffer->global_password_event().connect(
			sigc::mem_fun(
				*this,
				&JoinProgressDialog::on_global_password
			)
		);

		buffer->user_password_event().connect(
			sigc::mem_fun(
				*this,
				&JoinProgressDialog::on_user_password
			)
		);

		buffer->sync_init_event().connect(
			sigc::mem_fun(
				*this,
				&JoinProgressDialog::on_sync_init
			)
		);

		buffer->sync_final_event().connect(
			sigc::mem_fun(
				*this,
				&JoinProgressDialog::on_sync_final
			)
		);

		buffer->close_event().connect(
			sigc::mem_fun(*this, &JoinProgressDialog::on_close) );

		// Establish connection
		buffer->connect(hostname, port);
	}
	catch(net6::error& e)
	{
		// Store error message, if any
		error = e.what();
	}

	// Regain lock
	lock(thread);
	// Set resulting buffer
	m_buffer = buffer;
	m_error = error;
	// Unlock before exiting
	unlock(thread);

	// Thread may finish now
}

void Gobby::JoinProgressDialog::on_done()
{
	ProgressDialog::on_done();

	// Did we get an error while connecting?
	if(!m_error.empty() )
	{
		// Display it
		display_error(m_error);
		// Bad response
		response(Gtk::RESPONSE_CANCEL);
	}

	// Thread has established connection, wait for welcome packet
	set_status_text(_("Waiting for welcome packet...") );
	set_progress_fraction(1.0/4.0);
	m_got_done = true;

	// on_welcome may be called before on_done is called if the
	// server replies faster then the thread dispatches, this happens
	// especially with connections to localhost.
	// Recall on_welcome in this case now
	if(m_got_welcome)
		on_welcome();
}

void Gobby::JoinProgressDialog::on_welcome()
{
	m_got_welcome = true;

	// Do nothing if we have not already got the done signal from the
	// thread.
	if(!m_got_done)
		return;

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

