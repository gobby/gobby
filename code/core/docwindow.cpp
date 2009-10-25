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

#include "features.hpp"

#include "core/docwindow.hpp"
#include "core/preferences.hpp"
#include "core/closableframe.hpp"
#include "core/iconmanager.hpp"

#include "util/i18n.hpp"

#include <gtkmm/scrolledwindow.h>
#include <glibmm/pattern.h>

#include <libinftextgtk/inf-text-gtk-buffer.h>

#include <gtksourceview/gtksourcebuffer.h>

// TODO: Consider using a single user list for all DocWindows, reparenting
// into the current DocWindow's frame. Keep dummy widgets in other docwindows,
// so text does not resize.

namespace
{
	GtkWrapMode wrap_mode_from_preferences(const Gobby::Preferences& pref)
	{
		return static_cast<GtkWrapMode>(
			static_cast<Gtk::WrapMode>(pref.view.wrap_mode));
	}

	bool glob_matches(const gchar* const* globs, const std::string& str)
	{
		if(globs)
		{
			for(const gchar* const* glob = globs;
			    *glob != NULL;
			    ++ glob)
			{
				Glib::PatternSpec spec(*glob);
				if(spec.match(str))
					return true;
			}
		}

		return false;
	}

	bool language_matches_title(GtkSourceLanguage* language,
	                            const gchar* title)
	{
		bool result = false;
		gchar** globs = gtk_source_language_get_globs(language);
		if(glob_matches(globs, title))
			result = true;
		g_strfreev(globs);
		return result;
	}

	GtkSourceLanguage*
	get_language_for_title(GtkSourceLanguageManager* manager,
	                       const gchar* title)
	{
		const gchar* const* ids =
			gtk_source_language_manager_get_language_ids(manager);

		if(ids)
		{
			for(const gchar* const* id = ids; *id != NULL; ++ id)
			{
				GtkSourceLanguage* l;
				l = gtk_source_language_manager_get_language(
					manager, *id);

				if(l)
					if(language_matches_title(l, title))
						return l;
			}
		}

		return NULL;
	}

	bool tags_priority_idle_func(Gobby::DocWindow& window)
	{
		InfTextGtkBuffer* buffer = INF_TEXT_GTK_BUFFER(
			inf_session_get_buffer(
				INF_SESSION(window.get_session())));

		inf_text_gtk_buffer_ensure_author_tags_priority(buffer);

		// I don't know why it does not redraw automatically, perhaps
		// this is a bug.
		gtk_widget_queue_draw(GTK_WIDGET(window.get_text_view()));
		return false;
	}

	void on_tag_added(GtkTextTagTable* table, GtkTextTag* tag,
	                  gpointer user_data)
	{
		// We do the actual reordering in an idle handler because
		// the priority of the tag might not yet be set to its final
		// value.
		Glib::signal_idle().connect(
			sigc::bind(
				sigc::ptr_fun(tags_priority_idle_func),
				sigc::ref(*static_cast<Gobby::DocWindow*>(
					user_data))));
	}
}

