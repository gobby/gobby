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
#include <gtkmm/stock.h>
#include <obby/format_string.hpp>
#include "common.hpp"
#include "colorsel.hpp"
#include "passworddialog.hpp"
#include "document.hpp"
#include "joinprogressdialog.hpp"

namespace
{
	Glib::ustring make_user_password_info(const Glib::ustring& username)
	{
		obby::format_string str(
			Gobby::_(
				"User password for user '%0%' required. You "
				"may either choose another user name, type "
				"in your user password or cancel the "
				"connection."
			)
		);

		str << username.raw();
		return str.str();
	}

	obby::colour gdk_to_obby(const Gdk::Color& color)
	{
		return obby::colour(
			color.get_red() * 255 / 65535,
			color.get_green() * 255 / 65535,
			color.get_blue() * 255 / 65535
		);
	}

	Gdk::Color obby_to_gdk(const obby::colour& colour)
	{
		Gdk::Color color;
		color.set_red(colour.get_red() * 65535 / 255);
		color.set_green(colour.get_green() * 65535 / 255);
		color.set_blue(colour.get_blue() * 65535 / 255);
		return color;
	}
}

Gobby::JoinProgressDialog::Prompt::Prompt(Gtk::Window& parent,
                                          const Glib::ustring& title,
                                          const Glib::ustring& info,
                                          const Gtk::StockID& icon):
	Gtk::Dialog(title, parent, true, true), m_table(2, 2), m_info(info),
	m_icon(icon, Gtk::ICON_SIZE_DIALOG)
{
	m_info.set_line_wrap(true);

	m_table.attach(m_icon, 0, 1, 0, 2, Gtk::SHRINK, Gtk::SHRINK);
	m_table.attach(
		m_info,
		1, 2, 0, 1,
		Gtk::EXPAND | Gtk::FILL,
		Gtk::SHRINK
	);

	m_table.set_spacings(10);

	get_vbox()->pack_start(m_table, Gtk::PACK_EXPAND_WIDGET);
	get_vbox()->set_spacing(10);
	show_all();

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

	set_default_response(Gtk::RESPONSE_OK);
	set_border_width(10);
	set_resizable(false);
}

void Gobby::JoinProgressDialog::Prompt::set_custom_widget(Widget& widget)
{
	m_table.attach(
		widget,
		1, 2, 1, 2,
		Gtk::EXPAND | Gtk::FILL,
		Gtk::SHRINK
	);
}

Gobby::JoinProgressDialog::NamePrompt::
	NamePrompt(Gtk::Window& parent,
	           const Glib::ustring& initial_name):
	Prompt(
		parent,
		obby::login::errstring(net6::login::ERROR_NAME_IN_USE),
		_("Name is already in use. You may choose another name or "
		  "cancel the connection."),
		Gtk::Stock::DIALOG_QUESTION
	),
	m_initial_name(initial_name),
	m_label(_("New name:") )
{
	m_entry.signal_changed().connect(
		sigc::mem_fun(*this, &NamePrompt::on_change)
	);

	m_entry.set_text(m_initial_name);
	m_entry.set_activates_default(true);

	m_box.pack_start(m_label, Gtk::PACK_SHRINK);
	m_box.pack_start(m_entry, Gtk::PACK_EXPAND_WIDGET);
	m_box.set_spacing(10);

	m_box.show_all();
	set_custom_widget(m_box);

	m_entry.grab_focus();
}

Glib::ustring Gobby::JoinProgressDialog::NamePrompt::get_name() const
{
	return m_entry.get_text();
}

void Gobby::JoinProgressDialog::NamePrompt::on_change()
{
	const Glib::ustring name = m_entry.get_text();

	set_response_sensitive(
		Gtk::RESPONSE_OK,
		name != m_initial_name && !name.empty()
	);
}

