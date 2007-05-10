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
#include <gtkmm/treemodelsort.h>
#include "buffer_def.hpp"
#include "document_settings.hpp"
#include "togglewindow.hpp"
#include "header.hpp"
#include "folder.hpp"

namespace Gobby
{

/** List showing documents that are available in the session.
 */
class DocumentList: public ToggleWindow
{
public:
	DocumentList(Gtk::Window& parent,
	             DocumentSettings& settings,
	             Header& header,
		     Folder& folder,
		     const Preferences& preferences,
		     Config::ParentEntry& config_entry);

	// Calls from the window
	// TODO: Replace them by signal handlers from buf
	void obby_start(LocalBuffer& buf);
	void obby_end();

	void obby_user_join(const obby::user& user);
	void obby_user_part(const obby::user& user);
	void obby_user_colour(const obby::user& user);
	void obby_document_insert(LocalDocumentInfo& info);
	void obby_document_remove(LocalDocumentInfo& info);
protected:
	void on_user_subscribe(const obby::user& user);
	void on_user_unsubscribe(const obby::user& user);

	void on_subscribe();
	void on_selection_changed();
	void on_row_activated(const Gtk::TreePath& path,
                              Gtk::TreeViewColumn* column);

	LocalBuffer* m_buffer;
	DocumentSettings& m_settings;

	Folder& m_folder;

	Gtk::VBox m_mainbox;
	Gtk::Button m_btn_subscribe;

	Gtk::ScrolledWindow m_scrolled_wnd;
	Gtk::TreeView m_tree_view;
	Glib::RefPtr<Gtk::TreeModelSort> m_sorted;

	Gtk::TreeViewColumn m_view_col;
};

} // namespace obby

#endif // _GOBBY_DOCUMENTLIST_HPP_
