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

#include "core/textsessionview.hpp"
#include "util/i18n.hpp"
#include "util/color.hpp"

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textiter.h>

#include <libinftextgtk/inf-text-gtk-buffer.h>

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

	bool tags_priority_idle_func(Gobby::TextSessionView& view)
	{
		InfTextGtkBuffer* buffer = INF_TEXT_GTK_BUFFER(
			inf_session_get_buffer(
				INF_SESSION(view.get_session())));

		inf_text_gtk_buffer_ensure_author_tags_priority(buffer);

		// I don't know why it does not redraw automatically, perhaps
		// this is a bug.
		gtk_widget_queue_draw(GTK_WIDGET(view.get_text_view()));
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
				sigc::ref(
					*static_cast<Gobby::TextSessionView*>(
						user_data))));
	}
}

Gobby::TextSessionView::TextSessionView(InfTextSession* session,
                                        const Glib::ustring& title,
                                        const Glib::ustring& path,
                                        const Glib::ustring& hostname,
                                        const std::string& info_storage_key,
                                        Preferences& preferences,
                                        GtkSourceLanguageManager* manager):
	SessionView(INF_SESSION(session), title, path, hostname),
	m_info_storage_key(info_storage_key), m_preferences(preferences),
	m_view(GTK_SOURCE_VIEW(gtk_source_view_new()))
{
	InfBuffer* buffer = inf_session_get_buffer(INF_SESSION(session));
	m_buffer = GTK_SOURCE_BUFFER(inf_text_gtk_buffer_get_text_buffer(
		INF_TEXT_GTK_BUFFER(buffer)));

	g_signal_connect_after(
		G_OBJECT(m_view),
		"style-set",
		G_CALLBACK(on_style_set_static),
		this);

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
	set_language(get_language_for_title(manager, title.c_str()));

	m_preferences.user.hue.signal_changed().connect(
		sigc::mem_fun(
			*this, &TextSessionView::on_user_color_changed));
	m_preferences.editor.tab_width.signal_changed().connect(
		sigc::mem_fun(
			*this, &TextSessionView::on_tab_width_changed));
	m_preferences.editor.tab_spaces.signal_changed().connect(
		sigc::mem_fun(
			*this, &TextSessionView::on_tab_spaces_changed));
	m_preferences.editor.indentation_auto.signal_changed().connect(
		sigc::mem_fun(
			*this, &TextSessionView::on_auto_indent_changed));
	m_preferences.editor.homeend_smart.signal_changed().connect(
		sigc::mem_fun(
			*this, &TextSessionView::on_homeend_smart_changed));

	m_preferences.view.wrap_mode.signal_changed().connect(
		sigc::mem_fun(
			*this, &TextSessionView::on_wrap_mode_changed));
	m_preferences.view.linenum_display.signal_changed().connect(
		sigc::mem_fun(
			*this, &TextSessionView::on_linenum_display_changed));
	m_preferences.view.curline_highlight.signal_changed().connect(
		sigc::mem_fun(
			*this,
			&TextSessionView::on_curline_highlight_changed));
	m_preferences.view.margin_display.signal_changed().connect(
		sigc::mem_fun(
			*this, &TextSessionView::on_margin_display_changed));
	m_preferences.view.margin_pos.signal_changed().connect(
		sigc::mem_fun(
			*this, &TextSessionView::on_margin_pos_changed));
	m_preferences.view.bracket_highlight.signal_changed().connect(
		sigc::mem_fun(
			*this,
			&TextSessionView::on_bracket_highlight_changed));
	m_preferences.view.whitespace_display.signal_changed().connect(
		sigc::mem_fun(
			*this,
			&TextSessionView::on_whitespace_display_changed));
	m_preferences.appearance.font.signal_changed().connect(
		sigc::mem_fun(*this, &TextSessionView::on_font_changed));
	m_preferences.appearance.scheme_id.signal_changed().connect(
		sigc::mem_fun(*this, &TextSessionView::on_scheme_changed));
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
	gtk_source_view_set_draw_spaces(
		m_view, m_preferences.view.whitespace_display);
	const Pango::FontDescription& desc = m_preferences.appearance.font;
	gtk_widget_modify_font(
		GTK_WIDGET(m_view),
		const_cast<PangoFontDescription*>(desc.gobj()));

	gtk_widget_show(GTK_WIDGET(m_view));
	Gtk::ScrolledWindow* scroll = Gtk::manage(new Gtk::ScrolledWindow);
	scroll->set_shadow_type(Gtk::SHADOW_IN);
	scroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scroll->gobj()), GTK_WIDGET(m_view));
	scroll->show();

	pack_start(*scroll, Gtk::PACK_EXPAND_WIDGET);
	
	gtk_source_buffer_set_style_scheme(m_buffer, gtk_source_style_scheme_manager_get_scheme(gtk_source_style_scheme_manager_get_default(), static_cast<Glib::ustring>(preferences.appearance.scheme_id).c_str()));
}

void Gobby::TextSessionView::get_cursor_position(unsigned int& row,
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

void Gobby::TextSessionView::set_selection(const GtkTextIter* begin,
                                           const GtkTextIter* end)
{
	gtk_text_buffer_select_range(
		gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view)), begin, end);

	scroll_to_cursor_position(0.1);
}

Glib::ustring Gobby::TextSessionView::get_selected_text() const
{
	GtkTextIter start, end;
	gtk_text_buffer_get_selection_bounds(
		gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view)),
		&start, &end);

	Gtk::TextIter start_cpp(&start), end_cpp(&end);
	return start_cpp.get_slice(end_cpp);
}