Gobby::JoinProgressDialog::ColorPrompt::
	ColorPrompt(Gtk::Window& parent,
	            const Gdk::Color& initial_color):
	Prompt(
		parent,
		obby::login::errstring(obby::login::ERROR_COLOUR_IN_USE),
		_("Colour is already in use. You may choose another colour or "
		  "cancel the connection."),
		Gtk::Stock::DIALOG_QUESTION
	)
{
	m_button.set_color(initial_color);
	m_button.show_all();

	set_custom_widget(m_button);
	m_button.grab_focus();
}

Gdk::Color Gobby::JoinProgressDialog::ColorPrompt::get_color() const
{
	return m_button.get_color();
}

Gobby::JoinProgressDialog::SessionPasswordPrompt::
	SessionPasswordPrompt(Gtk::Window& parent):
	Prompt(
		parent,
		obby::login::errstring(
			obby::login::ERROR_WRONG_GLOBAL_PASSWORD
		),
		_("Session password required. You have to type in the "
		  "password to be able to join the obby session."),
		Gtk::Stock::DIALOG_AUTHENTICATION
	),
	m_label(_("Session password:") )
{
	m_entry.signal_changed().connect(
		sigc::mem_fun(*this, &SessionPasswordPrompt::on_change)
	);

	m_entry.set_activates_default(true);
	m_entry.set_visibility(false);

	m_box.pack_start(m_label, Gtk::PACK_SHRINK);
	m_box.pack_start(m_entry, Gtk::PACK_EXPAND_WIDGET);
	m_box.set_spacing(10);

	m_box.show_all();
	set_custom_widget(m_box);
	set_response_sensitive(Gtk::RESPONSE_OK, false);

	m_entry.grab_focus();
}

Glib::ustring Gobby::JoinProgressDialog::SessionPasswordPrompt::
	get_password() const
{
	return m_entry.get_text();
}

void Gobby::JoinProgressDialog::SessionPasswordPrompt::on_change()
{
	set_response_sensitive(Gtk::RESPONSE_OK, !m_entry.get_text().empty() );
}

Gobby::JoinProgressDialog::UserPasswordPrompt::
	UserPasswordPrompt(Gtk::Window& parent,
	                   const Glib::ustring& initial_name):
	Prompt(
		parent,
		obby::login::errstring(obby::login::ERROR_WRONG_USER_PASSWORD),
		make_user_password_info(initial_name),
		Gtk::Stock::DIALOG_AUTHENTICATION
	),
	m_initial_name(initial_name), m_table(2, 2),
	m_lbl_name(_("New name:"), Gtk::ALIGN_RIGHT),
	m_lbl_password(_("User password:"), Gtk::ALIGN_RIGHT)
{
	m_ent_name.set_text(m_initial_name);
	m_ent_password.set_visibility(false);

	m_ent_name.set_activates_default(true);
	m_ent_password.set_activates_default(true);

	m_ent_name.signal_changed().connect(
		sigc::mem_fun(*this, &UserPasswordPrompt::on_change)
	);

	m_ent_password.signal_changed().connect(
		sigc::mem_fun(*this, &UserPasswordPrompt::on_change)
	);

	m_table.attach(
		m_lbl_name,
		0, 1, 0, 1,
		Gtk::SHRINK, Gtk::SHRINK
	);

	m_table.attach(
		m_lbl_password,
		0, 1, 1, 2,
		Gtk::SHRINK, Gtk::SHRINK
	);

	m_table.attach(
		m_ent_name,
		1, 2, 0, 1,
		Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK
	);

	m_table.attach(
		m_ent_password,
		1, 2, 1, 2,
		Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK
	);

	m_table.set_spacings(5);

	set_response_sensitive(Gtk::RESPONSE_OK, false);
	m_ent_password.grab_focus();

	m_table.show_all();
	set_custom_widget(m_table);

	m_ent_password.grab_focus();
}

Glib::ustring Gobby::JoinProgressDialog::UserPasswordPrompt::get_name() const
{
	return m_ent_name.get_text();
}

Glib::ustring Gobby::JoinProgressDialog::UserPasswordPrompt::
	get_password() const
{
	return m_ent_password.get_text();
}

