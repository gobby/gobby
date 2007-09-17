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

#include "features.hpp"

#include <glibmm/pattern.h>
#include <gtkmm/textview.h>

#ifdef WITH_GTKSOURCEVIEW2
# include <gtksourceview/gtksourcebuffer.h>
#endif

#include "preferences.hpp"
#include "docwindow.hpp"

namespace
{
	GtkWrapMode wrap_mode_from_preferences(const Gobby::Preferences& pref)
	{
		if(pref.view.wrap_text)
		{
			if(pref.view.wrap_words)
				return GTK_WRAP_CHAR;
			else
				return GTK_WRAP_WORD;
		}
		else
		{
			return GTK_WRAP_NONE;
		}
	}
}

Gobby::DocWindow::DocWindow(LocalDocumentInfo& info,
                            const Preferences& preferences):
	m_view(GTK_SOURCE_VIEW(gtk_source_view_new())),
	m_info(info), m_doc(info.get_content() ),
	m_preferences(preferences), /*m_editing(false),*/
	m_title(info.get_title() ),
	m_scrolly(0.0),
	m_scroll_restore(false)
{
	if(!info.is_subscribed() )
	{
		throw std::logic_error(
			"Gobby::DocWindow::DocWindow:\n"
			"Local user is not subscribed"
		);
	}

	GtkSourceBuffer* buffer = m_doc.get_buffer();
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(m_view), GTK_TEXT_BUFFER(buffer));

	Glib::RefPtr<Gtk::TextBuffer> cpp_buffer =
		Glib::wrap(GTK_TEXT_BUFFER(buffer), true);

	// Set source language by filename
	gtk_source_buffer_set_highlight_syntax(buffer, FALSE);

	for(Preferences::FileList::iterator iter = preferences.files.begin();
	    iter != preferences.files.end();
	    ++ iter)
	{
		Glib::PatternSpec spec(iter.pattern());
		if(spec.match(info.get_title()) )
		{
			gtk_source_buffer_set_language(buffer, iter.language());
			gtk_source_buffer_set_highlight_syntax(buffer, TRUE);
		}
	}

#ifdef WITH_GTKSOURCEVIEW2
	// Set a theme so we see anything.
	// TODO: This should be temporary code until gtksourceview2 sets a default
	// theme.
/*	GtkSourceStyleManager* sm = gtk_source_style_manager_new();
	GtkSourceStyleScheme* scheme = gtk_source_style_manager_get_scheme(sm, "gvim");
	gtk_source_buffer_set_style_scheme(buffer, scheme);
	g_object_unref(G_OBJECT(sm));*/
#endif

	cpp_buffer->signal_mark_set().connect(
		sigc::mem_fun(*this, &DocWindow::on_mark_set)
	);

	cpp_buffer->signal_changed().connect(
		sigc::mem_fun(*this, &DocWindow::on_changed)
	);

	m_doc.local_insert_event().connect(
		sigc::mem_fun(*this, &DocWindow::on_local_insert)
	);

	m_doc.local_erase_event().connect(
		sigc::mem_fun(*this, &DocWindow::on_local_erase)
	);

	m_doc.remote_insert_before_event().connect(
		sigc::mem_fun(*this, &DocWindow::on_remote_insert_before)
	);

	m_doc.remote_erase_before_event().connect(
		sigc::mem_fun(*this, &DocWindow::on_remote_erase_before)
	);

	m_doc.remote_insert_after_event().connect(
		sigc::mem_fun(*this, &DocWindow::on_remote_insert_after)
	);

	m_doc.remote_erase_after_event().connect(
		sigc::mem_fun(*this, &DocWindow::on_remote_erase_after)
	);

	apply_preferences();
	gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(buffer), !m_doc.empty());

	set_shadow_type(Gtk::SHADOW_IN);
	set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(gobj()), GTK_WIDGET(m_view));
}

