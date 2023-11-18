/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
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

#include "commands/auth-commands.hpp"
#include "util/i18n.hpp"

#include <libinfinity/common/inf-xmpp-connection.h>
#include <libinfinity/common/inf-error.h>

namespace
{
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
                                  ConnectionManager& connection_manager,
                                  const Preferences& preferences):
	m_parent(parent),
	m_browser(browser),
	m_statusbar(statusbar),
	m_connection_manager(connection_manager),
	m_preferences(preferences)
{
	GError* error = NULL;
	m_sasl_context = inf_sasl_context_new(&error);

	if(!m_sasl_context)
	{
		std::string error_message =
			std::string("SASL initialization error: ") +
			error->message;
		g_error_free(error);
		throw std::runtime_error(error_message);
	}

	inf_sasl_context_set_callback(
		m_sasl_context, &AuthCommands::sasl_callback_static,
		this, NULL);

	// Set SASL context for new and existing connections:
	m_connection_manager.set_sasl_context(m_sasl_context,
	                                      "ANONYMOUS PLAIN");

	g_signal_connect(
		G_OBJECT(m_browser.get_store()),
		"set-browser",
		G_CALLBACK(&AuthCommands::set_browser_callback_static),
		this);

	// Setup callback for existing browsers
	GtkTreeIter iter;
	GtkTreeModel* model = GTK_TREE_MODEL(m_browser.get_store());
	for(gboolean have_entry = gtk_tree_model_get_iter_first(model, &iter);
	    have_entry == TRUE;
	    have_entry = gtk_tree_model_iter_next(model, &iter))
	{
		InfBrowser* browser;
		gtk_tree_model_get(
			model, &iter,
			INF_GTK_BROWSER_MODEL_COL_BROWSER, &browser, -1);

		if(browser != NULL)
		{
			set_browser_callback(NULL, browser);
			g_object_unref(browser);
		}
	}
}

Gobby::AuthCommands::~AuthCommands()
{
	m_connection_manager.set_sasl_context(NULL, NULL);
	inf_sasl_context_unref(m_sasl_context);

	for(RetryMap::iterator iter = m_retries.begin();
	    iter != m_retries.end(); ++iter)
	{
		g_signal_handler_disconnect(iter->first, iter->second.handle);
	}
}

void Gobby::AuthCommands::set_sasl_error(InfXmppConnection* connection,
                                         const gchar* message)
{
	GError* error = g_error_new_literal(
		inf_authentication_detail_error_quark(),
		INF_AUTHENTICATION_DETAIL_ERROR_AUTHENTICATION_FAILED,
		message
	);

	inf_xmpp_connection_set_sasl_error(connection, error);
	g_error_free(error);
}

void Gobby::AuthCommands::sasl_callback(InfSaslContextSession* session,
                                        InfXmppConnection* xmpp,
                                        Gsasl_property prop)
{
	const Glib::ustring username = m_preferences.user.name;
	const std::string correct_password = m_preferences.user.password;
	const char* password;
	gsize password_len;
	gchar cmp;

	switch(prop)
	{
	case GSASL_ANONYMOUS_TOKEN:
		inf_sasl_context_session_set_property(
			session, GSASL_ANONYMOUS_TOKEN, username.c_str());
		inf_sasl_context_session_continue(session, GSASL_OK);
		break;
	case GSASL_AUTHID:
		inf_sasl_context_session_set_property(
			session, GSASL_AUTHID, username.c_str());
		inf_sasl_context_session_continue(session, GSASL_OK);
		break;
	case GSASL_PASSWORD:
		{
			RetryMap::iterator i = m_retries.find(xmpp);
			if(i == m_retries.end())
				i = insert_retry_info(xmpp);
			RetryInfo& info(i->second);

			if(!info.last_password.empty())
			{
				inf_sasl_context_session_set_property(
					session, GSASL_PASSWORD,
					info.last_password.c_str());

				inf_sasl_context_session_continue(session,
				                                  GSASL_OK);
			}
			else
			{
				// Query user for password
				g_assert(info.password_dialog == NULL);

				gchar* remote_id;
				g_object_get(G_OBJECT(xmpp),
				             "remote-hostname", &remote_id,
					     NULL);
				Glib::ustring remote_id_(remote_id);
				g_free(remote_id);

				std::unique_ptr<PasswordDialog> dialog =
					PasswordDialog::create(
						m_parent, remote_id_,
						info.retries);
				info.password_dialog = dialog.release();
				info.password_dialog->add_button(
					_("_Cancel"), Gtk::RESPONSE_CANCEL);
				info.password_dialog->add_button(
					_("_Ok"), Gtk::RESPONSE_ACCEPT);

				Gtk::Dialog& dlg = *info.password_dialog;
				dlg.signal_response().connect(sigc::bind(
					sigc::mem_fun(
						*this,
						&AuthCommands::on_response),
					session, xmpp));

				info.password_dialog->present();
			}
		}

		break;
	case GSASL_VALIDATE_ANONYMOUS:
		if(m_preferences.user.require_password)
		{
			inf_sasl_context_session_continue(
				session,
				GSASL_AUTHENTICATION_ERROR
			);

			set_sasl_error(xmpp, _("Password required"));
		}
		else
		{
			inf_sasl_context_session_continue(session, GSASL_OK);
		}

		break;
	case GSASL_VALIDATE_SIMPLE:
		password = inf_sasl_context_session_get_property(
			session, GSASL_PASSWORD);

		/* length-independent string compare */
		cmp = 0;
		password_len = strlen(password);
		for(unsigned i = 0; i < correct_password.size(); ++i)
		{
			if(i < password_len)
				cmp |= (password[i] ^ correct_password[i]);
			else
				cmp |= (0x00 ^ correct_password[i]);
		}

		if(password_len != correct_password.size())
			cmp = 0xFF;

		if(cmp != 0)
		{
			inf_sasl_context_session_continue(
				session,
				GSASL_AUTHENTICATION_ERROR
			);

			set_sasl_error(xmpp, _("Incorrect password"));
		}
		else
		{
			inf_sasl_context_session_continue(session, GSASL_OK);
		}

		break;
	default:
		inf_sasl_context_session_continue(session, GSASL_NO_CALLBACK);
		break;
	}
}

