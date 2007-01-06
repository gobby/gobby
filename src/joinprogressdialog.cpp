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
#include "defaultdialog.hpp"
#include "colorsel.hpp"
#include "passworddialog.hpp"
#include "joinprogressdialog.hpp"

namespace {
	// TODO: How to use std::logical_and and such?
	bool logical_and(bool lhs, bool rhs) { return lhs && rhs; }
	bool logical_or(bool lhs, bool rhs) { return lhs || rhs; }
	bool logical_not(bool arg) { return !arg; }

	typedef bool(*compare_ustring_func)(
		const Glib::ustring&,
		const Glib::ustring&
	);

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
                                          Gtk::Widget& widget,
                                          const Gtk::StockID& icon):
	DefaultDialog(title, parent, true, true), m_table(2, 2), m_info(info),
	m_icon(icon, Gtk::ICON_SIZE_DIALOG)
{
	m_info.set_line_wrap(true);

	m_table.attach(m_icon, 0, 1, 0, 2, Gtk::SHRINK, Gtk::SHRINK);
	m_table.attach(m_info, 1, 2, 0, 1, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
	m_table.attach(widget, 1, 2, 1, 2, Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
	m_table.set_spacings(10);

	get_vbox()->pack_start(m_table, Gtk::PACK_EXPAND_WIDGET);
	get_vbox()->set_spacing(10);
	show_all();

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

	set_border_width(10);

	set_resizable(false);
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
	Gtk::HBox box;
	Gtk::Label label(_("New name:") );
	Gtk::Entry entry;
	entry.set_text(m_username);

	box.pack_start(label, Gtk::PACK_SHRINK);
	box.pack_start(entry, Gtk::PACK_EXPAND_WIDGET);
	box.set_spacing(10);

	Prompt prompt(
		*this,
		obby::login::errstring(net6::login::ERROR_NAME_IN_USE),
		_("Name is already in use. You may choose another name or "
		  "cancel the connection."),
		box,
		Gtk::Stock::DIALOG_QUESTION
	);

	prompt.set_response_sensitive(Gtk::RESPONSE_OK, false);

	// We are using the logical_or and logical_not functions defined above,
	// but the standard library provides functors like std::logical_or, but
	// how to use these?
	sigc::slot<void> changed_slot(
		sigc::compose(
			sigc::bind<0>(
				sigc::mem_fun(
					prompt,
					&DefaultDialog::set_response_sensitive
				),
				Gtk::RESPONSE_OK
			),
			sigc::compose(
				sigc::ptr_fun(&logical_and),
				sigc::compose(
					sigc::bind(
						sigc::ptr_fun(static_cast<
							compare_ustring_func
						>(&Glib::operator!=) ),
						sigc::ref(m_username)
					),
					sigc::mem_fun(
						entry,
						&Gtk::Entry::get_text
					)
				),
				sigc::compose(
					sigc::ptr_fun(&logical_not),
					sigc::compose(
						&Glib::ustring::empty,
						sigc::mem_fun(
							entry,
							&Gtk::Entry::get_text
						)
					)
				)
			)
		)
	);

	entry.signal_changed().connect(changed_slot);
	if(prompt.run() == Gtk::RESPONSE_OK)
	{
		settings.name = entry.get_text();
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
	ColorButton btn;
	btn.set_color(m_color);

	Prompt prompt(
		*this,
		obby::login::errstring(obby::login::ERROR_COLOUR_IN_USE),
		_("Colour is already in use. You may choose another colour or "
		  "cancel the connection."),
		btn,
		Gtk::Stock::DIALOG_QUESTION
	);

	if(prompt.run() == Gtk::RESPONSE_OK)
	{
		settings.colour = gdk_to_obby(btn.get_color() );
		m_color = btn.get_color();
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
	Gtk::HBox box;
	Gtk::Label label(_("Session password:") );
	Gtk::Entry entry;
	entry.set_visibility(false);

	box.pack_start(label, Gtk::PACK_SHRINK);
	box.pack_start(entry, Gtk::PACK_EXPAND_WIDGET);
	box.set_spacing(10);

	Prompt prompt(
		*this,
		obby::login::errstring(
			obby::login::ERROR_WRONG_GLOBAL_PASSWORD
		),
		_("Session password required. You have to type in the "
		  "password to be able to join the obby session."),
		box,
		Gtk::Stock::DIALOG_AUTHENTICATION
	);

	// Do not allow OK if the entry is empty
	prompt.set_response_sensitive(Gtk::RESPONSE_OK, false);
	entry.signal_changed().connect(
		sigc::compose(
			sigc::bind<0>(
				sigc::mem_fun(
					prompt,
					&DefaultDialog::set_response_sensitive
				),
				Gtk::RESPONSE_OK
			),
			sigc::compose(
				sigc::ptr_fun(&logical_not),
				sigc::compose(
					&Glib::ustring::empty,
					sigc::mem_fun(
						entry,
						&Gtk::Entry::get_text
					)
				)
			)
		)
	);

	if(prompt.run() == Gtk::RESPONSE_OK)
	{
		settings.global_password = entry.get_text();
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
	Gtk::Table table;
	Gtk::Label lbl_name(_("New name:"), Gtk::ALIGN_RIGHT);
	Gtk::Label lbl_password(_("User password:"), Gtk::ALIGN_RIGHT);
	Gtk::Entry ent_name;
	Gtk::Entry ent_password;
	ent_name.set_text(m_username);
	ent_password.set_visibility(false);

	table.attach(lbl_name, 0, 1, 0, 1,
		Gtk::SHRINK, Gtk::SHRINK);
	table.attach(lbl_password, 0, 1, 1, 2,
		Gtk::SHRINK, Gtk::SHRINK);
	table.attach(ent_name, 1, 2, 0, 1,
		Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
	table.attach(ent_password, 1, 2, 1, 2,
		Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
	table.set_spacings(5);

	obby::format_string str(
		_("User password for user '%0%' required. You may either "
		  "choose another user name, type in your user password or "
		  "cancel the connection.")
	);

	str << m_username;
	Prompt prompt(
		*this,
		obby::login::errstring(obby::login::ERROR_WRONG_USER_PASSWORD),
		str.str(),
		table,
		Gtk::Stock::DIALOG_AUTHENTICATION
	);

	prompt.set_response_sensitive(Gtk::RESPONSE_OK, false);
	ent_password.grab_focus();

	sigc::slot<void> changed_slot(
		sigc::compose(
			sigc::bind<0>(
				sigc::mem_fun(
					prompt,
					&DefaultDialog::set_response_sensitive
				),
				Gtk::RESPONSE_OK
			),
			sigc::compose(
				sigc::ptr_fun(&logical_and),
				sigc::compose(
					sigc::ptr_fun(&logical_not),
					sigc::compose(
						&Glib::ustring::empty,
						sigc::mem_fun(
							ent_name,
							&Gtk::Entry::
								get_text
						)
					)
				),
				sigc::compose(
					sigc::ptr_fun(&logical_or),
					sigc::compose(
						sigc::bind(
							sigc::ptr_fun(static_cast<
								compare_ustring_func
							>(&Glib::operator!=) ),
							sigc::ref(m_username)
						),
						sigc::mem_fun(
							ent_name,
							&Gtk::Entry::get_text
						)
					),
					sigc::compose(
						sigc::ptr_fun(&logical_not),
						sigc::compose(
							&Glib::ustring::empty,
							sigc::mem_fun(
								ent_password,
								&Gtk::Entry::
									get_text
							)
						)
					)
				)
			)
		)
	);

	ent_name.signal_changed().connect(changed_slot);
	ent_password.signal_changed().connect(changed_slot);

	if(prompt.run() == Gtk::RESPONSE_OK)
	{
		m_username = ent_name.get_text();
		settings.name = ent_name.get_text();
		settings.user_password = ent_password.get_text();
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
