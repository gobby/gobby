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
	// ...
protected:
	SourceBuffer();
	// explicit SourceBuffer(const Glib::RefPtr<TagTable>& tag_table);

public:
	static Glib::RefPtr<SourceBuffer> create();
	// static Glib::RefPtr<SourceBuffer> create(const Glib::RefPtr<TagTable>& tag_table);
	
};

}

namespace Glib
{
	Glib::RefPtr<Gtk::SourceBuffer> wrap(GtkSourceBuffer* object,
	                                     bool take_copy = false);
}

#endif // _GOBBY_SOURCEVIEW_SOURCEBUFFER_HPP_
