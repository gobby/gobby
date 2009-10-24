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

#ifndef _GOBBY_STATUSBAR_HPP_
#define _GOBBY_STATUSBAR_HPP_

#include "folder.hpp"

#include <gtkmm/box.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/statusbar.h>

#include <glibmm/ustring.h>

/* ARGH!!! */
#ifdef G_OS_WIN32
# ifdef ERROR
#  undef ERROR
# endif
#endif

namespace Gobby
{

class StatusBar: public Gtk::HBox
{
protected:
	class Message;
	typedef std::list<Message*> MessageList;

public:
	enum MessageType {
		INFO,
		ERROR
	};

	typedef MessageList::iterator MessageHandle;

	StatusBar(Folder& folder, const Preferences& preferences);
	~StatusBar();

	MessageHandle add_info_message(const Glib::ustring& message);

	MessageHandle add_error_message(const Glib::ustring& brief_desc,
	                                const Glib::ustring& detailed_desc);

	void remove_message(const MessageHandle& handle);

	MessageHandle invalid_handle();

protected:
	MessageHandle add_message(MessageType type,
	                          const Glib::ustring& message,
	                          const Glib::ustring& dialog_message);

	static void on_mark_set_static(GtkTextBuffer* buffer,
	                               GtkTextIter* location,
	                               GtkTextMark* mark,
	                               gpointer user_data)
	{
		static_cast<StatusBar*>(user_data)->on_mark_set(mark);
	}

	static void on_changed_static(GtkTextBuffer* buffer,
	                              gpointer user_data)
	{
		static_cast<StatusBar*>(user_data)->on_changed();
	}

	void on_message_clicked(GdkEventButton* button,
	                        const MessageHandle& message);

	void on_document_removed(SessionView& view);
	void on_document_changed(SessionView* view);
	void on_view_changed();
	
	void on_mark_set(GtkTextMark* mark);
	void on_changed();

	void update_pos_display();

	Folder& m_folder;
	const Preferences& m_preferences;
	MessageList m_list;

	Gtk::Statusbar m_bar_position;
	TextSessionView* m_current_view;
	gulong m_mark_set_handler;
	gulong m_changed_handler;

	guint m_position_context_id;
};

}

#endif // _GOBBY_STATUSBAR_HPP_