void Gobby::DocWindow::get_cursor_position(unsigned int& row,
                                           unsigned int& col)
{
	Glib::RefPtr<Gtk::TextBuffer> cpp_buffer = Glib::wrap(
		gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view)), true);

	Glib::RefPtr<Gtk::TextMark> mark = cpp_buffer->get_insert();

	// Gtk::TextBuffer::Mark::get_iter is not const. Why not? It prevents
	// this function from being const.
	Gtk::TextBuffer::iterator iter = mark->get_iter();

	// Row is trivial
	row = iter.get_line(); col = 0;
	int chars = iter.get_line_offset();

	// Tab characters expand to more than one column
	unsigned int tabs = m_preferences.editor.tab_width;
	for(iter.set_line_offset(0); iter.get_line_offset() < chars; ++ iter)
	{
		unsigned int width = 1;
		if(*iter == '\t')
		{
			width = (tabs - iter.get_line_offset() % tabs) % tabs;
			if(width == 0) width = tabs;
		}

		col += width;
	}
}

void Gobby::DocWindow::set_selection(const Gtk::TextIter& begin,
                                     const Gtk::TextIter& end)
{
	gtk_text_buffer_select_range(
		gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view)),
		begin.gobj(),
		end.gobj()
		);

	gtk_text_view_scroll_to_mark(
		GTK_TEXT_VIEW(m_view),
		gtk_text_buffer_get_insert(gtk_text_view_get_buffer(
			GTK_TEXT_VIEW(m_view))),
		0.1, FALSE, 0.0, 0.0);
}

void Gobby::DocWindow::disable()
{
	gtk_widget_set_sensitive(GTK_WIDGET(m_view), FALSE);
}

Glib::ustring Gobby::DocWindow::get_selected_text() const
{
	GtkTextIter start, end;
	gtk_text_buffer_get_selection_bounds(
		gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view)), &start, &end);

	Gtk::TextIter start_cpp(&start), end_cpp(&end);
	return start_cpp.get_slice(end_cpp);
}

const Glib::ustring& Gobby::DocWindow::get_title() const
{
	return m_title;
}

bool Gobby::DocWindow::get_modified() const
{
	return gtk_text_buffer_get_modified(
		gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view))) == TRUE;
}

void Gobby::DocWindow::grab_focus()
{
	Gtk::ScrolledWindow::grab_focus();
	gtk_widget_grab_focus(GTK_WIDGET(m_view));
}

GtkSourceLanguage* Gobby::DocWindow::get_language() const
{
	return gtk_source_buffer_get_language(GTK_SOURCE_BUFFER(
		gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view))));
}

void Gobby::DocWindow::set_language(GtkSourceLanguage* language)
{
	GtkSourceBuffer* buffer = GTK_SOURCE_BUFFER(
		gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view)));

	gtk_source_buffer_set_language(buffer, language);
	gtk_source_buffer_set_highlight_syntax(buffer, language != NULL);

	m_signal_language_changed.emit();
}

const Gobby::Preferences& Gobby::DocWindow::get_preferences() const
{
	return m_preferences;
}

void Gobby::DocWindow::set_preferences(const Preferences& preferences)
{
	m_preferences = preferences;
	apply_preferences();
}

Glib::ustring Gobby::DocWindow::get_content() const
{
	Glib::RefPtr<Gtk::TextBuffer> buffer = Glib::wrap(
		gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view)), true);

	return buffer->get_text();
}

Gobby::DocWindow::signal_cursor_moved_type
Gobby::DocWindow::cursor_moved_event() const
{
	return m_signal_cursor_moved;
}

Gobby::DocWindow::signal_content_changed_type
Gobby::DocWindow::content_changed_event() const
{
	return m_signal_content_changed;
}

Gobby::DocWindow::signal_language_changed_type
Gobby::DocWindow::language_changed_event() const
{
	return m_signal_language_changed;
}

const Gobby::LocalDocumentInfo& Gobby::DocWindow::get_info() const
{
	return m_info;
}

Gobby::LocalDocumentInfo& Gobby::DocWindow::get_info()
{
	return m_info;
}

const Gobby::Document& Gobby::DocWindow::get_document() const
{
	return m_doc;
}

void Gobby::DocWindow::on_mark_set(const Gtk::TextIter& location,
                                   const Glib::RefPtr<Gtk::TextMark>& mark)
{
	// Mark was deleted
	if(!mark) return;

	if(mark->gobj() == gtk_text_buffer_get_insert(
	   gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view))))
	{
		m_signal_cursor_moved.emit();
	}
}

void Gobby::DocWindow::on_changed()
{
	// Cursor may have moved.
	// TODO: Check if the cursor really moved
	m_signal_cursor_moved.emit();
	m_signal_content_changed.emit();
}

