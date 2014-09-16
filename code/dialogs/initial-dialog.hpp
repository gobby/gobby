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

#ifndef _GOBBY_INITIALDIALOG_HPP_
#define _GOBBY_INITIALDIALOG_HPP_

#include "core/certificatemanager.hpp"
#include "core/statusbar.hpp"
#include "core/preferences.hpp"
#include "core/huebutton.hpp"

#include <gtkmm/dialog.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>
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
	              StatusBar& status_bar,
	              Preferences& preferences,
	              CertificateManager& cert_manager);

protected:
	virtual void on_response(int id);

	void on_remote_allow_connections_toggled();
	void on_remote_require_password_toggled();
	void on_remote_auth_external_toggled();

	StatusBar& m_status_bar;
	Preferences& m_preferences;
	CertificateManager& m_cert_manager;

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

