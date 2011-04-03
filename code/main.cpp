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
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "features.hpp"
#include "window.hpp"

#include "core/iconmanager.hpp"
#include "util/config.hpp"
#include "util/file.hpp"
#include "util/i18n.hpp"

#include <libinfinity/common/inf-init.h>

#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <giomm/init.h>
#include <glibmm/optionentry.h>
#include <glibmm/optiongroup.h>
#include <glibmm/optioncontext.h>

#ifdef WITH_UNIQUE
# include <unique/unique.h>
#endif

#include <libintl.h> // bindtextdomain
#include <iostream>
#include <vector>

namespace
{
	void handle_exception(const Glib::ustring& message)
	{
		Gtk::MessageDialog dlg("Unhandled exception", false,
			Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
		dlg.set_secondary_text(message);
		dlg.run();

		std::cerr << "Unhandled exception: " << message << std::endl;
	}

	const char* _(const char* str)
	{
		return Gobby::_(str);
	}

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

#ifdef WITH_UNIQUE
	int send_message_with_uris(UniqueApp* app,
	                            gint message_id,
	                            const std::vector<Glib::ustring>& uris)
	{
		std::vector<const gchar*> uri_cstrs(uris.size() + 1);
		for(unsigned int i = 0; i < uris.size(); ++i)
			uri_cstrs[i] = uris[i].c_str();

		UniqueMessageData* message = unique_message_data_new();
		unique_message_data_set_uris(
			message, const_cast<gchar**>(&uri_cstrs[0]));
		UniqueResponse response = unique_app_send_message(
			app, message_id, message);
		unique_message_data_free(message);

		if(response == UNIQUE_RESPONSE_OK)
		{
			return 0;
		}
		else
		{
			std::cerr
				<< "error sending URIs to existing gobby "
				   "instance (libunique): "
				<< static_cast<int>(response)
				<< std::endl;
			return -1;
		}
	}

	int my_unique_activate(UniqueApp* app) {
		UniqueResponse response =
			unique_app_send_message(app, UNIQUE_ACTIVATE, NULL);
		if(response != UNIQUE_RESPONSE_OK)
		{
			std::cerr
				<< "error activating existing gobby "
				   "instance (libunique): "
				<< static_cast<int>(response)
				<< std::endl;
			return -1;
		}
		else
		{
			return 0;
		}
	}

	int my_unique_send_file_args(UniqueApp* app,
	                             int argc,
	                             const char* const* argv)
	{
		std::vector<Glib::ustring> uris(argc);
		for(int i = 0; i < argc; ++i)
		{
			uris[i] = Gio::File::create_for_commandline_arg(
					argv[i])->get_uri();
		}
		return send_message_with_uris(app, UNIQUE_OPEN, uris);
	}

	int my_unique_send_hostname_args(
		UniqueApp* app,
		const std::vector<Glib::ustring>& hostnames)
	{
		std::vector<Glib::ustring> uris(hostnames);
		for(unsigned int i = 0; i < uris.size(); ++i)
		{
			uris[i].insert(0, "infinote://");
		}
		return send_message_with_uris(
			app, Gobby::UNIQUE_GOBBY_CONNECT, uris);
	}

	int my_unique_check_other(UniqueApp* app,
	                          int argc, const char* const* argv,
	                          const std::vector<Glib::ustring>& hostnames)
	{
		if(argc == 0 && hostnames.empty())
		{
			return my_unique_activate(app);
		}
		
		if(argc) {
			if(my_unique_send_file_args(app, argc, argv) != 0)
				return -1;
		}

		if(!hostnames.empty()) {
			if (my_unique_send_hostname_args(app, hostnames))
				return -1;
		}

		return 0;
	}
#endif // WITH_UNIQUE
}

int main(int argc, char* argv[]) try
{
	g_thread_init(NULL);
	Gio::init();

	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE, gobby_localedir().c_str());
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");

	bool new_instance = false;
	bool display_version = false;
	std::vector<Glib::ustring> hostnames;

	Glib::OptionGroup opt_group_gobby("gobby",
		_("Gobby options"), _("Options related to Gobby"));
	Glib::OptionEntry opt_version;
	opt_version.set_short_name('v');
	opt_version.set_long_name("version");
	opt_version.set_description(
		_("Display version information and exit"));
	opt_group_gobby.add_entry(opt_version, display_version);

	Glib::OptionEntry opt_new_instance;
	opt_new_instance.set_short_name('n');
	opt_new_instance.set_long_name("new-instance");
	opt_new_instance.set_description(
		_("Also start a new Gobby instance when there is one "
		  "running already"));
	opt_group_gobby.add_entry(opt_new_instance, new_instance);

	Glib::OptionEntry opt_connect;
	opt_connect.set_short_name('c');
	opt_connect.set_long_name("connect");
	opt_connect.set_description(
		_("Connect to given host on startup, can be given multiple times"));
	opt_connect.set_arg_description(_("HOSTNAME"));
	opt_group_gobby.add_entry(opt_connect, hostnames);

	Glib::OptionContext opt_ctx;
	opt_ctx.set_help_enabled(true);
	opt_ctx.set_ignore_unknown_options(false);
	opt_ctx.set_main_group(opt_group_gobby);

	// I would rather like to have Gtk::Main on the stack, but I see
	// no other chance to catch exceptions from the command line option
	// parsing. armin.
	// TODO: Maybe we should parse before initializing GTK+, using
	// Gtk::Main::add_gtk_option_group() with open_default_display set
	// to false.
	std::auto_ptr<Gtk::Main> kit;

	try
	{
		kit.reset(new Gtk::Main(argc, argv, opt_ctx));
	}
	catch(Glib::Exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	if(display_version)
	{
		std::cout << "Gobby " << PACKAGE_VERSION << std::endl;
		return EXIT_SUCCESS;
	}

#ifdef WITH_UNIQUE
	UniqueApp* app = unique_app_new_with_commands(
		"de._0x539.gobby", NULL,
		"UNIQUE_GOBBY_CONNECT", Gobby::UNIQUE_GOBBY_CONNECT,
		NULL);

	if(!new_instance && unique_app_is_running(app))
	{
		int exit_code = my_unique_check_other(
			app,
			argc - 1, argv + 1,
			hostnames);
		g_object_unref(app);
		return exit_code;
	}
#endif // WITH_UNIQUE

	GError* error = NULL;
	if(!inf_init(&error))
	{
		std::string message = error->message;
		g_error_free(error);
		throw std::runtime_error(message);
	}

	Gobby::IconManager icon_manager;

	// Set default icon
	Gtk::Window::set_default_icon_name("gobby-0.5");

	// Read the configuration
	Gobby::Config config(Gobby::config_filename("config.xml"));

	// Create window
	Gobby::Window wnd(
		argc-1,
		argv+1,
		icon_manager,
		config
#ifdef WITH_UNIQUE
		, app
#endif
		);

#ifdef WITH_UNIQUE
	g_object_unref(app);
#endif

	wnd.show();

	for(std::vector<Glib::ustring>::const_iterator i = hostnames.begin();
	    i != hostnames.end();
			++ i)
	{
		wnd.connect_to_host(*i);
	}

	wnd.signal_hide().connect(sigc::ptr_fun(&Gtk::Main::quit) );
	kit->run();

	//inf_deinit();
	return 0;
}
catch(Glib::Exception& e)
{
	handle_exception(e.what() );
}
catch(std::exception& e)
{
	handle_exception(e.what() );
}
