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
#include "buffer_wrapper.hpp"
#include "hostprogressdialog.hpp"

Gobby::HostProgressDialog::HostProgressDialog(Gtk::Window& parent,
                                              Config& config,
                                              unsigned int port,
                                              const Glib::ustring& username,
                                              const Gdk::Color& color)
 : ProgressDialog(_("Opening obby session..."), parent), m_config(config),
   m_port(port), m_username(username), m_color(color), m_error("")
{
	set_status_text("Generating RSA key...");
}

Gobby::HostProgressDialog::~HostProgressDialog()
{
}

std::auto_ptr<obby::host_buffer> Gobby::HostProgressDialog::get_buffer()
{
	return m_buffer;
}

void Gobby::HostProgressDialog::on_thread()
{
	// Get color components
	unsigned int red = m_color.get_red() * 255 / 65535;
	unsigned int green = m_color.get_green() * 255 / 65535;
	unsigned int blue = m_color.get_blue() * 255 / 65535;

	try
	{
		// Create buffer
		m_buffer.reset(
			new HostBuffer(
				*static_cast<Gtk::Window*>(get_parent()),
				m_port, m_username,
				red, green, blue
			)
		);
	}
	catch(net6::error& e)
	{
		// Store error, if one occured
		m_error = e.what();
	}
}

void Gobby::HostProgressDialog::on_work()
{
	// Show that operations are in progress
	progress_pulse();
}

void Gobby::HostProgressDialog::on_done()
{
	// Call base function (which joins the thread)
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

