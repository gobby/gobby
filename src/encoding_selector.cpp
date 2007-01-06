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

#include <stdexcept>
#include <gtkmm/liststore.h>

#include "common.hpp"
#include "encoding.hpp"
#include "encoding_selector.hpp"

const std::string Gobby::EncodingSelector::AUTO_DETECT = "Auto Detect";

Gobby::EncodingSelector::EncodingSelector():
	m_show_automatic(false)
{
	set_row_separator_func(
		sigc::mem_fun(*this, &EncodingSelector::row_sep_func)
	);

	const std::vector<std::string>& encodings = Encoding::get_encodings();
	for(std::vector<std::string>::const_iterator iter = encodings.begin();
	    iter != encodings.end();
	    ++ iter)
	{
		append_text(*iter);
	}
}

void Gobby::EncodingSelector::set_encoding(const std::string& encoding)
{
	set_active_text(encoding);
}

std::string Gobby::EncodingSelector::get_encoding() const
{
	return get_active_text();
}

void Gobby::EncodingSelector::set_show_automatic(bool show_automatic)
{
	if(show_automatic == m_show_automatic) return;
	m_show_automatic = show_automatic;

	if(m_show_automatic)
	{
		prepend_text("Separator");
		prepend_text(AUTO_DETECT);
	}
	else
	{
		remove_text(AUTO_DETECT);
		remove_text("Separator");
	}
}

bool Gobby::EncodingSelector::get_show_automatic() const
{
	return m_show_automatic;
}

void Gobby::EncodingSelector::remove_text(const Glib::ustring& text)
{
	Glib::RefPtr<Gtk::ListStore> list =
		Glib::RefPtr<Gtk::ListStore>::cast_dynamic(get_model());

	if(!list)
	{
		throw std::logic_error(
			"Gobby::EncodingSelector::remove_text:\n"
			"Underlaying TreeModel is not a liststore"
		);
	}

	Gtk::TreeNodeChildren children = list->children();

	Gtk::TreeIter next_iter;
	for(Gtk::TreeIter iter = children.begin();
	    iter != children.end();
	    iter = next_iter)
	{
		next_iter = iter;
		++ next_iter;

		if( (*iter)[m_text_columns.m_column] == text)
			iter = list->erase(iter);
	}
}

bool Gobby::EncodingSelector::
	row_sep_func(const Glib::RefPtr<Gtk::TreeModel>& model,
	             const Gtk::TreeIter& iter)
{
	// I hope noone names his character encoding "Separator" :)
	return (*iter)[m_text_columns.m_column] == "Separator";
}

Gobby::EncodingFileChooserDialog::
	EncodingFileChooserDialog(const Glib::ustring& title,
	                          Gtk::FileChooserAction action):
	Gtk::FileChooserDialog(title, action)
{
	init_impl(action);
}

Gobby::EncodingFileChooserDialog::
	EncodingFileChooserDialog(Gtk::Window& parent,
	                          const Glib::ustring& title,
	                          Gtk::FileChooserAction action):
	Gtk::FileChooserDialog(parent, title, action)
{
	init_impl(action);
}

Gobby::EncodingSelector& Gobby::EncodingFileChooserDialog::get_selector()
{
	return m_selector;
}

const Gobby::EncodingSelector&
Gobby::EncodingFileChooserDialog::get_selector() const
{
	return m_selector;
}

void Gobby::EncodingFileChooserDialog::init_impl(Gtk::FileChooserAction action)
{
	if(action == Gtk::FILE_CHOOSER_ACTION_OPEN)
		m_selector.set_show_automatic(true);

	m_label.set_text(_("Character Encoding:") );

	m_hbox.pack_start(m_label, Gtk::PACK_SHRINK);
	m_hbox.pack_start(m_selector, Gtk::PACK_EXPAND_WIDGET);
	m_hbox.set_spacing(8);

	m_hbox.show_all();
	get_vbox()->pack_start(m_hbox, Gtk::PACK_SHRINK);
}

