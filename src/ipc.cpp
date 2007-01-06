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

// TODO: Split into two files?
#ifdef WIN32
# include <stdexcept>
#endif

#include <glibmm/pattern.h>
#include "ipc.hpp"

namespace
{
#ifdef WIN32
	// Maps HWND back to the HiddenWindow to get the HiddenWindow
	// from the windowproc
	std::map<HWND, Gobby::Ipc::HiddenWindow*> windows;
	ATOM hidden_class = 0;

	LRESULT CALLBACK hidden_window_proc(HWND hwnd,
	                                    UINT message,
	                                    WPARAM wparam,
	                                    LPARAM lparam)
	{
		std::map<HWND, Gobby::Ipc::HiddenWindow*>::const_iterator iter;
		iter = windows.find(hwnd);

		// Needed to continue with window creation. Real
		// initialization is done with WM_CREATE.
		if(message == WM_NCCREATE)
			return TRUE;

		if(message == WM_CREATE)
		{
			// Window has been created, put into windows map
			if(iter != windows.end() )
			{
				throw std::logic_error(
					"Gobby::ipc.cpp::hidden_window_proc:\n"
					"Got WM_CREATE for window in window map"
				);
			}

			const CREATESTRUCT* cs =
				reinterpret_cast<CREATESTRUCT*>(lparam);

			windows[hwnd] = static_cast<Gobby::Ipc::HiddenWindow*>(
				cs->lpCreateParams
			);

			// TODO: Dispatch further?
			return 0;
		}

		if(iter == windows.end() )
		{
			// Window not found in window map. This may occur
			// since some messages are sent before window creation
			// or after window destruction. Ignore them for now.
			return 0;
		}

		if(message == WM_DESTROY)
		{
			windows.erase(hwnd);

			// TODO: Dispatch further?
			return 0;
		}

		return iter->second->message_event().emit(
			message,
			wparam,
			lparam
		);
	}

	static ATOM make_hidden_window_class()
	{
		WNDCLASSEX classex;
		classex.cbSize = sizeof(WNDCLASSEX);
		classex.style = 0;
		classex.lpszClassName = "gobbyHidden";
		classex.lpfnWndProc = hidden_window_proc;
		classex.cbClsExtra = 0;
		classex.cbWndExtra = 0;
		classex.hInstance = GetModuleHandle(NULL);
		classex.hIcon = 0;
		classex.lpszMenuName = NULL;
		classex.hIconSm = 0;
		classex.hbrBackground = NULL;
		classex.hCursor = LoadCursor(NULL, IDC_ARROW);

		ATOM klass = RegisterClassEx(&classex);

		if(klass == 0)
		{
			throw Gobby::Ipc::Error(
				Gobby::Ipc::Error::WINDOW_CREATION_FAILED,
				"Failed to create hidden window class"
			);
		}

		return klass;
	}

	// Find another gobby window
	HWND find_hidden_window(const char* title, HWND ignore)
	{
		// Create window class
		if(hidden_class == 0)
			hidden_class = make_hidden_window_class();

		// Find first hidden window
		HWND wnd = FindWindowEx(
			HWND_MESSAGE,
			NULL,
			MAKEINTRESOURCE(hidden_class),
			NULL // title
		);

		// Get next one if we ought to be ignore this
		if(ignore != NULL && wnd == ignore)
		{
			wnd = FindWindowEx(
				HWND_MESSAGE,
				ignore,
				"gobbyHidden",
				title
			);
		}

		if(wnd != NULL) return wnd;

		throw Gobby::Ipc::Error(
			Gobby::Ipc::Error::NO_REMOTE_INSTANCE,
			"No remote Gobby instance available"
		);
	}
#else
	// Find another gobby process by its socket in $TEMP
	Gobby::Unix::Address find_gobby_addr()
	{
		Glib::PatternSpec spec("gobby_*.sock");
		Glib::Dir dir(Glib::get_tmp_dir() );

		for(Glib::DirIterator it = dir.begin(); it != dir.end(); ++ it)
		{
			std::string file = *it;
			if(spec.match(file) )
			{
				std::string abs_file(
					Glib::build_filename(
						Glib::get_tmp_dir(),
						file
					)
				);

				// TODO: stat the file first to see whether
				// it is a socket or not.
				Gobby::Unix::FileAddress addr(abs_file.c_str());

				// We found a file that looks like a socket
				// of a gobby process. To find out whether it
				// really is we try to connect.
				try
				{
					net6::tcp_client_socket sock(addr);
					return addr;
				}
				catch(net6::error& e)
				{
					// File is no socket (or an old socket
					// from a previous session that is not
					// used anymore).
					unlink(abs_file.c_str() );
				}
			}
		}

		throw Gobby::Ipc::Error(
			Gobby::Ipc::Error::NO_REMOTE_INSTANCE,
			"No remote Gobby instance available"
		);
	}

