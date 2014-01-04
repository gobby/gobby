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

#include "commands/help-commands.hpp"
#include "util/i18n.hpp"
#include "util/gtk-compat.hpp"

#include <gio/gio.h>
#include <gtkmm/messagedialog.h>

namespace
{
	void url_hook(Gtk::AboutDialog& dialog, const Glib::ustring& link)
	{
		// TODO: Set correct timestamp here, using
		// GdkAppLaunchContext?
		g_app_info_launch_default_for_uri(link.c_str(), NULL, NULL);
	}

	void email_hook(Gtk::AboutDialog& dialog, const Glib::ustring& link)
	{
		// TODO: Set correct timestamp here, using
		// GdkAppLaunchContext?
		g_app_info_launch_default_for_uri(
			("mailto:" + link).c_str(), NULL, NULL);
	}
}

Gobby::HelpCommands::HelpCommands(Gtk::Window& parent, Header& header,
                                  const IconManager& icon_manager):
	m_parent(parent), m_header(header), m_icon_manager(icon_manager)
{
#ifndef USE_GTKMM3
	Gtk::AboutDialog::set_url_hook(sigc::ptr_fun(url_hook));
	Gtk::AboutDialog::set_email_hook(sigc::ptr_fun(email_hook));
#endif

	header.action_help_contents->signal_activate().connect(
		sigc::mem_fun(*this, &HelpCommands::on_contents));
	header.action_help_about->signal_activate().connect(
		sigc::mem_fun(*this, &HelpCommands::on_about));
}

void Gobby::HelpCommands::on_contents()
{
	GError* error = NULL;

	gtk_show_uri(m_parent.get_screen()->gobj(),
		"ghelp:gobby",
		GDK_CURRENT_TIME,
		&error);

	if(error == NULL)
		return;

	// Help browser could not be invoked, show an error message to the user.
	Gtk::MessageDialog dlg(m_parent, _("There was an error displaying help."),
		false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
	dlg.set_secondary_text(error->message);
	dlg.run();

	g_error_free(error);
}

void Gobby::HelpCommands::on_about()
{
	if(m_about_dialog.get() == NULL)
	{
		m_about_dialog.reset(new Gtk::AboutDialog);
		m_about_dialog->set_transient_for(m_parent);

		std::vector<Glib::ustring> artists;
		artists.push_back("Benjamin Herr <ben@0x539.de>");
		artists.push_back("Thomas Glatt <tom@0x539.de>");

		std::vector<Glib::ustring> authors;
		authors.push_back("Armin Burgmeier <armin@arbur.net>");
		authors.push_back("Philipp Kern <phil@0x539.de>");
		authors.push_back("");
		authors.push_back(_("Contributors:"));
		authors.push_back("\tBenjamin Herr <ben@0x539.de>");
		authors.push_back("\tBen Levitt <benjie@gmail.com>");
		authors.push_back("\tGabríel A. Pétursson <gabrielp@simnet.is>");

		std::vector<Glib::ustring> translators;
		translators.push_back(_("British English:"));
		translators.push_back("\tGabríel A. Pétursson <gabrielp@simnet.is>");
		translators.push_back(_("German:"));
		translators.push_back("\tMichael Frey <michael.frey@gmx.ch>");

		Glib::ustring transl = "";
		for(std::vector<Glib::ustring>::iterator i = translators.begin();
		    i != translators.end(); ++i)
		{
			if(i != translators.begin()) transl += "\n";
			transl += *i;
		}

		m_about_dialog->set_artists(artists);
		m_about_dialog->set_authors(authors);
		m_about_dialog->set_translator_credits(transl);
		m_about_dialog->set_copyright(
			"Copyright © 2008-2014 Armin Burgmeier");
		m_about_dialog->set_license(_(
			"This program is free software; you can redistribute "
			"it and/or modify it under the terms of the GNU "
			"General Public License as published by the Free "
			"Software Foundation; either version 2 of the "
			"License, or (at your option) any later version.\n\n"

			"This program is distributed in the hope that it "
			"will be useful,  but WITHOUT ANY WARRANTY; without "
			"even the implied warranty of MERCHANTABILITY or "
			"FITNESS FOR A PARTICULAR PURPOSE. See the GNU "
			"General Public License for more details.\n\n"

			"You should have received a copy of the GNU General "
			"Public License along with this program; if not, "
			"write to the Free Software Foundation, Inc., 51 "
			"Franklin Street, Fifth Floor, Boston, MA  "
                        "02110-1301, USA"));
		m_about_dialog->set_logo_icon_name("gobby-0.5");
		m_about_dialog->set_program_name("Gobby");
		m_about_dialog->set_version(PACKAGE_VERSION);
		m_about_dialog->set_website("http://gobby.0x539.de/");
		m_about_dialog->set_wrap_license(true);

		m_about_dialog->signal_response().connect(
			sigc::mem_fun(
				*this, &HelpCommands::on_about_response));
	}

	m_about_dialog->present();
}

void Gobby::HelpCommands::on_about_response(int id)
{
	m_about_dialog.reset(NULL);
}
