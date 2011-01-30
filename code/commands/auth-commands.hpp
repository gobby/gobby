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
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _GOBBY_AUTH_COMMANDS_HPP_
#define _GOBBY_AUTH_COMMANDS_HPP_

#include "dialogs/password-dialog.hpp"

#include "core/browser.hpp"
#include "core/statusbar.hpp"
#include "core/preferences.hpp"

#include <gtkmm/window.h>
#include <sigc++/trackable.h>

namespace Gobby
{

class AuthCommands: public sigc::trackable
{
public:
	AuthCommands(Gtk::Window& parent,
	             Browser& browser,
	             StatusBar& statusbar,
	             const Preferences& preferences);

	~AuthCommands();

protected:
	static void sasl_callback_static(InfSaslContextSession* session,
	                                 Gsasl_property prop,
					 gpointer session_data,
					 gpointer user_data)
	{
		AuthCommands* auth = static_cast<AuthCommands*>(user_data);

		auth->sasl_callback(
			session, INF_XMPP_CONNECTION(session_data), prop);
	}

	static void set_browser_callback_static(InfGtkBrowserModel*,
	                                       GtkTreePath*,
	                                       GtkTreeIter*,
	                                       InfcBrowser* browser,
	                                       gpointer user_data)
	{
		AuthCommands* auth = static_cast<AuthCommands*>(user_data);
		auth->set_browser_callback(browser);
	}

	static void on_notify_status_static(GObject* connection_obj,
	                                    GParamSpec*,
	                                    gpointer user_data)
	{
		AuthCommands* auth = static_cast<AuthCommands*>(user_data);
		auth->on_notify_status(INF_XMPP_CONNECTION(connection_obj));
	}

	static void browser_error_callback_static(InfcBrowser* browser,
	                                          gpointer error_ptr,
	                                          gpointer user_data)
	{
		AuthCommands* auth = static_cast<AuthCommands*>(user_data);
		GError* error = static_cast<GError*>(error_ptr);

		auth->browser_error_callback(browser, error);
	}

	void sasl_callback(InfSaslContextSession* session,
	                   InfXmppConnection* xmpp,
	                   Gsasl_property prop);

	void set_browser_callback(InfcBrowser* browser);

	void browser_error_callback(InfcBrowser* browser, GError* error);

	void handle_error_detail(InfXmppConnection* xmpp,
	                         const GError* detail_error,
	                         Glib::ustring& old_password,
	                         Glib::ustring& last_password);

	struct RetryInfo {
		unsigned int retries;
		Glib::ustring last_password;
		gulong handle;
		PasswordDialog* password_dialog;
	};

	typedef std::map<InfXmppConnection*, RetryInfo> RetryMap;

	RetryMap::iterator insert_retry_info(InfXmppConnection* xmpp);

	void on_notify_status(InfXmppConnection* connection);
	void on_response(int response_id, InfSaslContextSession* session,
	                 InfXmppConnection* xmpp);

	Gtk::Window& m_parent;
	Browser& m_browser;
	StatusBar& m_statusbar;
	const Preferences& m_preferences;
	InfSaslContext* m_sasl_context;

	RetryMap m_retries;
};

}

#endif // _GOBBY_AUTH_COMMANDS_HPP_
