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

#include "commands/help-commands.hpp"
#include "util/i18n.hpp"
#include "features.hpp"

#include <gio/gio.h>
#include <gtkmm/messagedialog.h>

Gobby::HelpCommands::HelpCommands(Gtk::Application& application,
                                  const ApplicationActions& actions):
	m_application(application)
{
#ifndef HAVE_GNOME_DOC_UTILS
	actions.help->set_enabled(false);
#endif

	actions.help->signal_activate().connect(
		sigc::hide(sigc::mem_fun(*this, &HelpCommands::on_contents)));
	actions.about->signal_activate().connect(
		sigc::hide(sigc::mem_fun(*this, &HelpCommands::on_about)));
}

void Gobby::HelpCommands::on_contents()
{
	GError* error = NULL;

	Gtk::Window* parent = m_application.get_windows()[0];

	gtk_show_uri(parent->get_screen()->gobj(),
		"ghelp:gobby",
		GDK_CURRENT_TIME,
		&error);

	if(error == NULL)
		return;

	// Help browser could not be invoked, show an error message to the user.
	Gtk::MessageDialog dlg(*parent, _("There was an error displaying help."),
		false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
	dlg.set_secondary_text(error->message);
	dlg.run();

	g_error_free(error);
}

void Gobby::HelpCommands::on_about()
{
	if(m_about_dialog.get() == NULL)
	{
		Gtk::Window* parent = m_application.get_windows()[0];

		m_about_dialog.reset(new Gtk::AboutDialog);
		m_about_dialog->set_transient_for(*parent);

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
			"Copyright © 2008-2015 Armin Burgmeier");
		m_about_dialog->set_license(_(
			"Permission to use, copy, modify, and/or distribute "
			"this software for any urpose with or without fee is "
			"hereby granted, provided that the above copyright "
			"notice and this permission notice appear in all "
			"copies.\n\n"

			"THE SOFTWARE IS PROVIDED \"AS IS\" AND THE AUTHOR "
			"DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS "
			"SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF "
			"MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE "
			"AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, "
			"OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER "
			"RESULTING FROM LOSS OF USE, DATA OR PROFITS, "
			"WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR "
			"OTHER TORTIOUS ACTION, ARISING OUT OF OR IN "
			"CONNECTION WITH THE USE OR PERFORMANCE OF THIS "
			"SOFTWARE."));
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