void Gobby::AuthCommands::on_response(int response_id,
                                      InfSaslContextSession* session,
                                      InfXmppConnection* xmpp)
{
	RetryMap::iterator i = m_retries.find(xmpp);
	g_assert(i != m_retries.end());
	RetryInfo& info(i->second);

	if(response_id == Gtk::RESPONSE_ACCEPT)
		info.last_password = info.password_dialog->get_password();
	else
		info.last_password = "";

	delete info.password_dialog;
	info.password_dialog = NULL;

	++info.retries;

	if(info.last_password.empty())
	{
		inf_sasl_context_session_continue(session, GSASL_NO_PASSWORD);
	}
	else
	{
		inf_sasl_context_session_set_property(
			session, GSASL_PASSWORD, info.last_password.c_str());
		inf_sasl_context_session_continue(session, GSASL_OK);
	}
}

void Gobby::AuthCommands::set_browser_callback(InfBrowser* old_browser,
                                               InfBrowser* new_browser)
{
	// TODO: Disconnect from the signal on destruction?
	if(new_browser != NULL && INFC_IS_BROWSER(new_browser))
	{
		g_signal_connect(
			G_OBJECT(new_browser),
			"error",
			G_CALLBACK(browser_error_callback_static),
			this);
	}
}

void Gobby::AuthCommands::browser_error_callback(InfcBrowser* browser,
                                                 GError* error)
{
	// The Browser already displays errors inline, but we want
	// auth-related error messages to show up in the status bar.

	InfXmlConnection* connection = infc_browser_get_connection(browser);
	g_assert(INF_IS_XMPP_CONNECTION(connection));

	InfXmppConnection* xmpp = INF_XMPP_CONNECTION(connection);
	RetryMap::iterator iter = m_retries.find(xmpp);
	if(iter == m_retries.end())
		iter = insert_retry_info(xmpp);
	Glib::ustring& last_password(iter->second.last_password);
	Glib::ustring old_password;

	old_password.swap(last_password);

	if(error->domain ==
	     g_quark_from_static_string("INF_XMPP_CONNECTION_AUTH_ERROR"))
	{
		// Authentication failed for some reason, maybe because the
		// server aborted authentication. If we were querying a
		// password then close the dialog now.
		delete iter->second.password_dialog;
		iter->second.password_dialog = NULL;

		const GError* sasl_error =
			inf_xmpp_connection_get_sasl_error(xmpp);
		if(sasl_error != NULL &&
		   sasl_error->domain ==
		     inf_authentication_detail_error_quark())
		{
			handle_error_detail(xmpp, sasl_error,
			                    old_password,
			                    last_password);
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
	else if(error->domain == inf_gsasl_error_quark())
	{
		show_error(error, m_statusbar, connection);
	}
}

void Gobby::AuthCommands::handle_error_detail(InfXmppConnection* xmpp,
                                              const GError* detail_error,
                                              Glib::ustring& old_password,
                                              Glib::ustring& last_password)
{
	GError* error = NULL;
	switch(detail_error->code)
	{
	case INF_AUTHENTICATION_DETAIL_ERROR_AUTHENTICATION_FAILED:
		inf_xmpp_connection_retry_sasl_authentication(xmpp, &error);
		break;
	case INF_AUTHENTICATION_DETAIL_ERROR_TRY_AGAIN:
		old_password.swap(last_password);
		inf_xmpp_connection_retry_sasl_authentication(xmpp, &error);

		break;
	default:
		show_error(detail_error, m_statusbar,
		           INF_XML_CONNECTION(xmpp));
		break;
	}

	if(error)
	{
		show_error(error, m_statusbar,
		           INF_XML_CONNECTION(xmpp));
		g_error_free(error);
	}
}

Gobby::AuthCommands::RetryMap::iterator
Gobby::AuthCommands::insert_retry_info(InfXmppConnection* xmpp)
{
	RetryMap::iterator iter = m_retries.insert(
		std::make_pair(xmpp,
		               RetryInfo())).first;
	iter->second.retries = 0;
	iter->second.handle = g_signal_connect(
		G_OBJECT(xmpp),
		"notify::status",
		G_CALLBACK(on_notify_status_static),
		this);
	iter->second.password_dialog = NULL;

	return iter;
}

void Gobby::AuthCommands::on_notify_status(InfXmppConnection* connection)
{
	InfXmlConnectionStatus status;
	g_object_get(G_OBJECT(connection), "status", &status, NULL);

	if(status != INF_XML_CONNECTION_OPENING)
	{
		RetryMap::iterator iter = m_retries.find(connection);
		g_signal_handler_disconnect(connection, iter->second.handle);
		delete iter->second.password_dialog;
		m_retries.erase(iter);
	}
}
