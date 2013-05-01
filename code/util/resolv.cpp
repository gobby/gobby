/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2011 Armin Burgmeier <armin@arbur.net>
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
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "util/resolv.hpp"

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
# include <netinet/in.h> /* Required for FreeBSD. See bug #431. */
# include <netdb.h>
#endif

namespace
{
	class Resolver: public Gobby::AsyncOperation
	{
	public:
		Resolver(const Glib::ustring& hostname,
		         const Glib::ustring& service,
		         const Gobby::SlotResolvDone& slot_done,
		         const Gobby::SlotResolvError& slot_error):
			m_hostname(hostname), m_service(service),
			m_slot_done(slot_done), m_slot_error(slot_error),
			m_address(NULL), m_port(0), m_errcode(0)
		{
		}

		~Resolver()
		{
			if(m_address)
				inf_ip_address_free(m_address);
		}

	protected:
		virtual void run();
		virtual void finish();

	private:
		const Glib::ustring m_hostname;
		const Glib::ustring m_service;
		const Gobby::SlotResolvDone m_slot_done;
		const Gobby::SlotResolvError m_slot_error;

		InfIpAddress* m_address;
		guint m_port;
		int m_errcode;
	};

	void Resolver::run()
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
		m_errcode = getaddrinfo(m_hostname.c_str(),
		                        m_service.c_str(),
		                        &hint, &res);
		if(m_errcode != 0)
		{
			g_assert(res == NULL);
		}
		else
		{
			g_assert(res != NULL);

			switch(res->ai_family)
			{
			case AF_INET:
				m_address = inf_ip_address_new_raw4(
					reinterpret_cast<sockaddr_in*>(
						res->ai_addr)
					->sin_addr.s_addr);
				m_port = ntohs(
					reinterpret_cast<sockaddr_in*>(
						res->ai_addr)->sin_port);

				break;
			case AF_INET6:
				m_address = inf_ip_address_new_raw6(
					reinterpret_cast<sockaddr_in6*>(
						res->ai_addr)
					->sin6_addr.s6_addr);
				m_port = ntohs(
					reinterpret_cast<sockaddr_in6*>(
						res->ai_addr)->sin6_port);

				break;
			default:
				g_assert_not_reached();
				break;
			}

			freeaddrinfo(res);
		}
	}

	void Resolver::finish()
	{
		if(m_errcode != 0)
		{
			const std::runtime_error error(
				gai_strerror(m_errcode));
			m_slot_error(get_handle(), error);
		}
		else
		{
			m_slot_done(get_handle(), m_address, m_port);
		}
	}
}

std::auto_ptr<Gobby::ResolvHandle>
Gobby::resolve(const Glib::ustring& hostname,
               const Glib::ustring& service,
               const SlotResolvDone& slot_done,
               const SlotResolvError& slot_error)
{
	std::auto_ptr<AsyncOperation> resolver(new Resolver(
		hostname, service, slot_done, slot_error));
	return AsyncOperation::start(resolver);
}
