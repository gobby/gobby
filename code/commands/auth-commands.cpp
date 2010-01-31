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

#include "commands/auth-commands.hpp"
#include "dialogs/password-dialog.hpp"
#include "util/i18n.hpp"

#include <libinfinity/common/inf-xmpp-connection.h>
#include <libinfinity/common/inf-error.h>

#include <gtkmm/stock.h>

namespace
{
	Glib::ustring prompt_password(Gtk::Window& parent,
	                              InfXmppConnection* xmpp,
	                              unsigned int retry_counter)
	{
		gchar* remote_id;
		g_object_get(G_OBJECT(xmpp),
			"remote-hostname", &remote_id,
			NULL);
		Glib::ustring remote_id_(remote_id);
		g_free(remote_id);
		Gobby::PasswordDialog dialog(parent,
		                             remote_id_,
		                             retry_counter);
		dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

		dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
		if(dialog.run() != Gtk::RESPONSE_ACCEPT)
			return "";
		return dialog.get_password();
	}

	void show_error(const GError* error,
	                Gobby::StatusBar& statusbar,
	                InfXmlConnection* connection)
	{
		gchar* remote;
		g_object_get(connection,
			"remote-hostname", &remote,
			NULL);
		Glib::ustring short_message(Glib::ustring::compose(
			"Authentication failed for \"%1\"", remote));
		g_free(remote);

		if(error->domain ==
		   inf_authentication_detail_error_quark())
		{
			statusbar.add_error_message(
				short_message,
				inf_authentication_detail_strerror(
					InfAuthenticationDetailError(
						error->code)));
		}
		else
		{
			statusbar.add_error_message(
				short_message,
				error->message);
		}
	}
}

Gobby::AuthCommands::AuthCommands(Gtk::Window& parent,
                                  Browser& browser,
                                  StatusBar& statusbar,
                                  const Preferences& preferences):
	m_parent(parent),
	m_browser(browser),
	m_statusbar(statusbar),
	m_preferences(preferences)
{
	int gsasl_status = gsasl_init(&m_gsasl);
	if(gsasl_status != GSASL_OK)
		throw std::runtime_error(
			std::string("gsasl error: ") +
			gsasl_strerror(gsasl_status));
	gsasl_callback_set(m_gsasl, &AuthCommands::gsasl_callback_static);
	gsasl_callback_hook_set(m_gsasl, this);
	g_object_set_data_full(G_OBJECT(m_browser.get_store()),
	                       "Gobby::AuthCommands::m_gsasl",
	                       m_gsasl,
	                       reinterpret_cast<GDestroyNotify>(gsasl_done));
	m_browser.set_gsasl_context(m_gsasl, "ANONYMOUS PLAIN");
	g_signal_connect(
		G_OBJECT(m_browser.get_store()),
		"set-browser",
		G_CALLBACK(&AuthCommands::set_browser_callback_static),
		this);
}

Gobby::AuthCommands::~AuthCommands()
{
	m_browser.set_gsasl_context(NULL, NULL);

	for(RetryMap::iterator iter = m_retries.begin();
	    iter != m_retries.end(); ++iter)
	{
		g_signal_handler_disconnect(iter->first, iter->second.handle);
	}
}

int Gobby::AuthCommands::gsasl_callback(Gsasl_session* session,
                                        Gsasl_property prop)
{
	Glib::ustring username = m_preferences.user.name;
	switch(prop)
	{
	case GSASL_ANONYMOUS_TOKEN:
		gsasl_property_set(session,
		                   GSASL_ANONYMOUS_TOKEN,
		                   username.c_str());
		return GSASL_OK;
	case GSASL_AUTHID:
		gsasl_property_set(session,
		                   GSASL_AUTHID,
		                   username.c_str());
		return GSASL_OK;
	case GSASL_PASSWORD:
		{
			InfXmppConnection* xmpp =
				INF_XMPP_CONNECTION(
					gsasl_session_hook_get(session));
			RetryMap::iterator i = m_retries.find(xmpp);
			if(i == m_retries.end())
			{
				i = m_retries.insert(
					std::make_pair(xmpp,
					               RetryInfo())).first;
				i->second.retries = 0;
				i->second.handle = g_signal_connect(
					G_OBJECT(xmpp),
					"notify::status",
					G_CALLBACK(on_notify_status_static),
					this);
			}
			RetryInfo& info(i->second);

			Glib::ustring password =
				prompt_password(m_parent, xmpp, info.retries);
			++info.retries;

			if(password.empty())
				return GSASL_NO_PASSWORD;
			gsasl_property_set(session,
			                   GSASL_PASSWORD,
			                   password.c_str());
			return GSASL_OK;
		}
	default:
		return GSASL_NO_CALLBACK;
	}
}

void Gobby::AuthCommands::set_browser_callback(InfcBrowser* browser)
{
	g_signal_connect(
		G_OBJECT(browser),
		"error",
		G_CALLBACK(browser_error_callback_static),
		this);
}

void Gobby::AuthCommands::browser_error_callback(InfcBrowser* browser,
                                                 GError* error)
{
	// The Browser already displays errors inline, but we want
	// auth-related error messages to show up in the status bar.

	InfXmlConnection* connection = infc_browser_get_connection(browser);
	g_assert(INF_IS_XMPP_CONNECTION(connection));

	if(error->domain ==
	     g_quark_from_static_string("INF_XMPP_CONNECTION_AUTH_ERROR"))
	{
		InfXmppConnection* xmpp = INF_XMPP_CONNECTION(connection);
		const GError* sasl_error =
			inf_xmpp_connection_get_sasl_error(xmpp);
		if(sasl_error != NULL &&
		   sasl_error->domain ==
		     inf_authentication_detail_error_quark() &&
		   sasl_error->code ==
		     INF_AUTHENTICATION_DETAIL_ERROR_AUTHENTICATION_FAILED)
		{
			GError* my_error = NULL;
			inf_xmpp_connection_retry_sasl_authentication(
				INF_XMPP_CONNECTION(connection), &my_error);
			if(my_error)
			{
				show_error(my_error, m_statusbar, connection);
				g_error_free(my_error);
			}
		}
		else if(sasl_error != NULL)
		{
			show_error(sasl_error, m_statusbar, connection);
		}
		else
		{
			show_error(error, m_statusbar, connection);
		}
	}
	else if(error->domain == inf_gsasl_error_quark() ||
	        error->domain == inf_authentication_detail_error_quark())
	{
		show_error(error, m_statusbar, connection);
	}
}

void Gobby::AuthCommands::on_notify_status(InfXmppConnection* connection)
{
	InfXmlConnectionStatus status;
	g_object_get(G_OBJECT(connection), "status", &status, NULL);
	if(status != INF_XML_CONNECTION_OPENING)
	{
		RetryMap::iterator iter = m_retries.find(connection);
		g_signal_handler_disconnect(connection, iter->second.handle);
		m_retries.erase(iter);
	}
}