Gobby::DocWindow::DocWindow(InfTextSession* session,
                            const Glib::ustring& title,
                            const Glib::ustring& path,
                            const Glib::ustring& hostname,
                            const std::string& info_storage_key,
                            Preferences& preferences,
			    GtkSourceLanguageManager* manager):
	m_session(session), m_title(title), m_path(path),
	m_hostname(hostname), m_info_storage_key(info_storage_key),
	m_preferences(preferences),
	m_view(GTK_SOURCE_VIEW(gtk_source_view_new())),
	m_userlist(session), m_info_box(false, 0),
	m_info_close_button_box(false, 6)
{
	g_object_ref(m_session);

	InfBuffer* buffer = inf_session_get_buffer(INF_SESSION(session));
	m_buffer = GTK_SOURCE_BUFFER(inf_text_gtk_buffer_get_text_buffer(
		INF_TEXT_GTK_BUFFER(buffer)));

	// This is a hack to make sure that the author tags in the textview
	// have lowest priority of all tags, especially lower than
	// GtkSourceView's FIXME tags. We do this every time a new tag is
	// added to the tag table since GtkSourceView seems to create tags
	// that it needs on the fly.
	GtkTextTagTable* table = gtk_text_buffer_get_tag_table(
		GTK_TEXT_BUFFER(m_buffer));
	g_signal_connect(G_OBJECT(table), "tag-added",
	                 G_CALLBACK(on_tag_added), this);

	gtk_widget_set_has_tooltip(GTK_WIDGET(m_view), TRUE);
	g_signal_connect(m_view, "query-tooltip",
	                 G_CALLBACK(on_query_tooltip_static), this);

	gtk_text_view_set_buffer(GTK_TEXT_VIEW(m_view),
	                         GTK_TEXT_BUFFER(m_buffer));
	gtk_text_view_set_editable(GTK_TEXT_VIEW(m_view), FALSE);
	set_language(get_language_for_title(manager, m_title.c_str()));

	m_preferences.user.hue.signal_changed().connect(
		sigc::mem_fun(*this, &DocWindow::on_user_color_changed));
	m_preferences.editor.tab_width.signal_changed().connect(
		sigc::mem_fun(*this, &DocWindow::on_tab_width_changed));
	m_preferences.editor.tab_spaces.signal_changed().connect(
		sigc::mem_fun(*this, &DocWindow::on_tab_spaces_changed));
	m_preferences.editor.indentation_auto.signal_changed().connect(
		sigc::mem_fun(*this, &DocWindow::on_auto_indent_changed));
	m_preferences.editor.homeend_smart.signal_changed().connect(
		sigc::mem_fun(*this, &DocWindow::on_homeend_smart_changed));

	m_preferences.view.wrap_mode.signal_changed().connect(
		sigc::mem_fun(*this, &DocWindow::on_wrap_mode_changed));
	m_preferences.view.linenum_display.signal_changed().connect(
		sigc::mem_fun(*this, &DocWindow::on_linenum_display_changed));
	m_preferences.view.curline_highlight.signal_changed().connect(
		sigc::mem_fun(*this,
		              &DocWindow::on_curline_highlight_changed));
	m_preferences.view.margin_display.signal_changed().connect(
		sigc::mem_fun(*this, &DocWindow::on_margin_display_changed));
	m_preferences.view.margin_pos.signal_changed().connect(
		sigc::mem_fun(*this, &DocWindow::on_margin_pos_changed));
	m_preferences.view.bracket_highlight.signal_changed().connect(
		sigc::mem_fun(*this,
		              &DocWindow::on_bracket_highlight_changed));

	m_preferences.appearance.font.signal_changed().connect(
		sigc::mem_fun(*this, &DocWindow::on_font_changed));

	gtk_source_view_set_tab_width(m_view, m_preferences.editor.tab_width);
	gtk_source_view_set_insert_spaces_instead_of_tabs(
		m_view, m_preferences.editor.tab_spaces);
	gtk_source_view_set_auto_indent(
		m_view, m_preferences.editor.indentation_auto);
	gtk_source_view_set_smart_home_end(
		m_view, m_preferences.editor.homeend_smart ?
			GTK_SOURCE_SMART_HOME_END_ALWAYS :
			GTK_SOURCE_SMART_HOME_END_DISABLED);
	gtk_text_view_set_wrap_mode(
		GTK_TEXT_VIEW(m_view),
		wrap_mode_from_preferences(m_preferences));
	gtk_source_view_set_show_line_numbers(
		m_view, m_preferences.view.linenum_display);
	gtk_source_view_set_highlight_current_line(
		m_view, m_preferences.view.curline_highlight);
	gtk_source_view_set_show_right_margin(
		m_view, m_preferences.view.margin_display);
	gtk_source_view_set_right_margin_position(
		m_view, m_preferences.view.margin_pos);
	gtk_source_buffer_set_highlight_matching_brackets(
		m_buffer, m_preferences.view.bracket_highlight);
	const Pango::FontDescription& desc = m_preferences.appearance.font;
	gtk_widget_modify_font(
		GTK_WIDGET(m_view),
		const_cast<PangoFontDescription*>(desc.gobj()));

	m_info_label.set_selectable(true);
	m_info_label.set_line_wrap(true);
	m_info_label.show();

	m_info_close_button.signal_clicked().connect(
		sigc::mem_fun(m_info_frame, &Gtk::Frame::hide));
	m_info_close_button.show();

	m_info_close_button_box.pack_end(m_info_close_button, Gtk::PACK_SHRINK);
	// Don't show info close button box by default

	m_info_box.pack_start(m_info_close_button_box, Gtk::PACK_SHRINK);
	m_info_box.pack_start(m_info_label, Gtk::PACK_SHRINK);
	m_info_box.set_border_width(6);
	m_info_box.show();

	m_info_frame.set_shadow_type(Gtk::SHADOW_IN);
	m_info_frame.add(m_info_box);
	// Don't show infoframe by default

	gtk_widget_show(GTK_WIDGET(m_view));
	Gtk::ScrolledWindow* scroll = Gtk::manage(new Gtk::ScrolledWindow);
	scroll->set_shadow_type(Gtk::SHADOW_IN);
	scroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scroll->gobj()), GTK_WIDGET(m_view));
	scroll->show();

	Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox);
	vbox->pack_start(m_info_frame, Gtk::PACK_SHRINK);
	vbox->pack_start(*scroll, Gtk::PACK_EXPAND_WIDGET);
	vbox->show();

	m_userlist.show();
	Gtk::Frame* frame = Gtk::manage(new ClosableFrame(
		_("User List"), IconManager::STOCK_USERLIST,
		m_preferences.appearance.show_userlist));
	frame->set_shadow_type(Gtk::SHADOW_IN);
	frame->add(m_userlist);
	// frame manages visibility itself

	pack1(*vbox, true, false);
	pack2(*frame, false, false);
}

