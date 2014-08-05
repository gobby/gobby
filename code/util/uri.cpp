/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "util/uri.hpp"
#include "util/i18n.hpp"

#include <glibmm/uriutils.h>

#include <stdexcept>

#ifndef G_OS_WIN32
# include <sys/socket.h>
# include <net/if.h>
#endif

namespace Gobby
{

void parse_uri(const std::string& uri,
               std::string& scheme,
               std::string& netloc,
               std::string& path)
{
	// First, parse the scheme. If there is no scheme found, default
	// to infinote:
	std::string::size_type scheme_delim = uri.find("://");
	if(scheme_delim == std::string::npos)
	{
		scheme = "infinote";
		scheme_delim = 0;
	}
	else
	{
		scheme = uri.substr(0, scheme_delim);

		// Skip ':' and all following '/'
		do { ++scheme_delim; } while(uri[scheme_delim] == '/');
	}

	std::string::size_type path_delim = uri.find('/', scheme_delim);
	if(path_delim == std::string::npos)
	{
		netloc = uri.substr(scheme_delim);
		path.clear();
	}
	else
	{
		netloc = Glib::uri_unescape_string(
			uri.substr(scheme_delim, path_delim - scheme_delim));
		path = Glib::uri_unescape_string(uri.substr(path_delim));
	}
}

void parse_netloc(const std::string& netloc,
                  std::string& hostname,
                  std::string& service,
                  unsigned int& device_index)
{
	std::string str = netloc;
	service = "6523"; // Default
	device_index = 0; // Default

	// Strip away device name
	std::string::size_type pos;
	if( (pos = str.rfind('%')) != std::string::npos)
	{
		std::string device_name = str.substr(pos + 1);
		str.erase(pos);

#ifdef G_OS_WIN32
		// TODO: Implement
		device_index = 0;
#else
		device_index = if_nametoindex(device_name.c_str());
		if(device_index == 0)
		{
			throw std::runtime_error(
				_("Device \"%1\" does not exist"));
		}
#endif
	}

	if(str[0] == '[' && ((pos = str.find(']', 1)) != std::string::npos))
	{
		// Hostname surrounded by '[...]'
		hostname = str.substr(1, pos-1);
		++ pos;
		if(pos < str.length() && str[pos] == ':')
			service = str.substr(pos + 1);
	}
	else
	{
		pos = str.find(':');
		if(pos != std::string::npos)
		{
			hostname = str.substr(0, pos);
			service = str.substr(pos + 1);
		}
		else
		{
			hostname = str;
		}
	}
}

} // namespace Gobby
