/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
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

#include "dialogs/find-dialog.hpp"
#include "core/folder.hpp"
#include "util/i18n.hpp"

#include <gtkmm/messagedialog.h>
#include <gtkmm/textbuffer.h>

namespace
{
	typedef gboolean (*TextSearchFunc)(
		const GtkTextIter*,
		const gchar*,
		GtkTextSearchFlags,
		GtkTextIter*,
		GtkTextIter*,
		const GtkTextIter*
	);

	const int RESPONSE_FIND = 1;
	const int RESPONSE_REPLACE = 2;
	const int RESPONSE_REPLACE_ALL = 3;
}

Gobby::FindDialog::FindDialog(GtkDialog* cobject,
                              const Glib::RefPtr<Gtk::Builder>& builder):
	Gtk::Dialog(cobject), m_folder(NULL), m_status_bar(NULL)
{
	builder->get_widget("search-for", m_entry_find);
	builder->get_widget("replace-with-label", m_label_replace);
	builder->get_widget("replace-with", m_entry_replace);

	builder->get_widget("match-case", m_check_case);
	builder->get_widget("match-entire-word-only", m_check_whole_word);
	builder->get_widget("search-backwards", m_check_backwards);
	builder->get_widget("wrap-around", m_check_wrap_around);

	m_entry_find->signal_changed().connect(
		sigc::mem_fun(*this, &FindDialog::on_find_text_changed));
	m_entry_replace->signal_changed().connect(
		sigc::mem_fun(*this, &FindDialog::on_replace_text_changed));

	add_button(_("_Close"), Gtk::RESPONSE_CLOSE);
	m_button_replace_all = add_button(_("Replace _All"),
	                                  RESPONSE_REPLACE_ALL);
	m_button_replace = add_button(_("_Replace"), RESPONSE_REPLACE);
	add_button(_("_Find"), RESPONSE_FIND);

	set_default_response(RESPONSE_FIND);
}

std::auto_ptr<Gobby::FindDialog>
Gobby::FindDialog::create(Gtk::Window& parent, const Folder& folder,
                          StatusBar& status_bar)
{
	Glib::RefPtr<Gtk::Builder> builder =
		Gtk::Builder::create_from_resource(
			"/de/0x539/gobby/ui/find-dialog.ui");

	FindDialog* dialog_ptr;
	builder->get_widget_derived("FindDialog", dialog_ptr);
	std::auto_ptr<FindDialog> dialog(dialog_ptr);

	dialog->set_transient_for(parent);
	dialog->m_folder = &folder;
	dialog->m_status_bar = &status_bar;

	folder.signal_document_changed().connect(
		sigc::mem_fun(*dialog, &FindDialog::on_document_changed));

	// For initial sensitivity:
	dialog->on_document_changed(folder.get_current_document());
	return dialog;

}

Gobby::FindDialog::~FindDialog()
{
	on_document_changed(NULL);
}

bool Gobby::FindDialog::get_search_only() const
{
	return m_label_replace->get_visible();
}

void Gobby::FindDialog::set_search_only(bool search_only)
{
	if(search_only)
	{
		m_label_replace->hide();
		m_entry_replace->hide();
		m_button_replace->hide();
		m_button_replace_all->hide();
	}
	else
	{
		m_label_replace->show();
		m_entry_replace->show();
		m_button_replace->show();
		m_button_replace_all->show();
	}

	set_title(search_only ? _("Find") : _("Replace") );
}

Glib::ustring Gobby::FindDialog::get_find_text() const
{
	return m_entry_find->get_text();
}

Glib::ustring Gobby::FindDialog::get_replace_text() const
{
	return m_entry_replace->get_text();
}

bool Gobby::FindDialog::find_next()
{
	bool result = find_and_select(NULL, SEARCH_FORWARD);
	if(!result)
	{
		Glib::ustring str = Glib::ustring::compose(
			_("Phrase \"%1\" has not been found"),
			get_find_text());

		m_status_bar->add_info_message(str, 5);
		return false;
	}

	return true;
}

bool Gobby::FindDialog::find_previous()
{
	bool result = find_and_select(NULL, SEARCH_BACKWARD);
	if(!result)
	{
		Glib::ustring str = Glib::ustring::compose(
			_("Phrase \"%1\" has not been found"),
			get_find_text());

		m_status_bar->add_info_message(str, 5);
		return false;
	}

	return true;
}

