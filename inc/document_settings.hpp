/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005, 2006 0x539 dev group
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

#ifndef _GOBBY_DOCUMENT_SETTINGS_HPP_
#define _GOBBY_DOCUMENT_SETTINGS_HPP_

#include <map>
#include <gtkmm/liststore.h>
#include "buffer_def.hpp"

namespace Gobby
{

class Window;

/** @brief DocumentSettings stores several settings for a document such as
 * its original_encoding and the path it is saved to.
 */
class DocumentSettings: public sigc::trackable, private net6::non_copyable
{
public:
	class Columns: public Gtk::TreeModel::ColumnRecord
	{
	public:
		Columns();

		Gtk::TreeModelColumn<LocalDocumentInfo*> info;
		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
		Gtk::TreeModelColumn<Gdk::Color> color;
		Gtk::TreeModelColumn<Glib::ustring> title;
		Gtk::TreeModelColumn<Glib::ustring> original_encoding;
		Gtk::TreeModelColumn<Glib::ustring> path;
	};

	DocumentSettings(Window& wnd);

	typedef sigc::signal<void, const LocalDocumentInfo&>
		signal_document_insert_type;
	typedef sigc::signal<void, const LocalDocumentInfo&>
		signal_document_remove_type;

	/** @brief Called by the window when a new session has been opened.
	 *
	 * TODO: Window should provide a signal.
	 */
	void obby_start(LocalBuffer& buf);

	/** @brief Called by the window when the session has been closed.
	 */
	void obby_end();

	/** @brief Returns the original encoding for the given document info.
	 *
	 * The original encoding is the encoding the file has been opened with.
	 * Gobby uses always UTF-8 as encoding because the document is stored
	 * in a Gtk::TextBuffer, this field is used to convert the file back
	 * to its original encoding when the file is saved to disk.
	 */
	Glib::ustring
	get_original_encoding(const LocalDocumentInfo& info) const;

	/** @brief Changes the original encoding of a document.
	 */
	void set_original_encoding(const LocalDocumentInfo& info,
	                           const Glib::ustring& encoding);

	/** @brief Returns the save path of a document.
	 *
	 * The save path is the path where a document has been loaded from.
	 * It is used to store the document back to the original file when
	 * a normal save is invoked.
	 */
	Glib::ustring get_path(const LocalDocumentInfo& info) const;

	/** @brief Changes the save path of a document.
	 */
	void set_path(const LocalDocumentInfo& info,
	              const Glib::ustring& path);

	/** @brief Returns the columns for the underlaying ListStore.
	 */
	//Columns& get_columns();

	/** @brief Returns the columns for the underlaying ListStore.
	 */
	const Columns& get_columns() const;

	/** @brief Returns the underlaying list that may be displayed by
	 * a Gtk::TreeView.
	 */
	Glib::RefPtr<Gtk::ListStore> get_list();

	/** @brief Returns the underlaying list that may be displayed by
	 * a Gtk::TreeView.
	 */
	Glib::RefPtr<const Gtk::ListStore> get_list() const;

	/** @brief Signal that is emitted when a document has been added to
	 * the document settings.
	 */
	signal_document_insert_type document_insert_event() const;

	/** @brief Signal that is emitted when a document will be removed from
	 * the document settings.
	 *
	 * The signal is emitted before the entry is actually removed, so
	 * signal handlers may still query document path or encoding.
	 */
	signal_document_remove_type document_remove_event() const;

private:
	void on_document_insert(DocumentInfo& info);
	void on_document_remove(DocumentInfo& info);
	void on_document_rename(DocumentInfo& info);

	void on_subscribe(const obby::user& user,
	                  LocalDocumentInfo& info);
	void on_unsubscribe(const obby::user& user,
	                    LocalDocumentInfo& info);

	Gtk::TreeIter get_iter(const LocalDocumentInfo& info) const;

	typedef std::map<const LocalDocumentInfo*, Gtk::TreeIter> map_type;

	Glib::RefPtr<Gtk::ListStore> m_data;
	Columns m_cols;

	Glib::RefPtr<Gdk::Pixbuf> m_icon;

	// Map for faster access
	map_type m_map;

	signal_document_insert_type m_signal_document_insert;
	signal_document_remove_type m_signal_document_remove;
};

} // namespace Gobby

#endif // _GOBBY_DOCUMENT_SETTINGS_HPP_
