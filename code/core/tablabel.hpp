/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005 0x539 dev group
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

#include "core/docwindow.hpp"
#include "util/closebutton.hpp"

#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>

namespace Gobby
{

class Folder;

class TabLabel: public Gtk::HBox
{
public:
	typedef Glib::SignalProxy0<void> SignalCloseRequest;

	TabLabel(Folder& folder, DocWindow& document);
	~TabLabel();

	SignalCloseRequest signal_close_request()
	{
		return m_button.signal_clicked();
	}

protected:
	static void on_notify_editable_static(GObject* object,
	                                      GParamSpec* pspec,
	                                      gpointer user_data)
	{
		static_cast<TabLabel*>(user_data)->on_notify_editable();
	}

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

	static void on_modified_changed_static(GtkTextBuffer* buffer,
	                                       gpointer user_data)
	{
		static_cast<TabLabel*>(user_data)->on_modified_changed();
	}

	static void on_changed_static(GtkTextBuffer* buffer,
	                              gpointer user_data)
	{
		static_cast<TabLabel*>(user_data)->on_changed();
	}

	void on_notify_editable();
	void on_notify_status();
	void on_notify_subscription_group();

	void on_modified_changed();
	void on_changed();

	void on_folder_document_changed(DocWindow* document);

	void update_icon();
	void update_color();
	void update_modified();

	Folder& m_folder;
	DocWindow& m_document;

	Gtk::Image m_icon;
	Gtk::Label m_title;
	CloseButton m_button;

	// Whether the document was changed since it has been active.
	bool m_changed;

	gulong m_notify_editable_handle;
	gulong m_notify_status_handle;
	gulong m_notify_subscription_group_handle;
	gulong m_modified_changed_handle;
	gulong m_changed_handle;
};

}

#endif // _GOBBY_TABLABEL_HPP_
