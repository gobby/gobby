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

#ifndef _GOBBY_HOSTPROGRESSDIALOG_HPP_
#define _GOBBY_HOSTPROGRESSDIALOG_HPP_

#include <obby/host_buffer.hpp>
#include "progressdialog.hpp"
#include "config.hpp"

namespace Gobby
{

class HostProgressDialog : public ProgressDialog
{
public:
	HostProgressDialog(Gtk::Window& parent, Config& config,
	                   unsigned int port,
	                   const Glib::ustring& username,
	                   const Gdk::Color& color,
	                   const Glib::ustring& session);

	/** Never call this function twice because the auto_ptr of the
	 * HostDialog will be reset to NULL after having transferred the data
	 * to the caller.
	 */
	std::auto_ptr<obby::host_buffer> get_buffer();

private:
	virtual void on_thread(Thread& thread);

	virtual void on_work();
	virtual void on_done();

	Config& m_config;

	unsigned int m_port;
	Glib::ustring m_username;
	Gdk::Color m_color;
	Glib::ustring m_session;

	Glib::ustring m_error;

	std::auto_ptr<obby::host_buffer> m_buffer;
};

}

#endif // _GOBBY_HOSTPROGRESSDIALOG_HPP_
