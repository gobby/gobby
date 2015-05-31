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

#ifndef _GOBBY_TABLABEL_HPP_
#define _GOBBY_TABLABEL_HPP_

#include "core/sessionview.hpp"

#include <gtkmm/grid.h>
#include <gtkmm/image.h>

namespace Gobby
{

class Folder;

class TabLabel: public Gtk::Grid
{
public:
	typedef Glib::SignalProxy0<void> SignalCloseRequest;

	TabLabel(Folder& folder, SessionView& view,
	         const Glib::ustring& active_icon_name);
	virtual ~TabLabel();

	SignalCloseRequest signal_close_request()
	{
		return m_button.signal_clicked();
	}

protected:
	static void on_notify_status_static(GObject* object,
	                                    GParamSpec* pspec,
	                                    gpointer user_data)
	{
		static_cast<TabLabel*>(user_data)->on_notify_status();
	}

	static void on_notify_subscription_group_static(GObject* object,
	                                                GParamSpec* pspec,
	                                                gpointer user_data)
	{
		static_cast<TabLabel*>(user_data)->
			on_notify_subscription_group();
	}

	void on_folder_document_changed(SessionView* view);

	// Can be overriden by derived classes:
	virtual void on_active_user_changed(InfUser* user);
	virtual void on_notify_status();
	virtual void on_notify_subscription_group();
	virtual void on_activate();

	// To be called by derived classes:
	void set_changed();

	Folder& m_folder;
	SessionView& m_view;

	Gtk::Image m_icon;
	Gtk::Label m_title;
	CloseButton m_button;

	bool m_changed;

private:
	void update_icon();
	void update_color();

	const Glib::ustring m_active_icon_name;

	// Whether the document was changed since it has been active.
	gulong m_notify_status_handle;
	gulong m_notify_subscription_group_handle;
};

}

#endif // _GOBBY_TABLABEL_HPP_
