/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005, 2006 0x539 dev group
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

#ifndef _GOBBY_IPC_HPP_
#define _GOBBY_IPC_HPP_

#ifdef WIN32
# include <windows.h>
#else
# include <set>
# include <net6/packet.hpp>
# include <net6/connection.hpp>
# include "unix.hpp"
# include "gselector.hpp"
#endif

#include <gtkmm/window.h>

namespace Gobby
{

namespace Ipc
{

#ifndef WIN32
typedef net6::connection<GSelector> Connection;
#endif

class Error: public Glib::Error
{
public:
	enum Code {
		NO_REMOTE_INSTANCE, // No remote instance found
		SERVER_ERROR, // Error occured on server socket

		FAILED
	};

	Error(Code error_code, const Glib::ustring& error_message);
	Code code() const;
};

/** @brief Represents a remote gobby instance.
 *
 * A remote gobby instance actually is a gobby instance on the some machine
 * but not _this_ instance.
 */
class RemoteInstance
{
public:
	/** @Brief Finds a remote Gobby instance.
	 *
	 * If no instance has been found, an error with code
	 * Error::NO_REMOTE_INSTANCE is thrown.
	 */
	RemoteInstance();

#ifdef WIN32
	HWND get_hwnd() const { return m_hwnd; }
#else
	const Unix::Address& get_addr() const { return m_addr; }
#endif

private:
#ifdef WIN32
	HWND m_hwnd;
#else
	Unix::Address m_addr;
#endif
};

/** @brief Connection to a remote Gobby instance.
 */
class RemoteConnection: private net6::non_copyable
{
public:
	typedef sigc::signal<void> signal_done_type;

	/** @brief Creates a connection to a remote gobby instance.
	 *
	 *  WM_COPYDATA (IPC implementation on Win32) needs the source
	 *  window from which the message is sent, so we need a window
	 *  reference here.
	 */
	RemoteConnection(Gtk::Window& window,
	                 const RemoteInstance& to);

	/** @brief Sends a file to open by the remote instance.
	 */
	void send_file(const char* file);

	/** @brief Signal that is emitted when all files have been
	 * transmitted to the other process.
	 */
	signal_done_type done_event() const;
private:
#ifdef WIN32
	HWND m_local_hwnd;
	HWND m_remote_hwnd;
#else
	GSelector m_selector;
	Connection m_conn;
#endif

	signal_done_type m_signal_done;
};

/** @brief Represents the local obby instance.
 */
class LocalInstance: public sigc::trackable, private net6::non_copyable
{
public:
	typedef sigc::signal<void, const std::string&> signal_file_type;

	LocalInstance(Gtk::Window& window);
	~LocalInstance();

	/** @brief Signal that is emitted when another instance sent a file
	 * we have to open.
	 */
	signal_file_type file_event() const;
private:
#ifdef WIN32
	HWND m_hwnd;
	Gtk::Window& m_window;

	static GdkFilterReturn on_filter_cb(GdkXEvent* xevent,
	                                    GdkEvent* event,
	                                    gpointer data);

	GdkFilterReturn on_filter(MSG* msg);
#else
	GSelector m_selector;

	Unix::Address m_addr;
	std::auto_ptr<net6::tcp_server_socket> m_serv;
	std::set<Connection*> m_clients;

	void on_accept(net6::io_condition cond);

	void on_read(const net6::packet& pack);
	void on_close(Connection& conn);
#endif

	signal_file_type m_signal_file;
};

} // namespace Ipc

} // namespace Gobby

#endif // _GOBBY_IPC_HPP_
