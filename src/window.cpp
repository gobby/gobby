/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005-2006 0x539 dev group
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

#include "features.hpp"
#include "window.hpp"

#include "core/docwindow.hpp"
#include "core/iconmanager.hpp"
#include "core/noteplugin.hpp"
#include "core/closableframe.hpp"

#include "util/i18n.hpp"

#include <gtkmm/frame.h>

Gobby::Window::Window(const IconManager& icon_mgr, Config& config):
	Gtk::Window(Gtk::WINDOW_TOPLEVEL), m_config(config),
	m_lang_manager(gtk_source_language_manager_get_default()),
	m_preferences(m_config), m_icon_mgr(icon_mgr),
	m_header(m_preferences, m_lang_manager),
	m_folder(m_preferences, m_lang_manager),
	m_statusbar(m_folder, m_preferences),
	m_browser(*this, Plugins::TEXT, m_statusbar, m_preferences),
	m_info_storage(INF_GTK_BROWSER_MODEL(m_browser.get_store())),
	m_operations(m_info_storage, m_statusbar), 
	m_commands_browser(m_browser, m_folder, m_info_storage, m_statusbar,
	                   m_preferences),
	m_commands_browser_context(*this, m_browser, m_file_chooser,
	                           m_operations, m_preferences),
	m_commands_folder(m_folder),
	m_commands_file(*this, m_header, m_browser, m_folder, m_file_chooser,
	                m_operations, m_info_storage, m_preferences),
	m_commands_edit(*this, m_header, m_folder, m_statusbar,
	                m_preferences),
	m_commands_view(m_header, m_folder, m_preferences),
	m_commands_help(*this, m_header, m_icon_mgr)
{
	m_header.show();
	m_browser.show();
	m_folder.show();

	// Build UI
	add_accel_group(m_header.get_accel_group() );

	Gtk::Frame* frame_browser = Gtk::manage(new ClosableFrame(
		_("Document Browser"), IconManager::STOCK_DOCLIST,
		m_preferences.appearance.show_browser));
	frame_browser->set_shadow_type(Gtk::SHADOW_IN);
	frame_browser->add(m_browser);
	// frame_browser manages visibility itself

	Gtk::Frame* frame_text = Gtk::manage(new Gtk::Frame);
	frame_text->set_shadow_type(Gtk::SHADOW_IN);
	frame_text->add(m_folder);
	frame_text->show();

	m_paned.pack1(*frame_browser, false, false);
	m_paned.pack2(*frame_text, true, false);
	m_paned.show();

	m_mainbox.pack_start(m_header, Gtk::PACK_SHRINK);
	m_mainbox.pack_start(m_paned, Gtk::PACK_EXPAND_WIDGET);
	m_mainbox.pack_start(m_statusbar, Gtk::PACK_SHRINK);
	m_mainbox.show();

	add(m_mainbox);

	set_title("Gobby");
	set_default_size(800, 600);
}

Gobby::Window::~Window()
{
	// Serialise preferences into config
	m_preferences.serialise(m_config);
}

bool Gobby::Window::on_delete_event(GdkEventAny* event)
{
#if 0
	if(m_buffer.get() == NULL) return false;
	if(!m_buffer->is_open() ) return false;

	Gtk::MessageDialog dlg(
		*this,
		_("You are still connected to a session"),
		false,
		Gtk::MESSAGE_WARNING,
		Gtk::BUTTONS_NONE,
		true
	);

	dlg.set_secondary_text(
		_("Do you want to close Gobby nevertheless?")
	);

	Gtk::Image* img = Gtk::manage(new Gtk::Image(Gtk::Stock::CANCEL,
	                                             Gtk::ICON_SIZE_BUTTON));
	Gtk::Button* cancel_button
		= dlg.add_button(_("C_ancel"), Gtk::RESPONSE_CANCEL);
	dlg.add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_YES);
	cancel_button->set_image(*img);
	cancel_button->grab_focus();

	return dlg.run() != Gtk::RESPONSE_YES;
#endif
	return false;
}

void Gobby::Window::on_realize()
{
	Gtk::Window::on_realize();

	m_paned.set_position(m_paned.get_width() * 2 / 5);
}

void Gobby::Window::on_show()
{
	Gtk::Window::on_show();

	if(!m_config.get_root()["initial"].get_value<bool>("run", false))
	{
		m_initial_dlg.reset(new InitialDialog(*this, m_preferences,
		                                      m_icon_mgr));
		m_initial_dlg->present();
		m_initial_dlg->signal_hide().connect(
			sigc::mem_fun(*this,
			              &Window::on_initial_dialog_hide));
	}
}

void Gobby::Window::on_initial_dialog_hide()
{
	m_initial_dlg.reset(NULL);
	// Don't show again
	m_config.get_root()["initial"].set_value("run", true);
}
