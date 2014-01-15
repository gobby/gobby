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

#ifndef _GOBBY_SELF_HOSTER_HPP_
#define _GOBBY_SELF_HOSTER_HPP_

#include "core/credentialsgenerator.hpp"
#include "core/certificatemanager.hpp"
#include "core/statusbar.hpp"
#include "core/server.hpp"

#include <libinfinity/server/infd-directory.h>
#include <libinfinity/server/infd-xmpp-server.h>
#include <libinfinity/server/infd-server-pool.h>
#include <libinfinity/common/inf-local-publisher.h>
#include <libinfinity/common/inf-io.h>

namespace Gobby
{

class SelfHoster: public sigc::trackable
{
public:
	SelfHoster(InfIo* io, InfLocalPublisher* publisher,
	           StatusBar& status_bar,
	           CertificateManager& cert_manager,
	           const Preferences& preferences);
	~SelfHoster();

protected:
	bool ensure_dh_params();
	void on_dh_params_done(const DHParamsGeneratorHandle* handle,
	                       gnutls_dh_params_t dh_params,
	                       const GError* error);

	void apply_preferences();

	StatusBar& m_status_bar;
	CertificateManager& m_cert_manager;
	const Preferences& m_preferences;

	bool m_dh_params_loaded;

	StatusBar::MessageHandle m_info_handle;
	StatusBar::MessageHandle m_dh_params_message_handle;

	Server m_server;
	InfdDirectory* m_directory;

	std::auto_ptr<DHParamsGeneratorHandle> m_dh_params_handle;
};

}
	
#endif // _GOBBY_SELF_HOSTER_HPP_
