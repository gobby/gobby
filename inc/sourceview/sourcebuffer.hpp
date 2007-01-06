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

#ifndef _GOBBY_SOURCEVIEW_SOURCEBUFFER_HPP_
#define _GOBBY_SOURCEVIEW_SOURCEBUFFER_HPP_

/** C++ Wrapper for GtkSourceBuffer.
 */

#include <gtkmm/textbuffer.h>
#include <gtksourceview/gtksourcebuffer.h>
#include "sourceview/sourcelanguage.hpp"

typedef struct _GtkSourceBuffer GtkSourceBuffer;
typedef struct _GtkSourceBufferClass GtkSourceBufferClass;

namespace Gtk
{

class SourceBuffer_Class;

class SourceBuffer : public TextBuffer
{
public:
	typedef SourceBuffer CppObjectType;
	typedef SourceBuffer_Class CppClassType;
	typedef GtkSourceBuffer BaseObjectType;
	typedef GtkSourceBufferClass BaseClassType;

private:
	friend class SourceBuffer_Class;
	static CppClassType sourcebuffer_class_;

	// noncopyable
	SourceBuffer(const SourceBuffer& other);
	SourceBuffer& operator=(const SourceBuffer& other);

protected:
	explicit SourceBuffer(const Glib::ConstructParams& construct_params);
	explicit SourceBuffer(GtkSourceBuffer* castitem);

public:
	typedef TextBuffer::iterator iterator;

	virtual ~SourceBuffer();

	static GType get_type() G_GNUC_CONST;
	static GType get_base_type() G_GNUC_CONST;

	GtkSourceBuffer* gobj()
       		{ return reinterpret_cast<GtkSourceBuffer*>(gobject_); }
	const GtkSourceBuffer* gobj() const
       		{ return reinterpret_cast<GtkSourceBuffer*>(gobject_); }
	GtkSourceBuffer* gobj_copy();

protected:
	// Default Signal handlers
	virtual void on_can_undo(bool can_undo);
	virtual void on_can_redo(bool can_redo);
	virtual void on_highlight_updated(const iterator& begin,
	                                  const iterator& end);
	virtual void on_marker_updated(const iterator& pos);

protected:
	SourceBuffer();
	explicit SourceBuffer(const Glib::RefPtr<SourceLanguage>& language);
	// explicit SourceBuffer(const Glib::RefPtr<TagTable>& tag_table);

public:
	static Glib::RefPtr<SourceBuffer> create();
	static Glib::RefPtr<SourceBuffer> create(
		const Glib::RefPtr<SourceLanguage>& language
	);

	// static Glib::RefPtr<SourceBuffer> create(const Glib::RefPtr<TagTable>& tag_table);

	bool get_check_brackets() const;
	void set_check_brackets(bool check_brackets);
//	void set_bracket_match_style(const Glib::RefPtr<SourceTagStyle>& style);
	
	bool get_highlight() const;
	void set_highlight(bool highlight);

	gint get_max_undo_levels() const;
	void set_max_undo_levels(gint max_undo_levels);

	Glib::RefPtr<SourceLanguage> get_language() const;
	void set_language(const Glib::RefPtr<SourceLanguage> language);

	gunichar get_escape_char() const;
	void set_escape_char(gunichar escape_char);

	bool can_undo() const;
	bool can_redo() const;

	void undo();
	void redo();

	void begin_not_undoable_action();
	void end_not_undoable_action();

	// TODO: Marker stuff

	Glib::SignalProxy1<void, bool> signal_can_undo();
	Glib::SignalProxy1<void, bool> signal_can_redo();
	Glib::SignalProxy2<void, const iterator&, const iterator&>
		signal_highlight_updated();
	Glib::SignalProxy1<void, const iterator&> signal_marker_updated();
};

}

namespace Glib
{
	Glib::RefPtr<Gtk::SourceBuffer> wrap(GtkSourceBuffer* object,
	                                     bool take_copy = false);
}

#endif // _GOBBY_SOURCEVIEW_SOURCEBUFFER_HPP_
