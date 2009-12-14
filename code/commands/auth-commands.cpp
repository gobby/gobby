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

#include <gtkmm/stock.h>

namespace
{
	Glib::ustring prompt_password(Gtk::Window& parent,
	                              Gsasl_session* session)
	{
		InfXmppConnection* xmpp =
			INF_XMPP_CONNECTION(gsasl_session_hook_get(session));
		gchar* remote_id;
		g_object_get(G_OBJECT(xmpp),
			"remote-hostname", &remote_id,
			NULL);
		Glib::ustring remote_id_(remote_id);
		g_free(remote_id);
		Gobby::PasswordDialog dialog(parent, remote_id_);
		dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		/* TODO: Armin does not like the OK button */
		dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);
		if(dialog.run() != Gtk::RESPONSE_ACCEPT)
			return "";
		return dialog.get_password();
	}
}

Gobby::AuthCommands::AuthCommands(Gtk::Window& parent, Browser& browser,
                                  const Preferences& preferences):
	m_parent(parent), m_browser(browser), m_preferences(preferences)
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
}

Gobby::AuthCommands::~AuthCommands()
{
	m_browser.set_gsasl_context(NULL, NULL);
}

int Gobby::AuthCommands::gsasl_callback(Gsasl_session* session,
                                        Gsasl_property prop)
{
	Glib::ustring username = m_preferences.user.name;
	Glib::ustring password;
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
		password = prompt_password(m_parent, session);
		if(password.empty())
			return GSASL_NO_PASSWORD;
		gsasl_property_set(session,
		                   GSASL_PASSWORD,
		                   password.c_str());
		return GSASL_OK;
	default:
		return GSASL_NO_CALLBACK;
	}
}
