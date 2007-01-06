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

#ifndef _GOBBY_ENCODING_SELECTOR_HPP_
#define _GOBBY_ENCODING_SELECTOR_HPP_

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/window.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/filechooserdialog.h>

namespace Gobby
{

class EncodingSelector: public Gtk::ComboBoxText
{
public:
	static const std::string AUTO_DETECT;

	EncodingSelector();

	void set_encoding(const std::string& encoding);
	std::string get_encoding() const;

	void set_show_automatic(bool show_automatic);
	bool get_show_automatic() const;
protected:
	bool m_show_automatic;

	bool row_sep_func(const Glib::RefPtr<Gtk::TreeModel>& model,
	                  const Gtk::TreeIter& iter);
};

class EncodingFileChooserDialog: public Gtk::FileChooserDialog
{
public:
	EncodingFileChooserDialog(const Glib::ustring& title,
	                          Gtk::FileChooserAction action);
	EncodingFileChooserDialog(Gtk::Window& parent,
	                          const Glib::ustring& title,
	                          Gtk::FileChooserAction action);

	EncodingSelector& get_selector();
	const EncodingSelector& get_selector() const;

protected:
	Gtk::HBox m_hbox;
	Gtk::Label m_label;
	EncodingSelector m_selector;

	void init_impl(Gtk::FileChooserAction action);
};

}

#endif // _GOBBY_ENCODING_SELECTOR_HPP_
