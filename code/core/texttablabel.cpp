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

#include "core/texttablabel.hpp"
#include "core/folder.hpp"

Gobby::TextTabLabel::UserWatcher::UserWatcher(TextTabLabel* label,
                                              InfTextUser* user):
	m_label(label), m_user(user)
{
	connect();
}

Gobby::TextTabLabel::UserWatcher::UserWatcher(const UserWatcher& other):
	m_label(other.m_label), m_user(other.m_user)
{
	connect();
}

Gobby::TextTabLabel::UserWatcher::~UserWatcher()
{
	g_signal_handler_disconnect(m_user, m_handle);
}

InfTextUser* Gobby::TextTabLabel::UserWatcher::get_user() const
{
	return m_user;
}

bool
Gobby::TextTabLabel::UserWatcher::operator==(InfTextUser* other_user) const
{
	return m_user == other_user;
}

void Gobby::TextTabLabel::UserWatcher::connect()
{
	m_handle = g_signal_connect(
		G_OBJECT(m_user), "notify::hue",
		G_CALLBACK(&UserWatcher::on_notify_hue), m_label);
}

void Gobby::TextTabLabel::UserWatcher::on_notify_hue(GObject* user_object,
                                                     GParamSpec* spec,
                                                     gpointer user_data)
{
	static_cast<TextTabLabel*>(user_data)->update_dots();
}


Gobby::TextTabLabel::TextTabLabel(Folder& folder, TextSessionView& view):
	TabLabel(folder, view, "text-x-generic"), m_dot_char(0)
{
	m_modified_changed_handle = g_signal_connect_after(
		G_OBJECT(view.get_text_buffer()), "modified-changed",
		G_CALLBACK(on_modified_changed_static), this);

	InfTextBuffer* buffer = 
		INF_TEXT_BUFFER(
			inf_session_get_buffer(
				INF_SESSION(view.get_session())));
	m_insert_text_handle = g_signal_connect_after(
		G_OBJECT(buffer), "text-inserted",
		G_CALLBACK(on_text_inserted_static), this);
	m_erase_text_handle = g_signal_connect_after(
		G_OBJECT(buffer), "text-erased",
		G_CALLBACK(on_text_erased_static), this);

	insert_next_to(m_title, Gtk::POS_RIGHT);
	attach_next_to(m_dots, m_title, Gtk::POS_RIGHT, 1, 1);

	update_modified();
	update_dot_char();
}

Gobby::TextTabLabel::~TextTabLabel()
{
	TextSessionView& text_view = dynamic_cast<TextSessionView&>(m_view);

	g_signal_handler_disconnect(text_view.get_text_buffer(),
	                            m_modified_changed_handle);
	InfTextBuffer* buffer = 
		INF_TEXT_BUFFER(
			inf_session_get_buffer(
				INF_SESSION(m_view.get_session())));

	g_signal_handler_disconnect(buffer, m_erase_text_handle);
	g_signal_handler_disconnect(buffer, m_insert_text_handle);
}

void Gobby::TextTabLabel::on_style_updated()
{
	TabLabel::on_style_updated();

	update_dot_char();
}

void Gobby::TextTabLabel::on_notify_status()
{
	TabLabel::on_notify_status();
	update_modified();
}

void Gobby::TextTabLabel::on_activate()
{
	TabLabel::on_activate();
	m_changed_by.clear();
	update_dots();
}

void Gobby::TextTabLabel::on_modified_changed()
{
	update_modified();
}

void Gobby::TextTabLabel::on_changed(InfTextUser* author)
{
	if(!m_changed)
	{
		InfSession* session = INF_SESSION(m_view.get_session());
		if(inf_session_get_status(session) == INF_SESSION_RUNNING)
			set_changed();
	}

	if(m_folder.get_current_document() != &m_view)
	{
		// TODO: remove dot if all the user's
		// new contributions where undone
		if(std::find(m_changed_by.begin(), m_changed_by.end(), author)
		   == m_changed_by.end())
		{
			m_changed_by.push_back(UserWatcher(this, author));
			update_dots();
		}
	}
}

void Gobby::TextTabLabel::update_modified()
{
	InfSession* session = INF_SESSION(m_view.get_session());
	bool modified =
		inf_buffer_get_modified(inf_session_get_buffer(session));

	InfSessionStatus status = inf_session_get_status(session);
	if(status == INF_SESSION_SYNCHRONIZING ||
	   status == INF_SESSION_PRESYNC)
	{
		modified = false;
	}

	if(modified)
		m_title.set_text("*" + m_view.get_title());
	else
		m_title.set_text(m_view.get_title());
}

void Gobby::TextTabLabel::update_dot_char()
{
	static const gunichar dot_chars[] = {
		0x270E, /* pencil */
		0x26AB, /* medium black circle */
		0x25CF, /* black circle */
		0x002A, /* asterisk */
		0x0000
	};

	// Find a glyph for the user dots
	const gunichar* c;
	for(c = dot_chars; *c; ++c)
	{
		m_dots.set_text(Glib::ustring(1, *c));
		if(m_dots.get_layout()->get_unknown_glyphs_count() == 0)
			break;
	}

	m_dot_char = *c;

	// Update dots using this char
	update_dots();
}

void Gobby::TextTabLabel::update_dots()
{
	g_assert(m_dot_char != 0);
	if (m_changed_by.empty())
	{
		m_dots.hide();
	}
	else
	{
		Glib::ustring markup;
		for(UserWatcherList::iterator iter = m_changed_by.begin();
		    iter != m_changed_by.end(); ++iter)
		{
			double hue = inf_text_user_get_hue(iter->get_user());

			Gdk::Color color;
			color.set_hsv(hue * 360., 0.6, 0.6);

			markup += "<span color=\"" + color.to_string() +
			          "\">" + m_dot_char + "</span>";
		}
		m_dots.set_markup(markup);
		m_dots.show();
	}
}
