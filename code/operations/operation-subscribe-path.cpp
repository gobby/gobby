/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
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

#include "operations/operation-subscribe-path.hpp"
#include "util/i18n.hpp"
#include "util/uri.hpp"

namespace
{
	std::vector<std::string> split_path(const std::string& path)
	{
		std::vector<std::string> result;
		if(path.empty())
			return result;

		if(path[0] != '/')
		{
			throw std::runtime_error(
				Glib::ustring::compose(
					Gobby::_("Invalid path: \"%1\""),
					path));
		}

		std::string::size_type prev = 1, pos;
		while( (pos = path.find('/', prev)) != std::string::npos)
		{
			std::string component = path.substr(prev, pos - prev);
			if(component.empty())
			{
				throw std::runtime_error(
					Glib::ustring::compose(
						Gobby::_("Invalid path "
						         "component: \"%1\""),
						component));
			}

			result.push_back(component);
			prev = pos + 1;
		}

		// Trailing '/' is allowed
		std::string component = path.substr(prev);
		if(!component.empty())
			result.push_back(component);

		return result;
	}

	std::string make_path_string(const std::vector<std::string>& path)
	{
		std::string result;

		for(std::vector<std::string>::const_iterator iter =
			path.begin();
		    iter != path.end(); ++iter)
		{
			result += "/";
			result += *iter;
		}

		return result;
	}
}

Gobby::OperationSubscribePath::OperationSubscribePath(Operations& operations,
                                                      Folder& folder,
                                                      const std::string& uri):
	Operation(operations), m_folder(folder), m_browser(NULL),
	m_target(uri), m_request(NULL), m_notify_status_id(0),
	m_message_handle(get_status_bar().invalid_handle())
{
}

Gobby::OperationSubscribePath::OperationSubscribePath(Operations& operations,
                                                      Folder& folder,
                                                      InfBrowser* inf_browser,
                                                      const std::string& p):
	Operation(operations), m_folder(folder), m_browser(inf_browser),
	m_target(p), m_request(NULL), m_notify_status_id(0),
	m_message_handle(get_status_bar().invalid_handle())
{
}

Gobby::OperationSubscribePath::~OperationSubscribePath()
{
	if(m_request != NULL)
	{
		g_signal_handlers_disconnect_by_func(
			G_OBJECT(m_request),
			(gpointer)G_CALLBACK(on_subscribe_finished_static),
			this);

		g_signal_handlers_disconnect_by_func(
			G_OBJECT(m_request),
			(gpointer)G_CALLBACK(on_explore_finished_static),
			this);
	}

	if(m_notify_status_id != 0)
		g_signal_handler_disconnect(m_browser, m_notify_status_id);

	if(m_message_handle != get_status_bar().invalid_handle())
		get_status_bar().remove_message(m_message_handle);
}

void Gobby::OperationSubscribePath::start()
{
	try
	{
		if(m_browser)
		{
			m_path = split_path(m_target);

			m_message_handle = get_status_bar().add_info_message(
				Glib::ustring::compose(
					_("Subscribing to \"%1\"..."),
					m_target));

			start_with_browser();
		}
		else
		{
			start_without_browser();
		}
	}
	catch(const std::exception& ex)
	{
		get_status_bar().add_error_message(
			Glib::ustring::compose(
				_("Failed to connect to \"%1\""), m_target),
			ex.what());

		fail();
	}
}

void Gobby::OperationSubscribePath::start_without_browser()
{
	std::string scheme, netloc, path;
	parse_uri(m_target, scheme, netloc, path);
	if(scheme != "infinote")
	{
		throw std::runtime_error(
			Glib::ustring::compose(
				_("URI scheme \"%1\" not supported"),
				scheme));
	}

	m_path = split_path(path);

	std::string hostname, service;
	unsigned int device_index;

	parse_netloc(netloc, hostname, service, device_index);

	if(path.empty())
	{
		m_message_handle = get_status_bar().add_info_message(
			Glib::ustring::compose(
				_("Connecting to \"%1\"..."),
				m_target));
	}
	else
	{
		m_message_handle = get_status_bar().add_info_message(
			Glib::ustring::compose(
				_("Subscribing to \"%1\"..."),
				m_target));
	}

	m_resolve_handle = Gobby::resolve(
		hostname, service,
		sigc::bind(
			sigc::mem_fun(
				*this,
				&OperationSubscribePath::on_resolv_done),
			hostname, device_index),
		sigc::bind(
			sigc::mem_fun(
				*this,
				&OperationSubscribePath::on_resolv_error),
			hostname));
}

