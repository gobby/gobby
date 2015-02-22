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

#include "commands/application-commands.hpp"

Gobby::ApplicationCommands::ApplicationCommands(
	Gtk::Application& application,
        const ApplicationActions& actions,
        FileChooser& file_chooser,
	Preferences& preferences,
	CertificateManager& cert_manager)
:
	m_application(application),
	m_file_chooser(file_chooser),
	m_preferences(preferences),
	m_cert_manager(cert_manager)
{
	actions.preferences->signal_activate().connect(
		sigc::hide(sigc::mem_fun(
			*this, &ApplicationCommands::on_preferences)));
	actions.quit->signal_activate().connect(
		sigc::hide(sigc::mem_fun(
			*this, &ApplicationCommands::on_quit)));
}

void Gobby::ApplicationCommands::on_preferences()
{
	Gtk::Window* parent = m_application.get_windows()[0];

	if(!m_preferences_dialog.get())
	{
		m_preferences_dialog.reset(
			new PreferencesDialog(*parent, m_file_chooser,
			                      m_preferences, m_cert_manager));
	}

	m_preferences_dialog->present();
}

void Gobby::ApplicationCommands::on_quit()
{
	// TODO: m_application.quit() produces a memory leak
	// since it results in the Gtk::Application destructor not running,
	// which will cause the settings not to be saved.
	//m_application.quit();

	// Instead, hide all windows, which will cause a clean exit.
	std::vector<Gtk::Window*> windows = m_application.get_windows();
	for(std::vector<Gtk::Window*>::iterator iter = windows.begin();
	    iter != windows.end(); ++iter)
	{
		(*iter)->hide();
	}
}