Gobby::DocWindow::~DocWindow()
{
	g_object_unref(m_session);
	m_session = NULL;
}

void Gobby::DocWindow::get_cursor_position(unsigned int& row,
                                           unsigned int& col) const
{
	GtkTextMark* insert_mark =
		gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(m_buffer));

	GtkTextIter iter;
	gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(m_buffer),
	                                 &iter, insert_mark);

	row = gtk_text_iter_get_line(&iter);
	col = 0;

	unsigned int chars = gtk_text_iter_get_line_offset(&iter);
	unsigned int tabs = m_preferences.editor.tab_width;

	// Tab characters expand to more than one column
	for(gtk_text_iter_set_line_offset(&iter, 0);
	    gtk_text_iter_get_line_offset(&iter) < chars;
	    gtk_text_iter_forward_char(&iter))
	{
		unsigned int width = 1;
		if(gtk_text_iter_get_char(&iter) == '\t')
		{
			unsigned int offset =
				gtk_text_iter_get_line_offset(&iter);
			width = (tabs - offset % tabs) % tabs;
			if(width == 0) width = tabs;
		}

		col += width;
	}
}

void Gobby::DocWindow::set_selection(const GtkTextIter* begin,
                                     const GtkTextIter* end)
{
	gtk_text_buffer_select_range(
		gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view)), begin, end);

	scroll_to_cursor_position(0.1);
}

Glib::ustring Gobby::DocWindow::get_selected_text() const
{
	GtkTextIter start, end;
	gtk_text_buffer_get_selection_bounds(
		gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view)),
		&start, &end);

	Gtk::TextIter start_cpp(&start), end_cpp(&end);
	return start_cpp.get_slice(end_cpp);
}

void Gobby::DocWindow::scroll_to_cursor_position(double within_margin)
{
	gtk_text_view_scroll_to_mark(
		GTK_TEXT_VIEW(m_view),
		gtk_text_buffer_get_insert(gtk_text_view_get_buffer(
			GTK_TEXT_VIEW(m_view))),
		within_margin, FALSE, 0.0, 0.0);
}

