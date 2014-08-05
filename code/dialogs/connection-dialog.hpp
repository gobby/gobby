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

#ifndef _GOBBY_CONNECTIONDIALOG_HPP_
#define _GOBBY_CONNECTIONDIALOG_HPP_

#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/box.h>
#include <gtkmm/image.h>

namespace Gobby
{

class ConnectionDialog: public Gtk::Dialog
{
public:
	ConnectionDialog(Gtk::Window& parent);

	Glib::ustring get_host_name() const;

protected:
	virtual void on_show();

	Gtk::HBox m_box;
	Gtk::VBox m_rightbox;
	Gtk::HBox m_promptbox;
        Gtk::Image m_image;
	Gtk::Label m_intro_label;
	Gtk::Label m_prompt_label;
	Gtk::Entry m_entry;
};

}

#endif // _GOBBY_CONNECTIONDIALOG_HPP_