void Gobby::FindDialog::on_show()
{
	Gtk::Dialog::on_show();
	m_entry_find->grab_focus();
}

void Gobby::FindDialog::on_response(int id)
{
	switch(id)
	{
	case Gtk::RESPONSE_CLOSE:
		hide();
		break;
	case RESPONSE_FIND:
		find();
		break;
	case RESPONSE_REPLACE:
		replace();
		break;
	case RESPONSE_REPLACE_ALL:
		replace_all();
		break;
	}

	Gtk::Dialog::on_response(id);
}

void Gobby::FindDialog::on_document_changed(SessionView* view)
{
	m_active_user_changed_connection.disconnect();
	TextSessionView* text_view = dynamic_cast<TextSessionView*>(view);

	if(text_view != NULL)
	{
		m_active_user_changed_connection =
			text_view->signal_active_user_changed().connect(
				sigc::mem_fun(
					*this,
					&FindDialog::on_active_user_changed));
	}

	update_sensitivity();
}

void Gobby::FindDialog::on_active_user_changed(InfUser* user)
{
	update_sensitivity();
}

void Gobby::FindDialog::on_find_text_changed()
{
	update_sensitivity();
	m_signal_find_text_changed.emit();
}

void Gobby::FindDialog::on_replace_text_changed()
{
	m_signal_replace_text_changed.emit();
}

Gobby::FindDialog::SearchDirection Gobby::FindDialog::get_direction() const
{
	if(m_check_backwards->get_active())
		return SEARCH_BACKWARD;
	else
		return SEARCH_FORWARD;
}

bool Gobby::FindDialog::find()
{
	if(get_direction() == SEARCH_FORWARD)
		return find_next();
	else
		return find_previous();
}

bool Gobby::FindDialog::replace()
{
	SessionView* view = m_folder->get_current_document();
	TextSessionView* text_view = dynamic_cast<TextSessionView*>(view);
	g_assert(text_view != NULL);

	// Get selected string
	Glib::ustring sel_str = text_view->get_selected_text();
	Glib::ustring find_str = get_find_text();

	// Lowercase both if we are comparing insensitive
	if(!m_check_case->get_active() )
	{
		sel_str = sel_str.casefold();
		find_str = find_str.casefold();
	}

	// Replace them if they are the same
	if(sel_str == find_str)
	{
		GtkTextBuffer* buffer =
			GTK_TEXT_BUFFER(text_view->get_text_buffer());

		// Replace occurrence
		Glib::ustring replace_text = get_replace_text();
		gtk_text_buffer_delete_selection(buffer, TRUE, TRUE);
		gtk_text_buffer_insert_at_cursor(buffer, replace_text.c_str(),
		                                 replace_text.bytes());

		// and find the next
		find_and_select(NULL, get_direction());
		return true;
	}
	else
	{
		// Search the first occurrence
		return find();
	}
}

bool Gobby::FindDialog::replace_all()
{
	// TODO: Add helper function to get textsessionview? Maybe even add
	// to Folder?
	SessionView* view = m_folder->get_current_document();
	TextSessionView* text_view = dynamic_cast<TextSessionView*>(view);
	g_assert(text_view != NULL);

	GtkTextIter begin;
	GtkTextBuffer* buffer = GTK_TEXT_BUFFER(text_view->get_text_buffer());
	gtk_text_buffer_get_start_iter(buffer, &begin);

	unsigned int replace_count = 0;
	GtkTextIter match_start, match_end;
	while(find_range(&begin, NULL, SEARCH_FORWARD,
	                 &match_start, &match_end))
	{
		Glib::ustring replace_text = get_replace_text();
		gtk_text_buffer_delete(buffer, &match_start, &match_end);
		gtk_text_buffer_insert(buffer, &match_start,
		                       replace_text.c_str(),
		                       replace_text.bytes());

		++ replace_count;
		begin = match_start;
	}

	Glib::ustring message;
	bool result;

	if(replace_count == 0)
	{
		message = _("No occurrence has been replaced");
		result = false;
	}
	else
	{
		message = Glib::ustring::compose(
			ngettext("%1 occurrence has been replaced",
			         "%1 occurrences have been replaced",
			         replace_count), replace_count);
		result = true;
	}

	m_status_bar->add_info_message(message, 5);
	return result;
}