	Gobby::Unix::Address make_gobby_addr()
	{
		serialise::context<pid_t> ctx;
		std::string file = Glib::build_filename(
			Glib::get_tmp_dir(),
			"gobby_" + ctx.to_string(getpid()) + ".sock"
		);

		return Gobby::Unix::FileAddress(file.c_str() );
	}
#endif

	enum Command  {
		COMMAND_OPEN_FILE
	};
}

Gobby::Ipc::Error::Error(Code error_code, const Glib::ustring& error_message):
	Glib::Error(
		g_quark_from_static_string("GOBBY_IPC_ERROR"),
		static_cast<int>(error_code),
		error_message
	)
{
}

Gobby::Ipc::Error::Code Gobby::Ipc::Error::code() const
{
	return static_cast<Code>(gobject_->code);
}

#ifdef WIN32
Gobby::Ipc::HiddenWindow::HiddenWindow(const char* title):
	m_hwnd(NULL)
{
	if(hidden_class == 0)
		hidden_class = make_hidden_window_class();

	m_hwnd = CreateWindow(
		MAKEINTRESOURCE(hidden_class),
		title,
		0,
		0,0,100,100,
		HWND_MESSAGE,
		NULL,
		GetModuleHandle(NULL),
		this
	);

	if(m_hwnd == NULL)
	{
		throw Error(
			Error::WINDOW_CREATION_FAILED,
			"Failed to create hidden window"
		);
	}

	// TODO: Verify that window is in windows map
}

Gobby::Ipc::HiddenWindow::~HiddenWindow()
{
	DestroyWindow(m_hwnd);
}

Gobby::Ipc::HiddenWindow::signal_message_type
Gobby::Ipc::HiddenWindow::message_event() const
{
	return m_signal_message;
}
#endif

Gobby::Ipc::RemoteInstance::RemoteInstance():
#ifdef WIN32
	m_hwnd(find_hidden_window("LocalInstance", NULL) )
#else
	m_addr(find_gobby_addr() )
#endif
{
}

Gobby::Ipc::RemoteConnection::RemoteConnection(const RemoteInstance& to):
#ifdef WIN32
	m_local_hwnd("RemoteConnection"),
	m_remote_hwnd(to.get_hwnd() )
#else
	m_conn(m_selector)
#endif
{
#ifndef WIN32
	m_conn.connect(to.get_addr());
	m_conn.send_event().connect(
		sigc::mem_fun(m_signal_done, &signal_done_type::emit)
	);
#endif
	// WIN32: No further connection necessary. We could send something
	// like a HELLO message, but when we have the window of the remote
	// application we also have the "connection" since we can send
	// messages to theother gobby process.
}

void Gobby::Ipc::RemoteConnection::send_file(const char* file)
{
#ifdef WIN32
	COPYDATASTRUCT cds;

	cds.dwData = COMMAND_OPEN_FILE;
	cds.cbData = std::strlen(file);

	char* data = new char(cds.cbData);
	std::strncpy(data, file, cds.cbData);
	cds.lpData = data;

	SendMessage(
		m_remote_hwnd,
		WM_COPYDATA,
		reinterpret_cast<WPARAM>(m_local_hwnd.get_hwnd()),
		reinterpret_cast<LPARAM>(&cds)
	);

	delete[] data;

	// SendMessage is synchronousely, so register idle handler that
	// emits signal done to emulate the behaviour on unix where data
	// is transferred asynchronousely
	//
	// TODO: If called multiple times, we get hundreds of idle handlers.
	Glib::signal_idle().connect(
		sigc::bind_return(
			sigc::mem_fun(m_signal_done, &signal_done_type::emit),
			false
		)
	);
#else
	net6::packet pack("gobby_ipc");
	pack << static_cast<int>(COMMAND_OPEN_FILE) << file;
	m_conn.send(pack);
#endif
}

