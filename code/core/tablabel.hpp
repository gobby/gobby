/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008, 2009 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_TABLABEL_HPP_
#define _GOBBY_TABLABEL_HPP_

#include "core/sessionview.hpp"

#include <gtkmm/box.h>
#include <gtkmm/image.h>

namespace Gobby
{

class Folder;

class TabLabel: public Gtk::HBox
{
public:
	typedef Glib::SignalProxy0<void> SignalCloseRequest;

	TabLabel(Folder& folder, SessionView& view, Gtk::StockID active_icon);
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
	Gtk::HBox m_extra;
	CloseButton m_button;

	bool m_changed;

private:
	void update_icon();
	void update_color();

	Gtk::StockID m_active_icon;

	// Whether the document was changed since it has been active.
	gulong m_notify_status_handle;
	gulong m_notify_subscription_group_handle;
};

}

#endif // _GOBBY_TABLABEL_HPP_