bool Gobby::FindDialog::find_and_select(const GtkTextIter* from,
                                        SearchDirection direction)
{
	SessionView* view = m_folder->get_current_document();
	TextSessionView* text_view = dynamic_cast<TextSessionView*>(view);
	g_assert(text_view != NULL);

	const GtkTextIter* real_begin = from;
	GtkTextIter insert_iter;

	// Search from cursor position if from is not given
	if(from == NULL)
	{
		GtkTextBuffer* buffer =
			GTK_TEXT_BUFFER(text_view->get_text_buffer());
		GtkTextMark* mark = gtk_text_buffer_get_insert(buffer);
		gtk_text_buffer_get_iter_at_mark(buffer, &insert_iter, mark);
		real_begin = &insert_iter;
	}

	GtkTextIter match_start, match_end;
	if(find_wrap(real_begin, direction, &match_start, &match_end))
	{
		if(direction == SEARCH_FORWARD)
			text_view->set_selection(&match_end, &match_start);
		else
			text_view->set_selection(&match_start, &match_end);

		return true;
	}

	return false;
}

bool Gobby::FindDialog::find_wrap(const GtkTextIter* from,
                                  SearchDirection direction,
                                  GtkTextIter* match_start,
                                  GtkTextIter* match_end)
{
	SessionView* view = m_folder->get_current_document();
	TextSessionView* text_view = dynamic_cast<TextSessionView*>(view);
	g_assert(text_view != NULL);

	GtkTextIter start_pos = *from;

	bool result = find_range(&start_pos, NULL, direction,
	                         match_start, match_end);
	if(result == true) return true;

	if(!m_check_wrap_around->get_active()) return false;

	// Wrap around
	GtkTextIter restart_pos;
	GtkTextBuffer* buffer = GTK_TEXT_BUFFER(text_view->get_text_buffer());

	if(direction == SEARCH_FORWARD)
		gtk_text_buffer_get_start_iter(buffer, &restart_pos);
	else
		gtk_text_buffer_get_end_iter(buffer, &restart_pos);
		
	// Limit to search to: Normally the position where we started.
	GtkTextIter* relimit = &start_pos;
	if(direction == SEARCH_BACKWARD)
	{
		// ???
		gtk_text_iter_forward_chars(&start_pos,
		                            get_find_text().length());
		if(gtk_text_iter_is_end(&start_pos))
			relimit = NULL;
	}

	return find_range(&restart_pos, relimit, direction,
	                  match_start, match_end);
}

bool Gobby::FindDialog::find_range(const GtkTextIter* from,
                                   const GtkTextIter* to,
                                   SearchDirection direction,
                                   GtkTextIter* match_start,
                                   GtkTextIter* match_end)
{
	GtkTextIter start_pos = *from;
	while(find_range_once(&start_pos, to, direction,
	                      match_start, match_end))
	{
		if(m_check_whole_word->get_active() )
		{
			if(!gtk_text_iter_starts_word(match_start) ||
			   !gtk_text_iter_ends_word(match_end))
			{
				if(direction == SEARCH_FORWARD)
					start_pos = *match_end;
				else
					start_pos = *match_start;

				continue;
			}
		}

		return true;
	}

	return false;
}

bool Gobby::FindDialog::find_range_once(const GtkTextIter* from,
                                        const GtkTextIter* to,
                                        SearchDirection direction,
                                        GtkTextIter* match_start,
                                        GtkTextIter* match_end)
{
	GtkTextSearchFlags flags = GtkTextSearchFlags(0);
	if(!m_check_case->get_active() )
		flags = GTK_TEXT_SEARCH_CASE_INSENSITIVE;

	TextSearchFunc search_func = (direction == SEARCH_FORWARD ?
		gtk_text_iter_forward_search :
		gtk_text_iter_backward_search);
	Glib::ustring find_str = m_entry_find->get_text();

	gboolean result = search_func(
		from,
		find_str.c_str(),
		flags,
		match_start,
		match_end,
		to != NULL ? to : NULL
	);

	return result;
}

void Gobby::FindDialog::update_sensitivity()
{
	SessionView* view = m_folder->get_current_document();
	TextSessionView* text_view = dynamic_cast<TextSessionView*>(view);

	bool find_sensitivity =
		(!m_entry_find->get_text().empty() && text_view != NULL);
	bool replace_sensitivity =
		(find_sensitivity && text_view->get_active_user() != NULL);

	set_response_sensitive(RESPONSE_FIND, find_sensitivity);
	set_response_sensitive(RESPONSE_REPLACE, replace_sensitivity);
	set_response_sensitive(RESPONSE_REPLACE_ALL, replace_sensitivity);
}
