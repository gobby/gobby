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

#ifndef _GOBBY_APPLICATION_COMMANDS_HPP_
#define _GOBBY_APPLICATION_COMMANDS_HPP_

#include "dialogs/preferences-dialog.hpp"
#include "core/applicationactions.hpp"
#include "core/filechooser.hpp"
#include "core/preferences.hpp"
#include "core/certificatemanager.hpp"

#include <gtkmm/application.h>
#include <sigc++/trackable.h>

#include <memory>

namespace Gobby
{

class ApplicationCommands: public sigc::trackable
{
public:
	ApplicationCommands(Gtk::Application& application,
	                    const ApplicationActions& actions,
	                    FileChooser& file_chooser,
	                    Preferences& preferences,
	                    CertificateManager& cert_manager);

protected:
	void on_preferences();
	void on_quit();

	Gtk::Application& m_application;
	FileChooser& m_file_chooser;
	Preferences& m_preferences;
	CertificateManager& m_cert_manager;

	std::auto_ptr<PreferencesDialog> m_preferences_dialog;
};

}

#endif // _GOBBY_APPLICATION_COMMANDS_HPP_
