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

#ifndef _GOBBY_GOTODIALOG_HPP_
#define _GOBBY_GOTODIALOG_HPP_

#include "core/folder.hpp"
#include "core/sessionview.hpp"

#include <gtkmm/dialog.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/builder.h>

namespace Gobby
{

class GotoDialog: public Gtk::Dialog
{
private:
	friend class Gtk::Builder;

	GotoDialog(GtkDialog* cobject,
	           const Glib::RefPtr<Gtk::Builder>& builder);
public:
	~GotoDialog();

	static std::unique_ptr<GotoDialog> create(Gtk::Window& parent,
	                                        const Folder& folder);

protected:
	static void on_changed_static(GtkTextBuffer* buffer,
	                              gpointer user_data)
	{
		static_cast<GotoDialog*>(user_data)->on_changed();
	}

	virtual void on_show();
	virtual void on_response(int id);

	void on_document_changed(SessionView* view);
	void on_changed();

	const Folder* m_folder;

	Gtk::SpinButton* m_entry_line;

	TextSessionView* m_current_view;
	gulong m_changed_handler;
};

}

#endif // _GOBBY_GOTODIALOG_HPP_
