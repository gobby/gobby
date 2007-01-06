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

#ifndef _GOBBY_FILEENTRY_HPP_
#define _GOBBY_FILEENTRY_HPP_

#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooserdialog.h>

namespace Gobby
{

/** Entry field with a browse button next to the entry.
 */
class FileEntry: public Gtk::HBox
{
public:
	FileEntry(const Glib::ustring& title);
	FileEntry(Gtk::Window& parent, const Glib::ustring& title);

	Glib::ustring get_text() const;
	void set_text(const Glib::ustring& text);

	Gtk::FileChooser& get_file_chooser();
	const Gtk::FileChooser& get_file_chooser() const;

protected:
	virtual void on_browse();

	Gtk::Entry m_ent_file;
	Gtk::Button m_btn_browse;
	Gtk::FileChooserDialog m_dialog;

private:
	void init();
};

}
	
#endif // _GOBBY_FILEENTRY_HPP_
