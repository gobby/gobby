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
#include <gtkmm/scrolledwindow.h>

#include <libinftextgtk/inf-text-gtk-buffer.h>

#include <gtksourceview/gtksourcebuffer.h>

#include "preferences.hpp"
#include "docwindow.hpp"

namespace
{
	GtkWrapMode wrap_mode_from_preferences(const Gobby::Preferences& pref)
	{
		return static_cast<GtkWrapMode>(static_cast<Gtk::WrapMode>(pref.view.wrap_mode));
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
}

Gobby::DocWindow::DocWindow(InfTextSession* session,
                            const Glib::ustring& title,
                            const Preferences& preferences,
			    GtkSourceLanguageManager* manager):
	m_session(session), m_title(title), m_preferences(preferences),
	m_view(GTK_SOURCE_VIEW(gtk_source_view_new())),
	m_info_box(false, 0)
{
	g_object_ref(m_session);

	InfBuffer* buffer = inf_session_get_buffer(INF_SESSION(session));
	m_buffer = GTK_SOURCE_BUFFER(inf_text_gtk_buffer_get_text_buffer(
		INF_TEXT_GTK_BUFFER(buffer)));

	gtk_text_view_set_buffer(GTK_TEXT_VIEW(m_view),
	                         GTK_TEXT_BUFFER(m_buffer));
	gtk_text_view_set_editable(GTK_TEXT_VIEW(m_view), FALSE);
	gtk_source_buffer_set_language(
		m_buffer, get_language_for_title(manager, m_title.c_str()));

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
		m_buffer, m_preferences.view.margin_pos);
	const Pango::FontDescription& desc = m_preferences.appearance.font;
	gtk_widget_modify_font(
		GTK_WIDGET(m_view),
		const_cast<PangoFontDescription*>(desc.gobj()));

	m_info_label.set_selectable(true);
	m_info_label.set_line_wrap(true);
	m_info_label.show();

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

	pack1(*vbox, true, false);
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

void Gobby::DocWindow::set_selection(const Gtk::TextIter& begin,
                                     const Gtk::TextIter& end)
{
	gtk_text_buffer_select_range(
		gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_view)),
		begin.gobj(),
		end.gobj());

	gtk_text_view_scroll_to_mark(
		GTK_TEXT_VIEW(m_view),
		gtk_text_buffer_get_insert(gtk_text_view_get_buffer(
			GTK_TEXT_VIEW(m_view))),
		0.1, FALSE, 0.0, 0.0);
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

void Gobby::DocWindow::set_info(const Glib::ustring& info)
{
	m_info_label.set_text(info);
	m_info_frame.show();
}

void Gobby::DocWindow::unset_info()
{
	m_info_frame.hide();
}

void Gobby::DocWindow::set_active_user(InfTextUser* user)
{
	g_assert(
		inf_user_table_lookup_user_by_id(
			inf_session_get_user_table(INF_SESSION(m_session)),
			inf_user_get_id(INF_USER(user)))
		== INF_USER(user));

	inf_text_gtk_buffer_set_active_user(
		INF_TEXT_GTK_BUFFER(
			inf_session_get_buffer(INF_SESSION(m_session))),
		user);

	if(user != NULL)
		gtk_text_view_set_editable(GTK_TEXT_VIEW(m_view), TRUE);
	else
		gtk_text_view_set_editable(GTK_TEXT_VIEW(m_view), FALSE);
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
