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
#include <gtkmm/grid.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooserbutton.h>

namespace Gobby
{

class InitialDialog : public Gtk::Dialog
{
private:
	friend class Gtk::Builder;
	InitialDialog(GtkDialog* cobject,
	              const Glib::RefPtr<Gtk::Builder>& builder);

public:
	static std::auto_ptr<InitialDialog>
	create(Gtk::Window& parent, StatusBar& status_bar,
	       Preferences& preferences, CertificateManager& cert_manager);

protected:
	virtual void on_response(int id);

	void on_remote_allow_connections_toggled();
	void on_remote_require_password_toggled();
	void on_remote_auth_external_toggled();

	StatusBar* m_status_bar;
	Preferences* m_preferences;
	CertificateManager* m_cert_manager;

	Gtk::Entry* m_name_entry;
	HueButton* m_color_button;

	Gtk::CheckButton* m_remote_allow_connections;
	Gtk::CheckButton* m_remote_require_password;
	Gtk::Entry* m_remote_password_entry;
	Gtk::RadioButton* m_remote_auth_self;
	Gtk::RadioButton* m_remote_auth_external;
	Gtk::FileChooserButton* m_remote_auth_external_keyfile;
	Gtk::FileChooserButton* m_remote_auth_external_certfile;

	Gtk::Grid* m_remote_connections_grid;
	Gtk::Grid* m_password_grid;
	Gtk::Grid* m_certificate_grid;
};

}

#endif // _GOBBY_INITIALDIALOG_HPP_

