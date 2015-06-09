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

#ifndef _GOBBY_OPENLOCATIONDIALOG_HPP_
#define _GOBBY_OPENLOCATIONDIALOG_HPP_

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>

#include "util/historyentry.hpp"

namespace Gobby
{

class OpenLocationDialog: public Gtk::Dialog
{
private:
	friend class Gtk::Builder;
	OpenLocationDialog(GtkDialog* cobject,
	                   const Glib::RefPtr<Gtk::Builder>& builder);

public:
	static std::auto_ptr<OpenLocationDialog> create(Gtk::Window& parent);

	Glib::ustring get_uri() const;
	void set_uri(const Glib::ustring& uri);

protected:
	virtual void on_response(int response_id);
	virtual void on_show();

	void on_entry_changed();

	HistoryComboBox m_combo;
};

}

#endif // _GOBBY_OPENLOCATIONDIALOG_HPP_

