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

#ifndef _GOBBY_DOCUMENT_HPP_
#define _GOBBY_DOCUMENT_HPP_

#include <map>
#include <gtkmm/textbuffer.h>
#include <gtksourceview/gtksourcebuffer.h>
#include <obby/position.hpp>
#include <obby/user_table.hpp>
#include <obby/text.hpp>
#include <obby/local_buffer.hpp>
#include "gselector.hpp"

namespace Gobby
{

/** @brief Implementation of gobby documents for the obby buffer templates.
 *
 * This class stores a complete obby document in a Gtk::SourceBuffer. This
 * has the advantage that the buffer can simply be displayed in a
 * Gtk::SourceView.
 *
 * Note that all positions the document refers to are given in byte indizes
 * rather than character offsets.
 */
class Document: private net6::non_copyable, public sigc::trackable
{
public:
	/** @brief Iterator class to iterate over chunks of the text.
	 *
	 * A chunk is a piece of text that is written by a single user.
	 */
	class chunk_iterator
	{
	public:
		/** @brief Constructor, used by the Document class.
		 */
		chunk_iterator(const Document& doc,
		               const Gtk::TextIter& begin);

		/** @brief Returns the author of this chunk.
		 */
		const obby::user* get_author() const;

		/** @brief Returns the content of this chunk.
		 */
		std::string get_text() const;

		/** @brief Advances to the next chunk in the document.
		 */
		chunk_iterator& operator++();

		/** @brief Advances to the next chunk in the document.
		 */
		chunk_iterator operator++(int);

		/** @brief Simple comparison.
		 */
		bool operator==(const chunk_iterator& other) const;

		/** @brief Simple comparison.
		 */
		bool operator!=(const chunk_iterator& other) const;
	protected:
		/** @brief Internal function that sets m_iter_end to the end
		 * of the current chunk.
		 */
		void proceed_end();

		const Document& m_doc;

		const obby::user* m_author;
		const obby::user* m_next_author;

		Gtk::TextIter m_iter_begin;
		Gtk::TextIter m_iter_end;
	};

	// TODO: Only take user table as soon as the user table has signals
	// like on_user_join and on_user_part
	class template_type
	{
	public:
		// buffer_def cannot be included since it depends on this file
		typedef obby::basic_local_buffer<Document, GSelector>
			buffer_type;

		template_type(); // Default ctor, needed by obby, invalid
		template_type(const buffer_type& buffer);

		const buffer_type& get_buffer() const;

	protected:
		const buffer_type* m_buffer;
	};

	typedef sigc::signal<void, obby::position, const std::string>
		signal_insert_type;
	typedef sigc::signal<void, obby::position, obby::position>
		signal_erase_type;

	/** @brief Creates a new document that belongs to the given buffer.
	 */
	Document(const template_type& tmpl);
	~Document();

	/** @brief Returns TRUE when the document is empty e.g. does not
	 * contain any text.
	 */
	bool empty() const;

	/** @brief Returns the amount of bytes in the document.
	 */
	obby::position size() const;

	/** @brief Extracts a part from the document.
	 */
	obby::text get_slice(obby::position from,
	                     obby::position len) const;

	/** @brief Returns an iterator pointing to the first chunk of the
	 * document.
	 */
	chunk_iterator chunk_begin() const;

	/** @brief Returns an iterator that points past the last chunk of the
	 * document.
	 */
	chunk_iterator chunk_end() const;

	/** @brief Clears the whole document.
	 */
	void clear();

	/** @brief Inserts the given text at the given position in the
	 * document.
	 */
	void insert(obby::position pos,
	            const obby::text& str);

	/** @brief Inserts text written by <em>author</em> at the given
	 * position.
	 */
	void insert(obby::position pos,
	            const std::string& str,
	            const obby::user* author);

	/** @brief Erases text from the document.
	 */
	void erase(obby::position pos,
	           obby::position len);

	/** @brief Inserts the given text at the end of the document.
	 */
	void append(const obby::text& str);

	/** @brief Inserts text written by <em>author</em> at the end
	 * of the document.
	 */
	void append(const std::string& str,
	            const obby::user* author);

	/** @brief Returns the underlaying Gtk::SourceBuffer.
	 */
	GtkSourceBuffer* get_buffer() const;

	/** @brief Signal that is emitted when the local user wants to insert
	 * text.
	 */
	signal_insert_type local_insert_event() const;