void Gobby::DocWindow::on_local_insert(obby::position pos,
                                       const std::string& error)
{
	m_info.insert(pos, error);
}

void Gobby::DocWindow::on_local_erase(obby::position pos,
                                      obby::position len)
{
	m_info.erase(pos, len);
}

void Gobby::DocWindow::on_remote_insert_before(obby::position,
                                               const std::string& text)
{
	store_scroll();
}

void Gobby::DocWindow::on_remote_erase_before(obby::position pos,
                                              obby::position len)
{
	store_scroll();
}

void Gobby::DocWindow::on_remote_insert_after(obby::position,
                                              const std::string& text)
{
	restore_scroll();
}

void Gobby::DocWindow::on_remote_erase_after(obby::position pos,
                                             obby::position len)
{
	restore_scroll();
}

void Gobby::DocWindow::apply_preferences()
{
	GtkTextBuffer* buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view));

	gtk_source_view_set_tab_width(GTK_SOURCE_VIEW(m_view),
		m_preferences.editor.tab_width);
	gtk_source_view_set_insert_spaces_instead_of_tabs(GTK_SOURCE_VIEW(m_view),
		m_preferences.editor.tab_spaces);
	gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(m_view),
		m_preferences.editor.indentation_auto);
#ifdef WITH_GTKSOURCEVIEW2
	gtk_source_view_set_smart_home_end(GTK_SOURCE_VIEW(m_view),
		m_preferences.editor.homeend_smart ?
		GTK_SOURCE_SMART_HOME_END_ALWAYS :
		GTK_SOURCE_SMART_HOME_END_DISABLED);
#else
	gtk_source_view_set_smart_home_end(GTK_SOURCE_VIEW(m_view),
		m_preferences.editor.homeend_smart);
#endif

	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m_view),
		wrap_mode_from_preferences(m_preferences));
	gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(m_view),
		m_preferences.view.linenum_display);
	gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(m_view),
		m_preferences.view.curline_highlight);
#ifdef WITH_GTKSOURCEVIEW2
	gtk_source_view_set_show_right_margin(GTK_SOURCE_VIEW(m_view),
		m_preferences.view.margin_display);
	gtk_source_view_set_right_margin_position(GTK_SOURCE_VIEW(m_view),
		m_preferences.view.margin_pos);
	gtk_source_buffer_set_highlight_matching_brackets(GTK_SOURCE_BUFFER(buffer),
		m_preferences.view.bracket_highlight);
#else
	gtk_source_view_set_show_margin(GTK_SOURCE_VIEW(m_view),
		m_preferences.view.margin_display);
	gtk_source_view_set_margin(GTK_SOURCE_VIEW(m_view),
		m_preferences.view.margin_pos);
	gtk_source_buffer_set_check_brackets(GTK_SOURCE_BUFFER(buffer),
		m_preferences.view.bracket_highlight);
#endif

	gtk_widget_modify_font(GTK_WIDGET(m_view), m_preferences.font.desc.gobj());

	// Cursor position may have changed because of new tab width
	// TODO: Only emit if the position really changed
	m_signal_cursor_moved.emit();
}

void Gobby::DocWindow::store_scroll()
{
	Gdk::Rectangle curs_rect;
	int x, y;

	Gtk::TextView* cpp_view = Glib::wrap(GTK_TEXT_VIEW(m_view));
	cpp_view->get_iter_location(cpp_view->get_buffer()->get_insert()->get_iter(), curs_rect);
	cpp_view->buffer_to_window_coords(
		Gtk::TEXT_WINDOW_WIDGET,
		curs_rect.get_x(), curs_rect.get_y(),
		x, y);

	m_scrolly = y / static_cast<double>((cpp_view->get_height() - curs_rect.get_height()));
	m_scroll_restore = true;
}

void Gobby::DocWindow::restore_scroll()
{
	if(m_scroll_restore)
	{
		Gtk::TextView* cpp_view = Glib::wrap(GTK_TEXT_VIEW(m_view));
		if(m_scrolly >= 0.0 && m_scrolly <= 1.0)
			cpp_view->scroll_to(
				cpp_view->get_buffer()->get_insert(),
				0, 0,
				m_scrolly);

		m_scroll_restore = false;
	}
}