void Gobby::OperationSubscribePath::start_with_browser()
{
	InfBrowserStatus status;
	g_object_get(G_OBJECT(m_browser), "status", &status, NULL);

	if(status == INF_BROWSER_OPEN)
	{
		inf_browser_get_root(m_browser, &m_path_iter);
		m_path_index = 0;

		explore();
	}
	else
	{
		m_notify_status_id = g_signal_connect(
			G_OBJECT(m_browser), "notify::status",
			G_CALLBACK(on_notify_status_static), this);
	}
}

void Gobby::OperationSubscribePath::explore()
{
	if(m_path_index == m_path.size())
	{
		// We are done
		get_browser().set_selected(m_browser, &m_path_iter);
		if(inf_browser_is_subdirectory(m_browser, &m_path_iter))
		{
			finish();
		}
		else
		{
			InfSessionProxy* proxy =
				inf_browser_get_session(
					m_browser, &m_path_iter);

			if(proxy != NULL)
			{
				InfSession* session;
				g_object_get(G_OBJECT(proxy),
				             "session", &session, NULL);
				SessionView* view =
					m_folder.lookup_document(session);
				g_object_unref(session);

				g_assert(view != NULL);
				m_folder.switch_to_document(*view);

				finish();
			}
			else
			{
				make_subscribe_request();
			}
		}
	}
	else
	{
		if(inf_browser_is_subdirectory(m_browser, &m_path_iter))
		{
			// This is a subdirectory node. Explore it.
			if(inf_browser_get_explored(m_browser, &m_path_iter))
			{
				descend();
			}
			else
			{
				make_explore_request();
			}
		}
		else
		{
			// This is a leaf node. This is an error, since we
			// did not yet end up at the end of the path.
			get_browser().set_selected(m_browser, &m_path_iter);

			get_status_bar().remove_message(m_message_handle);
			get_status_bar().add_error_message(
				Glib::ustring::compose(
					_("Could not subscribe to \"%1\""),
					m_target),
				Glib::ustring::compose(
					_("Path \"%1\" does not exist"),
					make_path_string(m_path)));
			fail();
		}
	}
}

void Gobby::OperationSubscribePath::descend()
{
	g_assert(m_path_index < m_path.size());
	g_assert(inf_browser_is_subdirectory(m_browser, &m_path_iter));
	g_assert(inf_browser_get_explored(m_browser, &m_path_iter));

	if(inf_browser_get_child(m_browser, &m_path_iter))
	{
		do
		{
			const char* name = inf_browser_get_node_name(
				m_browser, &m_path_iter);
			if(m_path[m_path_index] == name)
			{
				++m_path_index;
				explore();
				return;
			}
		} while(inf_browser_get_next(m_browser, &m_path_iter));
	}

	// Cannot find child
	get_status_bar().remove_message(m_message_handle);
	m_message_handle = get_status_bar().invalid_handle();

	get_status_bar().add_error_message(
		Glib::ustring::compose(
			_("Could not subscribe to \"%1\""), m_target),
		Glib::ustring::compose(
			_("Path \"%1\" does not exist"),
			make_path_string(m_path)));
	fail();
}

void Gobby::OperationSubscribePath::make_explore_request()
{
	g_assert(m_request == NULL);

	m_request = INF_NODE_REQUEST(
		inf_browser_get_pending_request(
			m_browser, &m_path_iter,
			"explore-node"));

	if(m_request == NULL)
	{
		m_request = INF_NODE_REQUEST(
			inf_browser_explore(
				m_browser,
				&m_path_iter,
				on_explore_finished_static, // ???
				this));
	}
	else
	{
		g_signal_connect(
			G_OBJECT(m_request),
			"finished",
			G_CALLBACK(on_explore_finished_static), // ???
			this);
	}
}

