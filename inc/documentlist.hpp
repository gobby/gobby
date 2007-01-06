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

#ifndef _GOBBY_DOCUMENTLIST_HPP_
#define _GOBBY_DOCUMENTLIST_HPP_

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <obby/local_buffer.hpp>
#include "togglewindow.hpp"
#include "header.hpp"

namespace Gobby
{

/** List showing documents that are available in the session.
 */
class DocumentList: public ToggleWindow
{
public:
	class Columns: public Gtk::TreeModel::ColumnRecord
	{
	public:
		Columns();

		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
		Gtk::TreeModelColumn<Glib::ustring> text;
		Gtk::TreeModelColumn<Gdk::Color> color;
		Gtk::TreeModelColumn<void*> data;
	};

	DocumentList(Gtk::Window& parent,
	             Header& header,
		     const Preferences& preferences,
		     Config::Entry& config_entry);

	// Calls from the window
	// TODO: Replace them by signal handlers from buf
	virtual void obby_start(obby::local_buffer& buf);
	virtual void obby_end();
	virtual void obby_user_join(const obby::user& user);
	virtual void obby_user_part(const obby::user& user);
	virtual void obby_user_colour(const obby::user& user);
	virtual void obby_document_insert(obby::basic_local_document_info<obby::document, net6::selector>& info);
	virtual void obby_document_remove(obby::basic_local_document_info<obby::document, net6::selector>& info);
protected:
	Gtk::TreeIter find_iter(const obby::basic_local_document_info<obby::document, net6::selector>& info) const;

	virtual void on_user_subscribe(const obby::user& user,
	                               const obby::basic_local_document_info<obby::document, net6::selector>& info);
	virtual void on_user_unsubscribe(const obby::user& user,
	                                 const obby::basic_local_document_info<obby::document, net6::selector>& info);

	virtual void on_subscribe();
	virtual void on_selection_changed();
	virtual bool on_tree_button_press(GdkEventButton* event);

	/** Reference to header.
	 */
	Header& m_header;

	obby::local_buffer* m_buffer;

	/** GUI components.
	 */
	Gtk::VBox m_mainbox;
	Gtk::Button m_btn_subscribe;

	Gtk::ScrolledWindow m_scrolled_wnd;
	Gtk::TreeView m_tree_view;
	Glib::RefPtr<Gtk::TreeStore> m_tree_data;
	Columns m_tree_cols;

	Gtk::TreeViewColumn m_view_col;
};

} // namespace obby

#endif // _GOBBY_DOCUMENTLIST_HPP_
