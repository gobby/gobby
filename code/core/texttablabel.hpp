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
	void update_dot_char();
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
