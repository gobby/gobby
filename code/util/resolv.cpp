/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008, 2009 Armin Burgmeier <armin@arbur.net>
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

#include "util/resolv.hpp"

#include <glibmm/main.h>
#include <glibmm/thread.h>

#ifdef G_OS_WIN32
# include <ws2tcpip.h>
/* We need to include wspiapi.h to support getaddrinfo on Windows 2000.
 * See the MSDN article for getaddrinfo
 * http://msdn.microsoft.com/en-us/library/ms738520(VS.85).aspx
 * and bug #425. */
# include <wspiapi.h>
#else
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
#endif

struct Gobby::ResolvHandle
{
	Glib::ustring hostname;
	Glib::ustring service;
	SlotResolvDone slot_done;
	SlotResolvError slot_error;

	bool cancel;
};

namespace
{
	using namespace Gobby;

	bool on_done(ResolvHandle* handle, InfIpAddress* address, guint port)
	{
		std::auto_ptr<ResolvHandle> auto_handle(handle);
		if(!handle->cancel)
			handle->slot_done(handle, address, port);
		inf_ip_address_free(address);
		return false;
	}

	bool on_error(ResolvHandle* handle, int errcode)
	{
		std::auto_ptr<ResolvHandle> auto_handle(handle);
		if(!handle->cancel)
		{
			std::runtime_error error(gai_strerror(errcode));
			handle->slot_error(handle, error);
		}

		return false;;
	}

	void on_resolv_thread(ResolvHandle* handle)
	{
		addrinfo hint;
#ifdef AI_ADDRCONFIG
		hint.ai_flags = AI_ADDRCONFIG;
#else
		hint.ai_flags = 0;
#endif
		hint.ai_family = AF_UNSPEC;
		hint.ai_socktype = SOCK_STREAM;
		hint.ai_protocol = 0;
		hint.ai_addrlen = 0;
		hint.ai_canonname = NULL;
		hint.ai_addr = NULL;
		hint.ai_next = NULL;

		addrinfo* res = NULL;
		int val = getaddrinfo(handle->hostname.c_str(),
		                      handle->service.c_str(),
		                      &hint, &res);
		if(val != 0)
		{
			g_assert(res == NULL);

			Glib::signal_idle().connect(
				sigc::bind(sigc::ptr_fun(&on_error),
					handle, val));
		}
		else
		{
			g_assert(res != NULL);

			InfIpAddress* addr;
			guint port;

			switch(res->ai_family)
			{
			case AF_INET:
				addr = inf_ip_address_new_raw4(
					reinterpret_cast<sockaddr_in*>(
						res->ai_addr)
					->sin_addr.s_addr);
				port = ntohs(
					reinterpret_cast<sockaddr_in*>(
						res->ai_addr)->sin_port);

				break;
			case AF_INET6:
				addr = inf_ip_address_new_raw6(
					reinterpret_cast<sockaddr_in6*>(
						res->ai_addr)
					->sin6_addr.s6_addr);
				port = ntohs(
					reinterpret_cast<sockaddr_in6*>(
						res->ai_addr)->sin6_port);

				break;
			default:
				g_assert_not_reached();
				break;
			}

			freeaddrinfo(res);

			Glib::signal_idle().connect(
				sigc::bind(sigc::ptr_fun(&on_done),
					handle, addr, port));
		}
	}
}

Gobby::ResolvHandle* Gobby::resolve(const Glib::ustring& hostname,
                                    const Glib::ustring& service,
                                    const SlotResolvDone& slot_done,
                                    const SlotResolvError& slot_error)
{
	std::auto_ptr<ResolvHandle> handle(new ResolvHandle);
	handle->hostname = hostname;
	handle->service = service;
	handle->slot_done = slot_done;
	handle->slot_error = slot_error;
	handle->cancel = false;

	Glib::Thread::create(
		sigc::bind(sigc::ptr_fun(on_resolv_thread), handle.get()),
		false);

	return handle.release();
}

void Gobby::cancel(ResolvHandle* handle)
{
	handle->cancel = TRUE;
}
