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

#include "core/filechooser.hpp"
#include "util/i18n.hpp"

#include <glibmm/miscutils.h>
#include <glibmm/convert.h>

Gobby::FileChooser::Dialog::Dialog(Gobby::FileChooser& chooser,
                                   Gtk::Window& parent,
                                   const Glib::ustring& title,
                                   Gtk::FileChooserAction action):
	Gtk::FileChooserDialog(parent, title, action),
	m_chooser(chooser)
{
	// Set defaults depending on action
	switch(action)
	{
	case Gtk::FILE_CHOOSER_ACTION_SAVE:
		add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
		add_button(_("_Save"), Gtk::RESPONSE_ACCEPT);
		set_do_overwrite_confirmation(true);
		break;
	case Gtk::FILE_CHOOSER_ACTION_OPEN:
		add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
		add_button(_("_Open"), Gtk::RESPONSE_ACCEPT);
		break;
	default:
		g_assert_not_reached();
		break;
	}

	set_local_only(false);
	set_current_folder_uri(m_chooser.get_current_folder_uri());
}

Gobby::FileChooser::Dialog::~Dialog()
{
	m_chooser.set_current_folder_uri(get_current_folder_uri());
}

Gobby::FileChooser::FileChooser():
	m_current_folder_uri(Glib::filename_to_uri(Glib::get_current_dir()))
{
}

const std::string& Gobby::FileChooser::get_current_folder_uri() const
{
	return m_current_folder_uri;
}

void Gobby::FileChooser::set_current_folder_uri(const std::string& uri)
{
	m_current_folder_uri = uri;
}

