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

#include "application.hpp"
#include "features.hpp"

// Needed to register Gobby resource explicitly:
extern "C" {
#include "gobby-resources.h"
}

#include "util/i18n.hpp"
#include "util/file.hpp"

#include <gtkmm/icontheme.h>
#include <gtkmm/builder.h>

#include <iostream>

#include <libinfinity/common/inf-init.h>

#include <libintl.h>

namespace
{
	std::string gobby_localedir()
	{
#ifdef G_OS_WIN32
		gchar* root =
			g_win32_get_package_installation_directory_of_module(
				NULL);

		gchar* temp = g_build_filename(root, "share", "locale", NULL);
		g_free(root);

		gchar* result = g_win32_locale_filename_from_utf8(temp);
		g_free(temp);

		std::string cpp_result(result);
		g_free(result);

		return cpp_result;
#else
		return GOBBY_LOCALEDIR;
#endif
	}
}

class Gobby::Application::Data
{
public:
	Data();
	~Data();

	// TODO: Does the config object really need to stay around, or can
	// it be thrown away after we have loaded the preferences?
	Config config;
	Preferences preferences;
	CertificateManager certificate_manager;
};

Gobby::Application::Data::Data():
	config(config_filename("config.xml")),
	preferences(config),
	certificate_manager(preferences)
{
}

Gobby::Application::Data::~Data()
{
	preferences.serialize(config);
}

Gobby::Application::Application():
	Gtk::Application("de._0x539.gobby",
	                 Gio::APPLICATION_HANDLES_OPEN)
{
	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE, gobby_localedir().c_str());
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

	// There is no on_handle_local_options default handler in
	// Gio::Application:
	signal_handle_local_options().connect(
		sigc::mem_fun(*this, &Application::on_handle_local_options),
		false);

	// TODO: Here, probably N_(...) should be used for the translatable
	// strings, so that the option entry array can be static. However,
	// in this case the gettext translation domain cannot be set.
	// c.f. bugzilla.gnome.org #736637.
	// TODO: The only purpose of the G_OPTION_REMAINING option is to show
	// the arg_description in the --help output. However, in that case
	// the "open" signal is not emitted, since Glib assumes this is just
	// another option that we are handling ourselves. We could work around
	// it by handling the "command-line" signal instead of the "open"
	// signal.
	const GOptionEntry entries[] = {
		{ "version", 'v', 0, G_OPTION_ARG_NONE, NULL,
		  _("Display version information and exit"), NULL
		}, { "new-instance", 'n', 0, G_OPTION_ARG_NONE, NULL,
		  _("Start a new gobby instance also if there is one "
		     "already running"), NULL
		/*}, { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY,
		     NULL, NULL, N_("[FILE1 or URI1] [FILE2 or URI2] [...]")
		*/}, { NULL }
	};

	g_application_add_main_option_entries(G_APPLICATION(gobj()), entries);
}

Glib::RefPtr<Gobby::Application> Gobby::Application::create()
{
	return Glib::RefPtr<Gobby::Application>(
		new Application);
}

int Gobby::Application::on_handle_local_options(
	const Glib::RefPtr<Glib::VariantDict>& options_dict)
{
	bool display_version;
	if(options_dict->lookup_value("version", display_version))
	{
		std::cout << "Gobby " << PACKAGE_VERSION << std::endl;
		return 0;
	}

	bool new_instance;
	if(options_dict->lookup_value("new-instance", new_instance))
	{
		set_flags(get_flags() | Gio::APPLICATION_NON_UNIQUE);
		options_dict->remove("new-instance");
	}

	// Continue normal processing
	return -1;
}

void Gobby::Application::on_startup()
{
	Gtk::Application::on_startup();

	// Register Gobby resource explicitly -- GCC constructors do not
	// work for executables?
	_gobby_get_resource();

	// TODO: This should be handled by a new commands class,
	// application-commands
	add_action("quit",
	           sigc::mem_fun(*this, &Application::on_quit));
	add_action("preferences",
	           sigc::mem_fun(*this, &Application::on_preferences));

	Glib::RefPtr<Gtk::Builder> builder =
		Gtk::Builder::create_from_resource(
			"/de/0x539/gobby/menu/appmenu.ui");
	Glib::RefPtr<Gio::Menu> menu =
		Glib::RefPtr<Gio::Menu>::cast_dynamic(
			builder->get_object("appmenu"));

	set_app_menu(menu);

	try
	{
		Gtk::Window::set_default_icon_name("gobby-0.5");

		GError* error = NULL;
		if(inf_init(&error) != TRUE)
			throw Glib::Error(error);

		Gtk::IconTheme::get_default()->append_search_path(
			PUBLIC_ICONS_DIR);
		Gtk::IconTheme::get_default()->append_search_path(
			PRIVATE_ICONS_DIR);

		// Allocate the per-application data. This cannot be
		// done earlier, such as in the contrutor, since we only
		// need to do it if we are the primary instance.
		m_data.reset(new Data);

		m_gobby_window = new Gobby::Window(
			m_data->config, m_data->preferences,
			m_data->certificate_manager);

		m_window.reset(m_gobby_window);
		add_window(*m_gobby_window);

		m_window->show();
	}
	catch(const Glib::Exception& ex)
	{
		handle_error(ex.what());
	}
	catch(const std::exception& ex)
	{
		handle_error(ex.what());
	}
}

void Gobby::Application::on_activate()
{
	Gtk::Application::on_activate();

	if(m_window.get())
		m_window->present();
}

void Gobby::Application::on_open(const type_vec_files& files,
                                 const Glib::ustring& hint)
{
	Gtk::Application::on_open(files, hint);

	// If we don't have a window we ignore the file open request. This can
	// for example happen when Gobby was launched with command line
	// arguments, but the Gobby initialization failed.
	if(!m_gobby_window)
		return;

	Operations::file_list non_infinote_files;
	for(type_vec_files::const_iterator iter = files.begin();
	    iter != files.end(); ++iter)
	{
		Glib::RefPtr<Gio::File> file = *iter;
		if(file->get_uri_scheme() == "infinote")
			m_gobby_window->subscribe(file->get_uri());
		else
			non_infinote_files.push_back(file);
	}

	if(!files.empty())
		m_gobby_window->open_files(non_infinote_files);
}

void Gobby::Application::handle_error(const std::string& message)
{
	std::auto_ptr<Gtk::MessageDialog> dialog(
		new Gtk::MessageDialog(
			"Failed to startup Gobby", false,
			Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, false));
	dialog->signal_response().connect(
		sigc::hide(sigc::mem_fun(*dialog, &Gtk::Window::hide)));

	dialog->set_title("Gobby");
	dialog->set_secondary_text(message);
	m_window = dialog;
	add_window(*m_window);

	m_window->show();
}

void Gobby::Application::on_quit()
{
	m_window->hide();
}

void Gobby::Application::on_preferences()
{
	if(!m_gobby_window)
		return;

	m_gobby_window->open_preferences();
}