void Gobby::DocWindow::set_info(const Glib::ustring& info, bool closable)
{
	m_info_label.set_text(info);

	if(closable) m_info_close_button_box.show();
	else m_info_close_button_box.hide();

	m_info_frame.show();
}

void Gobby::DocWindow::unset_info()
{
	m_info_frame.hide();
}

InfTextUser* Gobby::DocWindow::get_active_user() const
{
	InfTextGtkBuffer* buffer = INF_TEXT_GTK_BUFFER(
		inf_session_get_buffer(INF_SESSION(m_session)));
	return inf_text_gtk_buffer_get_active_user(buffer);
}

void Gobby::DocWindow::set_active_user(InfTextUser* user)
{
	g_assert(
		user == NULL ||
		inf_user_table_lookup_user_by_id(
			inf_session_get_user_table(INF_SESSION(m_session)),
			inf_user_get_id(INF_USER(user)))
		== INF_USER(user));

	inf_text_gtk_buffer_set_active_user(
		INF_TEXT_GTK_BUFFER(
			inf_session_get_buffer(INF_SESSION(m_session))),
		user);

	// TODO: Make sure the active user has the color specified in the
	// preferences, and set color if not.

	if(user != NULL)
		gtk_text_view_set_editable(GTK_TEXT_VIEW(m_view), TRUE);
	else
		gtk_text_view_set_editable(GTK_TEXT_VIEW(m_view), FALSE);

	m_signal_active_user_changed.emit(user);
}

GtkSourceLanguage* Gobby::DocWindow::get_language() const
{
	return gtk_source_buffer_get_language(m_buffer);
}

void Gobby::DocWindow::set_language(GtkSourceLanguage* language)
{
	gtk_source_buffer_set_language(m_buffer, language);
	m_signal_language_changed.emit(language);
}

void Gobby::DocWindow::on_size_allocate(Gtk::Allocation& allocation)
{
	Gtk::HPaned::on_size_allocate(allocation);

	// Setup initial paned position. We can't do this simply every time
	// on_size_allocate() is called since this would lead to an endless
	// loop somehow when the userlist width is changed forcefully 
	// (for example by a set_info() requiring much width).
	if(!m_doc_userlist_width_changed_connection.connected())
	{
		Glib::SignalProxyProperty proxy =
			property_position().signal_changed();

		m_doc_userlist_width_changed_connection =
			proxy.connect(sigc::mem_fun(
				*this,
				&DocWindow::on_doc_userlist_width_changed));

		Preferences::Option<unsigned int>& option = 
			m_preferences.appearance.userlist_width;

		m_pref_userlist_width_changed_connection =
			option.signal_changed().connect(sigc::mem_fun(
				*this,
				&DocWindow::on_pref_userlist_width_changed));

		unsigned int desired_position =
			get_width() - m_preferences.appearance.userlist_width;
		desired_position = std::min<unsigned int>(
			desired_position, property_max_position());

		if(get_position() != desired_position)
			set_position(desired_position);
	}
}

void Gobby::DocWindow::on_user_color_changed()
{
	InfTextUser* user = get_active_user();

	if(user)
	{
		inf_text_session_set_user_color(m_session, user,
		                                m_preferences.user.hue);
	}
}

void Gobby::DocWindow::on_tab_width_changed()
{
	gtk_source_view_set_tab_width(m_view, m_preferences.editor.tab_width);
}

void Gobby::DocWindow::on_tab_spaces_changed()
{
	gtk_source_view_set_insert_spaces_instead_of_tabs(
		m_view, m_preferences.editor.tab_spaces);
}

void Gobby::DocWindow::on_auto_indent_changed()
{
	gtk_source_view_set_auto_indent(
		m_view, m_preferences.editor.indentation_auto);
}

void Gobby::DocWindow::on_homeend_smart_changed()
{
	gtk_source_view_set_smart_home_end(
		m_view, m_preferences.editor.homeend_smart ?
			GTK_SOURCE_SMART_HOME_END_ALWAYS :
			GTK_SOURCE_SMART_HOME_END_DISABLED);
}

