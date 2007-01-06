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

#include "sourceview/sourcebuffer.hpp"
#include "sourceview/private/sourcebuffer_p.hpp"

// Initialize static member
Gtk::SourceBuffer::CppClassType Gtk::SourceBuffer::sourcebuffer_class_;

const Glib::Class& Gtk::SourceBuffer_Class::init()
{
	if(!gtype_)
	{
		class_init_func_ = &SourceBuffer_Class::class_init_function;
		register_derived_type(gtk_source_buffer_get_type() );
	}

	return *this;
}

void Gtk::SourceBuffer_Class::class_init_function(void* g_class,
                                                  void* class_data)
{
	BaseClassType* const klass = static_cast<BaseClassType*>(g_class);
	CppClassParent::class_init_function(klass, class_data);

	// Hier irgendwelche Signale.
}

Glib::ObjectBase* Gtk::SourceBuffer_Class::wrap_new(GObject* o)
{
	return new SourceBuffer( (GtkSourceBuffer*)o);
}

Gtk::SourceBuffer::SourceBuffer()
 : Gtk::TextBuffer(Glib::ConstructParams(sourcebuffer_class_.init()) )
{
}

Gtk::SourceBuffer::SourceBuffer(const Glib::ConstructParams& construct_params)
 : Gtk::TextBuffer(construct_params)
{
}

Gtk::SourceBuffer::SourceBuffer(GtkSourceBuffer* castitem)
 : Gtk::TextBuffer(reinterpret_cast<GtkTextBuffer*>(castitem) )
{
}

/*Gtk::SourceBuffer::SourceBuffer(Glib::RefPtr<TagTable>& tag_table)
 : Gtk::TextBuffer(Glib::ConstructParams(
	sourcebuffer_class_.init(),
	"tag_table",
	Glib::unwrap(tag_table),
	(char*) 0
   ))
{
}*/

Gtk::SourceBuffer::~SourceBuffer()
{
}

GType Gtk::SourceBuffer::get_type()
{
	return sourcebuffer_class_.init().get_type();
}

GType Gtk::SourceBuffer::get_base_type()
{
	return gtk_source_buffer_get_type();
}

GtkSourceBuffer* Gtk::SourceBuffer::gobj_copy()
{
	reference();
	return gobj();
}

Glib::RefPtr<Gtk::SourceBuffer> Gtk::SourceBuffer::create()
{
	return Glib::RefPtr<SourceBuffer>(new SourceBuffer);
}

/*Glib::RefPtr<Gtk::SourceBuffer>
Gtk::SourceBuffer::create(const Glib::RefPtr<TagTable>& tag_table)
{
	return Glib::RefPtr<SourceBuffer>(new SourceBuffer(tag_table) );
}*/

Glib::RefPtr<Gtk::SourceBuffer>
Glib::wrap(GtkSourceBuffer* object, bool take_copy)
{
	return Glib::RefPtr<Gtk::SourceBuffer>(
		dynamic_cast<Gtk::SourceBuffer*>(Glib::wrap_auto(
			reinterpret_cast<GObject*>(object),
			take_copy
		) )
	);
}
