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

#ifndef _GOBBY_USER_JOIN_HPP_
#define _GOBBY_USER_JOIN_HPP_

#include "core/nodewatch.hpp"

#include <sigc++/trackable.h>
#include <sigc++/signal.h>

#include <vector>
#include <string>
#include <memory>

#include <libinfinity/common/inf-browser.h>
#include <libinfinity/common/inf-session-proxy.h>
#include <libinfinity/common/inf-request-result.h>
#include <libinfinity/common/inf-user.h>

namespace Gobby
{

class UserJoin: public sigc::trackable
{
public:
	class ParameterProvider
	{
	public:
		ParameterProvider() {}
		virtual ~ParameterProvider() {}

		virtual std::vector<GParameter>
		get_user_join_parameters() = 0;
	};

	typedef sigc::signal<void, InfUser*, const GError*>
		SignalFinished;

	UserJoin(InfBrowser* browser, const InfBrowserIter* iter,
	         InfSessionProxy* proxy,
	         std::auto_ptr<ParameterProvider> param_provider);
	~UserJoin();

	InfSessionProxy* get_proxy() const { return m_proxy; }

	// If both are NULL, the user join is still in progress:
	InfUser* get_user() const { return m_user; }
	const GError* get_error() const { return m_error; }

	SignalFinished signal_finished() const { return m_signal_finished; }
private:
	static void on_synchronization_complete_static(InfSession* session,
	                                               InfXmlConnection* conn,
	                                               gpointer user_data)
	{
		static_cast<UserJoin*>(user_data)->
			on_synchronization_complete();
	}

	static void on_user_join_finished_static(InfRequest* request,
	                                         const InfRequestResult* res,
	                                         const GError* error,
	                                         gpointer user_data)
	{
		InfUser* user = NULL;
		if(error == NULL)
			inf_request_result_get_join_user(res, NULL, &user);
		static_cast<UserJoin*>(user_data)->
			on_user_join_finished(user, error);

	}

	void on_synchronization_complete();
	void on_user_join_finished(InfUser* user, const GError* error);

	void attempt_user_join();
	void user_join_complete(InfUser* user, const GError* error);

	NodeWatch m_node;
	InfSessionProxy* m_proxy;
	std::auto_ptr<ParameterProvider> m_param_provider;
	gulong m_synchronization_complete_handler;

	InfRequest* m_request;

	guint m_retry_index;

	InfUser* m_user;
	GError* m_error;

	SignalFinished m_signal_finished;
};

}

#endif // _GOBBY_USER_JOIN_HPP_
