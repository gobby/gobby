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

#include "core/textsessionview.hpp"
#include "util/closebutton.hpp"

#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>

#include <list>

namespace Gobby
{

class Folder;

class TabLabel: public Gtk::HBox
{
public:
	typedef Glib::SignalProxy0<void> SignalCloseRequest;

	TabLabel(Folder& folder, TextSessionView& view);
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

	static void on_erase_text_static(InfTextBuffer* buffer,
	                                 guint position,
	                                 guint length,
	                                 InfTextUser* author,
	                                 gpointer user_data)
	{
		static_cast<TabLabel*>(user_data)->on_changed(author);
	}

	static void on_insert_text_static(InfTextBuffer* buffer,
	                                  guint position,
	                                  InfTextChunk* text,
	                                  InfTextUser* author,
	                                  gpointer user_data)
	{
		static_cast<TabLabel*>(user_data)->on_changed(author);
	}

	virtual void on_style_changed(const Glib::RefPtr<Gtk::Style>& prev);

	void on_notify_editable();
	void on_notify_status();
	void on_notify_subscription_group();

	void on_modified_changed();
	void on_changed(InfTextUser* author);

	void on_folder_document_changed(SessionView* view);

	void update_icon();
	void update_color();
	void update_modified();
	void update_dots();

	Folder& m_folder;
	TextSessionView& m_view;

	Gtk::Image m_icon;
	Gtk::Label m_title;
	Gtk::Label m_dots;
	CloseButton m_button;

	gunichar m_dot_char;

	// Whether the document was changed since it has been active.
	bool m_changed;

	gulong m_notify_editable_handle;
	gulong m_notify_status_handle;
	gulong m_notify_subscription_group_handle;
	gulong m_modified_changed_handle;
	gulong m_erase_text_handle;
	gulong m_insert_text_handle;

	class UserWatcher
	{
	public:
		UserWatcher(TabLabel* l, InfTextUser* u);

		UserWatcher(const UserWatcher& other);

		~UserWatcher();

		InfTextUser* get_user() const;

		bool operator==(InfTextUser* other_user) const;

		// UserWatcher& operator=(const UserWatcher& other);

	private:
		void connect();

		void disconnect();

		static void on_notify_hue(GObject* user_object,
		                          GParamSpec* spec,
		                          gpointer user_data);

		TabLabel* label;
		InfTextUser* user;
		gulong handle;
	};

	std::list<UserWatcher> m_changed_by;
};

}

#endif // _GOBBY_TABLABEL_HPP_