void Gobby::TextSessionView::scroll_to_cursor_position(double within_margin)
{
	gtk_text_view_scroll_to_mark(
		GTK_TEXT_VIEW(m_view),
		gtk_text_buffer_get_insert(gtk_text_view_get_buffer(
			GTK_TEXT_VIEW(m_view))),
		within_margin, FALSE, 0.0, 0.0);
}

InfUser* Gobby::TextSessionView::get_active_user() const
{
	InfTextGtkBuffer* buffer = INF_TEXT_GTK_BUFFER(
		inf_session_get_buffer(INF_SESSION(m_session)));
	return INF_USER(inf_text_gtk_buffer_get_active_user(buffer));
}

void Gobby::TextSessionView::set_active_user(InfTextUser* user)
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

	active_user_changed(INF_USER(user));

	if(user != NULL)
	{
		m_undo_grouping.reset(
			new TextUndoGrouping(
				inf_adopted_session_get_algorithm(
					INF_ADOPTED_SESSION(m_session)),
				user, GTK_TEXT_BUFFER(m_buffer)));
	}
	else
	{
		m_undo_grouping.reset(NULL);
	}
}

GtkSourceLanguage* Gobby::TextSessionView::get_language() const
{
	return gtk_source_buffer_get_language(m_buffer);
}

void Gobby::TextSessionView::set_language(GtkSourceLanguage* language)
{
	gtk_source_buffer_set_language(m_buffer, language);
	m_signal_language_changed.emit(language);
}

void Gobby::TextSessionView::on_user_color_changed()
{
	InfTextUser* user = INF_TEXT_USER(get_active_user());

	if(user)
	{
		inf_text_session_set_user_color(get_session(), user,
		                                m_preferences.user.hue);
	}
}

void Gobby::TextSessionView::on_tab_width_changed()
{
	gtk_source_view_set_tab_width(m_view, m_preferences.editor.tab_width);
}

void Gobby::TextSessionView::on_tab_spaces_changed()
{
	gtk_source_view_set_insert_spaces_instead_of_tabs(
		m_view, m_preferences.editor.tab_spaces);
}

void Gobby::TextSessionView::on_auto_indent_changed()
{
	gtk_source_view_set_auto_indent(
		m_view, m_preferences.editor.indentation_auto);
}

void Gobby::TextSessionView::on_homeend_smart_changed()
{
	gtk_source_view_set_smart_home_end(
		m_view, m_preferences.editor.homeend_smart ?
			GTK_SOURCE_SMART_HOME_END_ALWAYS :
			GTK_SOURCE_SMART_HOME_END_DISABLED);
}

void Gobby::TextSessionView::on_wrap_mode_changed()
{
	gtk_text_view_set_wrap_mode(
		GTK_TEXT_VIEW(m_view),
		wrap_mode_from_preferences(m_preferences));
}

void Gobby::TextSessionView::on_linenum_display_changed()
{
	gtk_source_view_set_show_line_numbers(
		m_view, m_preferences.view.linenum_display);
}

void Gobby::TextSessionView::on_curline_highlight_changed()
{
	gtk_source_view_set_highlight_current_line(
		m_view, m_preferences.view.curline_highlight);
}

void Gobby::TextSessionView::on_margin_display_changed()
{
	gtk_source_view_set_show_right_margin(
		m_view, m_preferences.view.margin_display);
}

void Gobby::TextSessionView::on_margin_pos_changed()
{
	gtk_source_view_set_right_margin_position(
		m_view, m_preferences.view.margin_pos);
}

void Gobby::TextSessionView::on_bracket_highlight_changed()
{
	gtk_source_buffer_set_highlight_matching_brackets(
		m_buffer, m_preferences.view.bracket_highlight);
}

void Gobby::TextSessionView::on_whitespace_display_changed()
{
	gtk_source_view_set_draw_spaces(
		m_view, m_preferences.view.whitespace_display);
}

void Gobby::TextSessionView::on_font_changed()
{
	const Pango::FontDescription& desc = m_preferences.appearance.font;
	gtk_widget_modify_font(
		GTK_WIDGET(m_view),
		const_cast<PangoFontDescription*>(desc.gobj()));
}

void Gobby::TextSessionView::on_scheme_changed()
{
	gtk_source_buffer_set_style_scheme(m_buffer, gtk_source_style_scheme_manager_get_scheme(gtk_source_style_scheme_manager_get_default(), static_cast<Glib::ustring>(m_preferences.appearance.scheme_id).c_str()));
}

bool Gobby::TextSessionView::
	on_query_tooltip(int x, int y, bool keyboard_mode,
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

void Gobby::TextSessionView::on_style_set()
{
	GtkStyle* style = gtk_widget_get_style(GTK_WIDGET(m_view));
	g_assert(style != NULL);

	const GdkColor& color = style->base[GTK_STATE_NORMAL];
	double rh = color.red / 65535.0;
	double gs = color.green / 65535.0;
	double bv = color.blue / 65535.0;

	Gobby::rgb_to_hsv(rh, gs, bv);

	double my_sat = gs * 0.5 + 0.3;
	double my_val = (std::pow(bv + 1, 3) - 1) / 7 * 0.6 + 0.4;

	InfTextGtkBuffer* buffer = INF_TEXT_GTK_BUFFER(
		inf_session_get_buffer(INF_SESSION(m_session)));

	inf_text_gtk_buffer_set_saturation_value(buffer, my_sat, my_val);
}