Gobby::Ipc::RemoteConnection::signal_done_type
Gobby::Ipc::RemoteConnection::done_event() const
{
	return m_signal_done;
}

Gobby::Ipc::LocalInstance::LocalInstance():
#ifdef WIN32
	m_hwnd("LocalInstance")
#else
	m_addr(make_gobby_addr()), m_serv(new net6::tcp_server_socket(m_addr))
#endif
{
#ifdef WIN32
	m_hwnd.message_event().connect(
		sigc::mem_fun(*this, &LocalInstance::on_message)
	);
#else
	// Watch for incoming connections
	m_selector.set(*m_serv, net6::IO_INCOMING | net6::IO_ERROR);

	m_serv->io_event().connect(
		sigc::mem_fun(*this, &LocalInstance::on_accept)
	);
#endif
}

Gobby::Ipc::LocalInstance::~LocalInstance()
{
#ifndef WIN32
	for(std::set<Connection*>::iterator iter = m_clients.begin();
	    iter != m_clients.end();
	    ++ iter)
	{
		delete *iter;
	}

	m_selector.set(*m_serv, net6::IO_NONE);

	m_serv.reset(NULL);
	if(unlink(m_addr.get_name().c_str() ) == -1)
	{
		// Could not delete socket file. :(.
	}
#endif
}

Gobby::Ipc::LocalInstance::signal_file_type
Gobby::Ipc::LocalInstance::file_event() const
{
	return m_signal_file;
}

#ifdef WIN32
LRESULT Gobby::Ipc::LocalInstance::on_message(UINT msg,
                                              WPARAM wparam,
                                              LPARAM lparam)
{
	if(msg == WM_COPYDATA)
	{
		const COPYDATASTRUCT* cds =
			reinterpret_cast<COPYDATASTRUCT*>(lparam);

		std::string file;
		switch(static_cast<Command>(cds->dwData) )
		{
		case COMMAND_OPEN_FILE:
			file = std::string(
				static_cast<const char*>(cds->lpData),
				cds->cbData
			);

			// Directly emitting the signal is dangerous from
			// the message handler. We put an idle handler
			// inbetween.
			Glib::signal_idle().connect(
				sigc::bind_return(
					sigc::bind(
						sigc::mem_fun(
							m_signal_file,
							&signal_file_type::emit
						),
						file
					),
					false
				)
			);

			return TRUE;
			break;
		default:
			// TODO: Throw error?
			break;
		}

		return 0;
	}
	else
	{
		// Not handled
		return 0;
	}
}
#else
void Gobby::Ipc::LocalInstance::on_accept(net6::io_condition cond)
{
	if(cond == net6::IO_ERROR)
	{
		throw Error(
			Error::SERVER_ERROR,
			"Error event occured on server socket"
		);
	}

	Unix::Address remote_addr;
	std::auto_ptr<net6::tcp_client_socket> sock(
		m_serv->accept(remote_addr)
	);

	std::auto_ptr<Connection> conn(new Connection(m_selector));
	conn->assign(sock, remote_addr);

	conn->recv_event().connect(
		sigc::mem_fun(*this, &LocalInstance::on_read)
	);

	conn->close_event().connect(
		sigc::bind(
			sigc::mem_fun(*this, &LocalInstance::on_close),
			sigc::ref(*conn)
		)
	);

	m_clients.insert(conn.release());
}

void Gobby::Ipc::LocalInstance::on_read(const net6::packet& pack)
{
	if(pack.get_command() != "gobby_ipc")
	{
		throw net6::bad_value(
			"Got packet whose command is not 'gobby_ipc' in "
			"IPC subsystem"
		);
	}

	Command cmd = static_cast<Command>(pack.get_param(0).as<int>());

	switch(cmd)
	{
	case COMMAND_OPEN_FILE:
		m_signal_file.emit(pack.get_param(1).as<std::string>());
		break;
	default:
		throw net6::bad_value(
			"Got unexpected command in IPC subsystem"
		);
	}
}

void Gobby::Ipc::LocalInstance::on_close(Connection& conn)
{
	m_clients.erase(&conn);
	delete &conn;
}
#endif
