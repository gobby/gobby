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

#ifndef _GOBBY_INITIALDIALOG_HPP_
#define _GOBBY_INITIALDIALOG_HPP_

#include "core/preferences.hpp"
#include "core/iconmanager.hpp"
#include "core/huebutton.hpp"

#include <gtkmm/dialog.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>
#include <gtkmm/entry.h>
#include <gtkmm/table.h>
#include <gtkmm/filechooserbutton.h>

namespace Gobby
{

class InitialDialog : public Gtk::Dialog
{
public:
	InitialDialog(Gtk::Window& parent,
	              Preferences& preferences,
	              const IconManager& icon_manager);

protected:
	virtual void on_response(int id);

	void on_remote_allow_connections_toggled();
	void on_remote_require_password_toggled();
	void on_remote_auth_external_toggled();

	Preferences& m_preferences;

	Gtk::VBox m_topbox;
	Gtk::Label m_title;

	Gtk::Image m_image;

	Gtk::VBox m_vbox;
	Gtk::Label m_intro;

	Gtk::Label m_name_label;
	Gtk::Entry m_name_entry;

	Gtk::Label m_color_label;
	HueButton m_color_button;

	Gtk::Table m_user_table;

	Gtk::Label m_remote_label;
	Gtk::CheckButton m_remote_allow_connections;
	Gtk::CheckButton m_remote_require_password;
	Gtk::HBox m_remote_password_box;
	Gtk::Label m_remote_password_label;
	Gtk::Entry m_remote_password_entry;
	Gtk::Label m_remote_auth_label;
	Gtk::RadioButton m_remote_auth_none;
	Gtk::Label m_remote_auth_none_help;
	Gtk::RadioButton m_remote_auth_self;
	Gtk::Label m_remote_auth_self_help;
	Gtk::RadioButton m_remote_auth_external;
	Gtk::Label m_remote_auth_external_help;
	Gtk::Label m_remote_auth_external_keyfile_label;
	Gtk::FileChooserButton m_remote_auth_external_keyfile;
	Gtk::Label m_remote_auth_external_certfile_label;
	Gtk::FileChooserButton m_remote_auth_external_certfile;
	Gtk::Table m_remote_auth_external_table;
	Gtk::VBox m_remote_auth_box;
	Gtk::VBox m_remote_options_box;
	Gtk::VBox m_remote_box;

	Gtk::HBox m_main_box;
};

}

#endif // _GOBBY_INITIALDIALOG_HPP_

