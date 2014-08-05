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

#ifndef _GOBBY_TITLEBAR_HPP_
#define _GOBBY_TITLEBAR_HPP_

#include "core/folder.hpp"
#include "core/sessionview.hpp"

#include <gtkmm/window.h>
#include <sigc++/trackable.h>

namespace Gobby
{

class TitleBar: public sigc::trackable
{
public:
	TitleBar(Gtk::Window& window, const Folder& folder);
	~TitleBar();

private:
	static void on_notify_status_static(GObject* object,
	                                    GParamSpec* pspec,
	                                    gpointer user_data)
	{
		static_cast<TitleBar*>(user_data)->on_notify_status();
	}

	static void on_notify_modified_static(InfBuffer* buffer,
	                                      GParamSpec* pspec,
	                                      gpointer user_data)
	{
		static_cast<TitleBar*>(user_data)->on_notify_modified();
	}

	void on_document_removed(SessionView& view);
	void on_document_changed(SessionView* view);

	void on_notify_status();
	void on_notify_modified();

	void update_title();

	Gtk::Window& m_window;
	const Folder& m_folder;
	SessionView* m_current_view;

	gulong m_notify_status_handler;
	gulong m_modified_changed_handler;
};

}

#endif // _GOBBY_TITLEBAR_HPP_