	/** @brief Signal that is emitted when a remote user inserted text.
	 */
	signal_insert_type remote_insert_before_event() const;
	signal_insert_type remote_insert_after_event() const;

	/** @brief Signal that is emitted when the local user wants to erase
	 * text.
	 */
	signal_erase_type local_erase_event() const;

	/** @brief Signal that is emitted when a remote user inserted text.
	 */
	signal_erase_type remote_erase_before_event() const;
	signal_erase_type remote_erase_after_event() const;

protected:
	typedef std::list<Glib::RefPtr<const Gtk::TextTag> > tag_list_type;

	/** @brief Callback to adjust the buffer's tag table when a new
	 * user joins.
	 */
	void on_user_join(const obby::user& user);

	/** @brief Callback to adjust the buffer's tag table when a user
	 * has changed its color.
	 */
	void on_user_color(const obby::user& user);

	/** @brief Callback when text is inserted. This tells obby to insert
	 * text into the document.
	 */
	void on_insert_before(const Gtk::TextIter& iter,
	                      const Glib::ustring& text);

	/** @brief Callback when text is inserted. This tags newly inserted
	 * text.
	 */
	void on_insert_after(const Gtk::TextIter& iter,
	                     const Glib::ustring& text);

	/** @brief Callback when text is erased. This tells obby to erase
	 * text from the document.
	 */
	void on_erase_before(const Gtk::TextIter& begin,
	                     const Gtk::TextIter& end);

	/** @brief Denies application of tags we do not want.
	 */
	void on_apply_tag_before(const Glib::RefPtr<Gtk::TextTag>& tag,
	                         const Gtk::TextIter& begin,
	                         const Gtk::TextIter& end);

	/** @brief Returns an iterator that points at the given position.
	 */
	Gtk::TextIter get_iter(obby::position at) const;

	/** @brief Checks whether a tag in the given tag list is a
	 * user tag.
	 */
	const obby::user* author_in_list(const tag_list_type& list) const;

	/** @brief Returns the user that wrote the text the iterator points to.
	 */
	const obby::user* author_at_iter(const Gtk::TextIter& pos) const;

	/** @brief Returns whether an author is toggled at the given position.
	 *
	 * The new author is stored in <em>to</em>.
	 */
	bool author_toggle(const Gtk::TextIter& at,
	                   const obby::user*& to) const;

	/** @brief Moves the iter to the beginning of the next chunk that
	 * was written by the returned user.
	 */
	const obby::user* forward_chunk(Gtk::TextIter& iter) const;

	/** @brief Inserts text at the given position.
	 */
	Gtk::TextIter insert_impl(const Gtk::TextIter& pos,
	                          const obby::text& str);

	/** @brief Inserts text written by <em>author</em> at the given
	 * position.
	 */
	Gtk::TextIter insert_impl(const Gtk::TextIter& pos,
	                          const std::string& str,
	                          const obby::user* author);

	/** @brief Tags a given range of text as written by <em>with</em>.
	 */
	void tag_text(const Gtk::TextIter& begin,
	              const Gtk::TextIter& end,
	              const obby::user* with);

	/** @brief Helper class to use Glib::RefPtr<Gtk::TextTag> as index
	 * of a std::map<>.
	 */
	struct TagCompare
	{
		typedef Glib::RefPtr<const Gtk::TextTag> compare_type;
		inline bool operator()(const compare_type& first,
		                       const compare_type& second) const
		{
			return first->gobj() < second->gobj();
		}
	};

	typedef std::map<
		const obby::user*,
		Glib::RefPtr<Gtk::TextTag>
	> map_user_type;

	typedef std::map<
		Glib::RefPtr<const Gtk::TextTag>,
		const obby::user*,
		TagCompare
	> map_tag_type;

	// Mapping from user to tag and vice versa
	map_user_type m_map_user;
	map_tag_type m_map_tag;
	const obby::user& m_self;

	// Whether text is currently edited, needed to prevent recursion
	// in signal emission
	bool m_editing;
	GtkSourceBuffer* m_buffer;

	signal_insert_type m_signal_local_insert;
	signal_insert_type m_signal_remote_insert_before;
	signal_insert_type m_signal_remote_insert_after;

	signal_erase_type m_signal_local_erase;
	signal_erase_type m_signal_remote_erase_before;
	signal_erase_type m_signal_remote_erase_after;
};

}

#endif // _GOBBY_DOCUMENT_HPP_