void Gobby::JoinProgressDialog::UserPasswordPrompt::on_change()
{
	const Glib::ustring name = m_ent_name.get_name();
	const Glib::ustring password = m_ent_password.get_name();

	set_response_sensitive(
		Gtk::RESPONSE_OK,
		!name.empty() && (name != m_initial_name || !password.empty())
	);
}

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

	// TODO: Write new username and colour (if any) to config, on destruction!
}

std::auto_ptr<Gobby::ClientBuffer> Gobby::JoinProgressDialog::get_buffer()
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

	std::auto_ptr<ClientBuffer> buffer; // Resulting buffer
	Glib::ustring error; // Error message

	// Establish connection
	try
	{
		buffer.reset(new ClientBuffer);

		buffer->set_document_template(
			ClientBuffer::document_type::template_type(
				*buffer
			)
		);

		// Install signal handlers (notice that those are called within
		// the main thread)
		buffer->welcome_event().connect(
			sigc::mem_fun(*this, &JoinProgressDialog::on_welcome) );

		buffer->login_failed_event().connect(
			sigc::mem_fun(
				*this,
				&JoinProgressDialog::on_login_failed
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
	m_buffer->prompt_name_event().connect(
		sigc::mem_fun(*this, &JoinProgressDialog::on_prompt_name) );

	m_buffer->prompt_colour_event().connect(
		sigc::mem_fun(*this, &JoinProgressDialog::on_prompt_colour) );

	m_buffer->prompt_global_password_event().connect(
		sigc::mem_fun(
			*this,
			&JoinProgressDialog::on_prompt_global_password
		)
	);

	m_buffer->prompt_user_password_event().connect(
		sigc::mem_fun(
			*this,
			&JoinProgressDialog::on_prompt_user_password
		)
	);

	m_buffer->sync_init_event().connect(
		sigc::mem_fun(
			*this,
			&JoinProgressDialog::on_sync_init
		)
	);

	m_buffer->sync_final_event().connect(
		sigc::mem_fun(
			*this,
			&JoinProgressDialog::on_sync_final
		)
	);

	m_buffer->login(m_username, gdk_to_obby(m_color) );

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

bool Gobby::JoinProgressDialog::on_prompt_name(connection_settings& settings)
{
	NamePrompt prompt(*this, m_username);

	if(prompt.run() == Gtk::RESPONSE_OK)
	{
		settings.name = prompt.get_name();
		m_username = settings.name;
		return true;
	}
	else
	{
		response(Gtk::RESPONSE_CANCEL);
		return false;
	}
}

bool Gobby::JoinProgressDialog::on_prompt_colour(connection_settings& settings)
{
	ColorPrompt prompt(*this, m_color);
	if(prompt.run() == Gtk::RESPONSE_OK)
	{
		settings.colour = gdk_to_obby(prompt.get_color() );
		m_color = prompt.get_color();
		return true;
	}
	else
	{
		response(Gtk::RESPONSE_CANCEL);
		return false;
	}
}

bool Gobby::JoinProgressDialog::
	on_prompt_global_password(connection_settings& settings)
{
	SessionPasswordPrompt prompt(*this);
	if(prompt.run() == Gtk::RESPONSE_OK)
	{
		settings.global_password = prompt.get_password();
		return true;
	}
	else
	{
		response(Gtk::RESPONSE_CANCEL);
		return false;
	}
}

bool Gobby::JoinProgressDialog::
	on_prompt_user_password(connection_settings& settings)
{
	UserPasswordPrompt prompt(*this, m_username);
	if(prompt.run() == Gtk::RESPONSE_OK)
	{
		m_username = prompt.get_name();
		settings.name = prompt.get_name();
		settings.user_password = prompt.get_password();
		return true;
	}
	else
	{
		response(Gtk::RESPONSE_CANCEL);
		return false;
	}
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

void Gobby::JoinProgressDialog::on_response(int response_id)
{
	m_config["session"]["name"].set(m_username);
	m_config["session"]["color"].set(m_color);
}
