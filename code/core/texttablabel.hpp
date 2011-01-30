/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2011 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_TEXTTABLABEL_HPP_
#define _GOBBY_TEXTTABLABEL_HPP_

#include "core/textsessionview.hpp"
#include "core/tablabel.hpp"
#include "util/gtk-compat.hpp"

namespace Gobby
{

class TextTabLabel: public TabLabel
{
public:
	TextTabLabel(Folder& folder, TextSessionView& view);
	~TextTabLabel();

protected:
	static void on_modified_changed_static(GtkTextBuffer* buffer,
	                                       gpointer user_data)
	{
		static_cast<TextTabLabel*>(user_data)->on_modified_changed();
	}

	static void on_text_erased_static(InfTextBuffer* buffer,
	                                  guint position,
	                                  InfTextChunk* chunk,
	                                  InfTextUser* author,
	                                  gpointer user_data)
	{
		static_cast<TextTabLabel*>(user_data)->on_changed(author);
	}

	static void on_text_inserted_static(InfTextBuffer* buffer,
	                                    guint position,
	                                    InfTextChunk* text,
	                                    InfTextUser* author,
	                                    gpointer user_data)
	{
		static_cast<TextTabLabel*>(user_data)->on_changed(author);
	}

#ifdef USE_GTKMM3
	virtual void on_style_updated();
#else
	virtual void on_style_changed(const Glib::RefPtr<Gtk::Style>& prev);
#endif

	virtual void on_notify_status(); // override
	virtual void on_activate();

	void on_modified_changed();
	void on_changed(InfTextUser* author);

	Gtk::Label m_dots;

private:
	void update_modified();
	void update_dots();

	gunichar m_dot_char;

	gulong m_modified_changed_handle;
	gulong m_erase_text_handle;
	gulong m_insert_text_handle;

	class UserWatcher
	{
	public:
		UserWatcher(TextTabLabel* label, InfTextUser* user);
		UserWatcher(const UserWatcher& other);
		~UserWatcher();

		InfTextUser* get_user() const;
		bool operator==(InfTextUser* other_user) const;

	private:
		void connect();

		void disconnect();

		static void on_notify_hue(GObject* user_object,
		                          GParamSpec* spec,
		                          gpointer user_data);

		TabLabel* m_label;
		InfTextUser* m_user;
		gulong m_handle;
	};

	typedef std::list<UserWatcher> UserWatcherList;
	UserWatcherList m_changed_by;
};

}

#endif // _GOBBY_TEXTTABLABEL_HPP_
