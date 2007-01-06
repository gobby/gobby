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
#include <obby/position.hpp>
#include <obby/user_table.hpp>
#include <obby/text.hpp>
#include <obby/local_buffer.hpp>
#include "gselector.hpp"
#include "sourceview/sourcebuffer.hpp"

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
	Glib::RefPtr<Gtk::SourceBuffer> get_buffer() const;

	/** @brief Signal that is emitted when the local user wants to insert
	 * text.
	 */
	signal_insert_type insert_event() const;

	/** @brief Signal that is emitted when the local user wants to erase
	 * text.
	 */
	signal_erase_type erase_event() const;
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
	Glib::RefPtr<Gtk::SourceBuffer> m_buffer;

	signal_insert_type m_signal_insert;
	signal_erase_type m_signal_erase;
};

#if 0
class Folder;

class Document : public Gtk::SourceView
{
public:
	typedef std::size_t size_type;

	typedef sigc::signal<void> signal_cursor_moved_type;
	typedef sigc::signal<void> signal_content_changed_type;
	typedef sigc::signal<void> signal_language_changed_type;

	Document(LocalDocumentInfo& doc, const Folder& folder,
	         const Preferences& preferences);

	const LocalDocumentInfo& get_document() const;
	LocalDocumentInfo& get_document();

	/** Writes the current cursor position into row and col.
	 */
	void get_cursor_position(unsigned int& row, unsigned int& col);

	/** Selects the given region and scrolls to the selected text.
	 */
	void set_selection(const Gtk::TextIter& begin,
	                   const Gtk::TextIter& end);

	/** Returns the currently selected text.
	 */
	Glib::ustring get_selection_text() const;

	/** Returns the title of the file.
	 */
	const Glib::ustring& get_title() const;

	/** Returns the path to the file.
	 */
	const Glib::ustring& get_path() const;

	/** Changes the path to the file.
	 */
	void set_path(const Glib::ustring& new_path);

	/** Whether this document has been modified since the last save.
	 */
	bool get_modified() const;

	/** Returns the currently selected Gtk::SourceLanguage.
	 */
	Glib::RefPtr<Gtk::SourceLanguage> get_language() const;

	/** Sets a new Language to use.
	 */
	void set_language(const Glib::RefPtr<Gtk::SourceLanguage>& language);

	/** Returns the currently preferences.
	 */
	const Preferences& get_preferences() const;

	/** Sets new preferences to use.
	 */
	void set_preferences(const Preferences& preferences);

	/** Returns the document content. Equivalent to
	 * get_document().get_whole_buffer(), but it may be used even if the
	 * obby buffer does not exist anymore (in which case get_document()
	 * returns an invalid reference!)
	 */
	Glib::ustring get_content();

v v v v v v v
*************
v v v v v v v
*************
	/** Returns whether the document is displayed with the words wrapped
	 * to the window's width.
	 */
	bool get_word_wrapping() const;

	/** Sets whether the words should be wrapped to the window's width.
	 */
	void set_word_wrapping(bool wrap);

	/** Returns whether line numbers are currently shown for this document.
	 */
	bool get_show_line_numbers() const;

	/** Sets whether to show line numbers for this document.
	 */
	void set_show_line_numbers(bool show);

^ ^ ^ ^ ^ ^ ^
^ ^ ^ ^ ^ ^ ^
	/** Signal which will be emitted if the cursor's position changed.
	 */
	signal_cursor_moved_type cursor_moved_event() const;

	/** Signal which will be emitted if the document's content was changed.
	 */
	signal_content_changed_type content_changed_event() const;

	/** Signal which will be emitted if the document's language has changed.
	 */
	signal_language_changed_type language_changed_event() const;

	/** Calls from the folder.
	 */
	void obby_user_join(const obby::user& user);
	void obby_user_part(const obby::user& user);
	void obby_user_colour(const obby::user& user);

protected:
	/** Obby signal handlers.
	 */
	void on_obby_insert_before(obby::position pos, const std::string& text,
	                           const obby::user* author);
	void on_obby_insert_after(obby::position pos, const std::string& text,
	                          const obby::user* author);
	void on_obby_delete_before(obby::position pos, obby::position len,
	                           const obby::user* author);
	void on_obby_delete_after(obby::position pos, obby::position len,
	                          const obby::user* author);

	void on_obby_change_before();
	void on_obby_change_after();

	/** TextBuffer signal handlers.
	 */
	void on_insert_before(const Gtk::TextBuffer::iterator& begin,
	                      const Glib::ustring& text,
		              int bytes);
	void on_erase_before(const Gtk::TextBuffer::iterator& begin,
	                     const Gtk::TextBuffer::iterator& end);

	void on_insert_after(const Gtk::TextBuffer::iterator& end,
	                     const Glib::ustring& text,
			     int bytes);
	void on_erase_after(const Gtk::TextBuffer::iterator& begin,
	                    const Gtk::TextBuffer::iterator& end);
	void on_apply_tag_after(const Glib::RefPtr<Gtk::TextTag>& tag,
	                        const Gtk::TextBuffer::iterator& begin,
	                        const Gtk::TextBuffer::iterator& end);

	/** Signal handler for the mark_set event to detect cursor movements.
	 */
	void on_mark_set(const Gtk::TextBuffer::iterator& location,
	                 const Glib::RefPtr<Gtk::TextBuffer::Mark>& mark);

	/** Marks the given part of the text as written by <em>user</em>.
	 */
	void update_user_colour(const Gtk::TextBuffer::iterator& begin,
	                        const Gtk::TextBuffer::iterator& end,
	                        const obby::user* user);

	/** Sets intro text for the document, if the user is not subscribed.
	 */
	//void set_intro_text();

	/** Applies the currently used preferences to the view.
	 */
	void apply_preferences();

	/** The underlaying obby document info.
	 */
	LocalDocumentInfo& m_doc;

	/** Preferences for this document.
	 */
	Preferences m_preferences;

	/** Variable to prevent event handlers from endless recursion. After
	 * an obby insert or textbuffer insert has occured, this variable is
	 * set to true, after the event has been handled, to false. If an
	 * obby event arrives and we insert the newly written text into the
	 * buffer, a textbuffer-insert event would occur. But if m_editing is
	 * true, the event is ignored.
	 */
	bool m_editing;

	/** Document title (even available when connection has been lost).
	 */
	Glib::ustring m_title;

	/** Path to the file on the local disc.
	 */
	Glib::ustring m_path;

	signal_cursor_moved_type m_signal_cursor_moved;
	signal_content_changed_type m_signal_content_changed;
	signal_language_changed_type m_signal_language_changed;
private:
	/** Handler for update_user_colour(): It removes the given tag in
	 * the given range if it is a gobby-user-tag.
	 */
	void on_remove_user_colour(Glib::RefPtr<Gtk::TextBuffer::Tag> tag,
	                           const Gtk::TextBuffer::iterator& begin,
	                           const Gtk::TextBuffer::iterator& end);

	void update_tag_colour(const obby::user& user);
};
#endif

}

#endif // _GOBBY_DOCUMENT_HPP_
