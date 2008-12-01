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
#include "common.hpp"
#include "hostprogressdialog.hpp"

Gobby::HostProgressDialog::HostProgressDialog(Gtk::Window& parent,
                                              Config& config,
                                              unsigned int port,
                                              const Glib::ustring& username,
                                              const Gdk::Color& color,
                                              const Glib::ustring& session):
	ProgressDialog(_("Opening obby session..."), parent), m_config(config),
	m_port(port), m_username(username), m_color(color), m_session(session),
	m_error("")
{
	set_status_text(_("Opening obby session..."));
}

std::auto_ptr<Gobby::HostBuffer> Gobby::HostProgressDialog::get_buffer()
{
	return m_buffer;
}

void Gobby::HostProgressDialog::on_thread(Thread& thread)
{
	// Lock the thread while retrieving data from the dialog because the
	// dialog may no longer exist.
	lock(thread);

	// Put data that is stored with the dialog onto the stack to be
	// allowed to use it without having locked the thread.
#ifdef WIN32
	Gtk::Window& parent = m_parent; // Parent window
#endif
	Glib::ustring username = m_username; // Local user name
	unsigned int port = m_port; // Port to open the server on

	// Local user colour
	unsigned int red = m_color.get_red() * 255 / 65535;
	unsigned int green = m_color.get_green() * 255 / 65535;
	unsigned int blue = m_color.get_blue() * 255 / 65535;

	// Session to restore
	Glib::ustring session = m_session;

	// Dialog may now be closed
	unlock(thread);

	std::auto_ptr<HostBuffer> buffer; // Resulting obby buffer
	Glib::ustring error; // Error message

	// Try to open the server.
	try
	{
		// Create buffer
		buffer.reset(
			new HostBuffer(
				username,
				obby::colour(red, green, blue)
			)
		);

		buffer->set_document_template(
			HostBuffer::document_type::template_type(
				*buffer
			)
		);

		work(thread);

		// Open the server on the given port
		if(session.empty() )
			buffer->open(port);
		else
			buffer->open(session, port);
	}
	catch(std::exception& e)
	{
		// Store error, if one occured
		error = e.what();
	}

	// Regain lock
	lock(thread);
	// Set resulting buffer
	m_buffer = buffer;
	// ... and error, if any
	m_error = error;
	// Unlock before exiting
	unlock(thread);

	// Resulting data has been transmitted, thread may exit
}

void Gobby::HostProgressDialog::on_work()
{
	// Show that operations are in progress
	progress_pulse();
}

void Gobby::HostProgressDialog::on_done()
{
	// Call base function (which removes references to the thread)
	ProgressDialog::on_done();

	// Show error, if there is one
	if(!m_error.empty() )
	{
		Gtk::MessageDialog dlg(*this, m_error, false,
		                       Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK,
		                       true);
		dlg.run();

		// Respond with CANCEL to indicate the calling function that
		// the creation failed.
		response(Gtk::RESPONSE_CANCEL);
	}
	else
	{
		// Repond with OK to indicate that the server is running.
		response(Gtk::RESPONSE_OK);
	}
}