void Gobby::OperationSubscribePath::make_subscribe_request()
{
	g_assert(m_request == NULL);

	m_request = INF_NODE_REQUEST(
		inf_browser_get_pending_request(
			m_browser, &m_path_iter,
			"subscribe-session"));

	if(m_request == NULL)
	{
		m_request = inf_browser_subscribe(
			m_browser, &m_path_iter,
			on_subscribe_finished_static,
			this);
	}
	else
	{
		g_signal_connect(
			G_OBJECT(m_request),
			"finished",
			G_CALLBACK(on_subscribe_finished_static), // ???
			this);
	}
}

void Gobby::OperationSubscribePath::on_resolv_done(
	const ResolvHandle* handle, const InfIpAddress* address, guint port,
	const std::string& hostname, unsigned int device_index)
{
	try
	{
		m_browser = get_browser().connect_to_host(
			address, port, device_index, hostname);
		g_assert(m_browser != NULL);

		// From here, go on as if we started from the 2nd destructor
		start_with_browser();
	}
	catch(const std::exception& ex)
	{
		if(m_path.empty())
		{
			get_status_bar().add_error_message(
				Glib::ustring::compose(
					_("Failed to connect to \"%1\""),
					m_target),
				ex.what());
		}
		else
		{
			get_status_bar().add_error_message(
				Glib::ustring::compose(
					_("Could not subscribe to \"%1\""),
					m_target),
				ex.what());
		}

		fail();
	}
}

void Gobby::OperationSubscribePath::on_resolv_error(
	const ResolvHandle* handle, const std::runtime_error& error,
	const std::string& hostname)
{
	get_status_bar().remove_message(m_message_handle);
	m_message_handle = get_status_bar().invalid_handle();

	if(m_path.empty())
	{
		get_status_bar().add_error_message(
			Glib::ustring::compose(
				_("Failed to connect to \"%1\""), m_target),
			Glib::ustring::compose(
				_("Failed to resolve \"%1\": %2"),
				hostname, error.what()));
	}
	else
	{
		get_status_bar().add_error_message(
			Glib::ustring::compose(
				_("Could not subscribe to \"%1\""), m_target),
			Glib::ustring::compose(
				_("Failed to resolve \"%1\": %2"),
				hostname, error.what()));
	}

	fail();
}

void Gobby::OperationSubscribePath::on_notify_status()
{
	InfBrowserStatus status;
	g_object_get(G_OBJECT(m_browser), "status", &status, NULL);

	switch(status)
	{
	case INF_BROWSER_OPEN:
		g_signal_handler_disconnect(m_browser, m_notify_status_id);
		m_notify_status_id = 0;

		inf_browser_get_root(m_browser, &m_path_iter);
		m_path_index = 0;

		explore();
		break;
	case INF_BROWSER_OPENING:
		break;
	case INF_BROWSER_CLOSED:
		// Don't set an error message, the user will already be
		// notified by the closed browser.
		fail();
		break;
	default:
		g_assert_not_reached();
		break;
	}
}

void Gobby::OperationSubscribePath::on_explore_finished(
	const InfBrowserIter* iter, const GError* error)
{
	m_request = NULL;

	if(error != NULL)
	{
		get_status_bar().add_error_message(
			Glib::ustring::compose(
				_("Could not subscribe to \"%1\""), m_target),
			error->message);

		fail();
	}
	else
	{
		descend();
	}
}

void Gobby::OperationSubscribePath::on_subscribe_finished(
	const InfBrowserIter* iter, const GError* error)
{
	m_request = NULL;

	if(error != NULL)
	{
		get_status_bar().add_error_message(
			Glib::ustring::compose(
				_("Could not subscribe to \"%1\""), m_target),
			error->message);

		fail();
	}

	/* From this point on subscription-commands takes over */
	finish();
}