void Gobby::DocWindow::on_wrap_mode_changed()
{
	gtk_text_view_set_wrap_mode(
		GTK_TEXT_VIEW(m_view),
		wrap_mode_from_preferences(m_preferences));
}

void Gobby::DocWindow::on_linenum_display_changed()
{
	gtk_source_view_set_show_line_numbers(
		m_view, m_preferences.view.linenum_display);
}

void Gobby::DocWindow::on_curline_highlight_changed()
{
	gtk_source_view_set_highlight_current_line(
		m_view, m_preferences.view.curline_highlight);
}

void Gobby::DocWindow::on_margin_display_changed()
{
	gtk_source_view_set_show_right_margin(
		m_view, m_preferences.view.margin_display);
}

void Gobby::DocWindow::on_margin_pos_changed()
{
	gtk_source_view_set_right_margin_position(
		m_view, m_preferences.view.margin_pos);
}

void Gobby::DocWindow::on_bracket_highlight_changed()
{
	gtk_source_buffer_set_highlight_matching_brackets(
		m_buffer, m_preferences.view.bracket_highlight);
}

void Gobby::DocWindow::on_font_changed()
{
	const Pango::FontDescription& desc = m_preferences.appearance.font;
	gtk_widget_modify_font(
		GTK_WIDGET(m_view),
		const_cast<PangoFontDescription*>(desc.gobj()));
}

void Gobby::DocWindow::on_doc_userlist_width_changed()
{
	unsigned int userlist_width = get_width() - get_position();

	if(m_preferences.appearance.userlist_width != userlist_width)
	{
		m_pref_userlist_width_changed_connection.block();
		m_preferences.appearance.userlist_width = userlist_width;
		m_pref_userlist_width_changed_connection.unblock();
	}
}

void Gobby::DocWindow::on_pref_userlist_width_changed()
{
	unsigned int position =
		get_width() - m_preferences.appearance.userlist_width;

	if(get_position() != position)
	{
		m_doc_userlist_width_changed_connection.block();
		set_position(position);
		m_doc_userlist_width_changed_connection.unblock();
	}
}

bool
Gobby::DocWindow::on_query_tooltip(int x, int y, bool keyboard_mode,
                                   const Glib::RefPtr<Gtk::Tooltip>& tooltip)
{
	if(keyboard_mode) return false;

	int buffer_x, buffer_y;
	gtk_text_view_window_to_buffer_coords(
		GTK_TEXT_VIEW(m_view),
		GTK_TEXT_WINDOW_WIDGET, x, y, &buffer_x, &buffer_y);

	// Pointer is in line number display
	if(buffer_x < 0) return false;

	// Get the character at the mouse position
	GtkTextIter iter;
	int trailing;
	gtk_text_view_get_iter_at_position(
		GTK_TEXT_VIEW(m_view), &iter, &trailing, buffer_x, buffer_y);

	// Don't show a tooltip if the character is a newline character */
	//if(gtk_text_iter_is_end(&iter)) return false;
	if(gtk_text_iter_ends_line(&iter)) return false;

	// Don't show a tooltip if we are past the end of the line
	GdkRectangle newline_location;
	GtkTextIter line_end = iter;
	gtk_text_iter_forward_to_line_end(&line_end);
	gtk_text_view_get_iter_location(
		GTK_TEXT_VIEW(m_view), &line_end, &newline_location);

	if(buffer_x >= newline_location.x &&
	   buffer_y >= newline_location.y)
	{
		return false;
	}

	InfTextGtkBuffer* buffer = INF_TEXT_GTK_BUFFER(
		inf_session_get_buffer(INF_SESSION(m_session)));

	InfTextUser* author = inf_text_gtk_buffer_get_author(buffer, &iter);
	if(author != NULL)
	{
		tooltip->set_markup(Glib::ustring::compose(
			_("Text written by <b>%1</b>"),
			Glib::Markup::escape_text(
				inf_user_get_name(INF_USER(author)))));
	}
	else
	{
		tooltip->set_text(_("Unowned text"));
	}

	return true;
}
