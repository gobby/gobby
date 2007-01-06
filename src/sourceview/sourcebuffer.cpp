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

	klass->can_undo = &can_undo_callback;
	klass->can_redo = &can_redo_callback;
	klass->highlight_updated = &highlight_updated_callback;
	klass->marker_updated = &marker_updated_callback;
}

void Gtk::SourceBuffer_Class::can_undo_callback(GtkSourceBuffer* self,
                                                gboolean can_undo)
{
	CppObjectType* const obj = dynamic_cast<CppObjectType*>(
		Glib::ObjectBase::_get_current_wrapper((GObject*)self));

	if(obj && obj->is_derived_() )
	{
		try
		{
			obj->on_can_undo(can_undo == TRUE);
		}
		catch(...)
		{
			Glib::exception_handlers_invoke();
		}
	}
	else
	{
		BaseClassType* const base = static_cast<BaseClassType*>(
			g_type_class_peek_parent(G_OBJECT_GET_CLASS(self))
		);

		if(base && base->can_undo)
			(*base->can_undo)(self, can_undo);
	}
}

void Gtk::SourceBuffer_Class::can_redo_callback(GtkSourceBuffer* self,
                                                gboolean can_redo)
{
	CppObjectType* const obj = dynamic_cast<CppObjectType*>(
		Glib::ObjectBase::_get_current_wrapper((GObject*)self));

	if(obj && obj->is_derived_() )
	{
		try
		{
			obj->on_can_redo(can_redo == TRUE);
		}
		catch(...)
		{
			Glib::exception_handlers_invoke();
		}
	}
	else
	{
		BaseClassType* const base = static_cast<BaseClassType*>(
			g_type_class_peek_parent(G_OBJECT_GET_CLASS(self))
		);

		if(base && base->can_redo)
			(*base->can_redo)(self, can_redo);
	}
}

void Gtk::SourceBuffer_Class::highlight_updated_callback(GtkSourceBuffer* self,
                                                         GtkTextIter* begin,
                                                         GtkTextIter* end)
{
	CppObjectType* const obj = dynamic_cast<CppObjectType*>(
		Glib::ObjectBase::_get_current_wrapper((GObject*)self));

	if(obj && obj->is_derived_() )
	{
		try
		{
			obj->on_highlight_updated(
				Glib::wrap(begin),
				Glib::wrap(end)
			);
		}
		catch(...)
		{
			Glib::exception_handlers_invoke();
		}
	}
	else
	{
		BaseClassType* const base = static_cast<BaseClassType*>(
			g_type_class_peek_parent(G_OBJECT_GET_CLASS(self))
		);

		if(base && base->highlight_updated)
			(*base->highlight_updated)(self, begin, end);
	}
}

void Gtk::SourceBuffer_Class::marker_updated_callback(GtkSourceBuffer* self,
                                                      GtkTextIter* pos)
{
	CppObjectType* const obj = dynamic_cast<CppObjectType*>(
		Glib::ObjectBase::_get_current_wrapper((GObject*)self));

	if(obj && obj->is_derived_() )
	{
		try
		{
			obj->on_marker_updated(Glib::wrap(pos) );
		}
		catch(...)
		{
			Glib::exception_handlers_invoke();
		}
	}
	else
	{
		BaseClassType* const base = static_cast<BaseClassType*>(
			g_type_class_peek_parent(G_OBJECT_GET_CLASS(self))
		);

		if(base && base->marker_updated)
			(*base->marker_updated)(self, pos);
	}
}

Glib::ObjectBase* Gtk::SourceBuffer_Class::wrap_new(GObject* o)
{
	return new SourceBuffer( (GtkSourceBuffer*)o);
}

namespace
{

void SourceBuffer_signal_can_undo_callback(GtkSourceBuffer* self,
                                           gboolean can_undo,
                                           void* data)
{
	using namespace Gtk;
	typedef sigc::slot<void, bool> SlotType;

	if(Glib::ObjectBase::_get_current_wrapper((GObject*)self) )
	{
		try
		{
			sigc::slot_base* const slot = 
				Glib::SignalProxyNormal::data_to_slot(data);

			if(slot)
				(*static_cast<SlotType*>(slot))(
					can_undo == TRUE
				);
		}
		catch(...)
		{
			Glib::exception_handlers_invoke();
		}
	}
}

const Glib::SignalProxyInfo SourceBuffer_signal_can_undo_info = 
{
	"can_undo",
	(GCallback) &SourceBuffer_signal_can_undo_callback,
	(GCallback) &SourceBuffer_signal_can_undo_callback
};

void SourceBuffer_signal_can_redo_callback(GtkSourceBuffer* self,
                                           gboolean can_redo,
                                           void* data)
{
	using namespace Gtk;
	typedef sigc::slot<void, bool> SlotType;

	if(Glib::ObjectBase::_get_current_wrapper((GObject*)self) )
	{
		try
		{
			sigc::slot_base* const slot = 
				Glib::SignalProxyNormal::data_to_slot(data);

			if(slot)
				(*static_cast<SlotType*>(slot))(
					can_redo == TRUE
				);
		}
		catch(...)
		{
			Glib::exception_handlers_invoke();
		}
	}
}

const Glib::SignalProxyInfo SourceBuffer_signal_can_redo_info = 
{
	"can_redo",
	(GCallback) &SourceBuffer_signal_can_redo_callback,
	(GCallback) &SourceBuffer_signal_can_redo_callback
};

void SourceBuffer_signal_highlight_updated_callback(GtkSourceBuffer* self,
                                                    GtkTextIter* begin,
                                                    GtkTextIter* end,
                                                    void* data)
{
	using namespace Gtk;
	typedef sigc::slot<
		void,
		const SourceBuffer::iterator&,
		const SourceBuffer::iterator&
	> SlotType;

	if(Glib::ObjectBase::_get_current_wrapper((GObject*)self) )
	{
		try
		{
			sigc::slot_base* const slot = 
				Glib::SignalProxyNormal::data_to_slot(data);

			if(slot)
				(*static_cast<SlotType*>(slot))(
					Glib::wrap(begin),
					Glib::wrap(end)
				);
		}
		catch(...)
		{
			Glib::exception_handlers_invoke();
		}
	}
}

const Glib::SignalProxyInfo SourceBuffer_signal_highlight_updated_info = 
{
	"highlight_updated",
	(GCallback) &SourceBuffer_signal_highlight_updated_callback,
	(GCallback) &SourceBuffer_signal_highlight_updated_callback
};

void SourceBuffer_signal_marker_updated_callback(GtkSourceBuffer* self,
                                                 GtkTextIter* pos,
                                                 void* data)
{
	using namespace Gtk;
	typedef sigc::slot<
		void,
		const SourceBuffer::iterator&
	> SlotType;

	if(Glib::ObjectBase::_get_current_wrapper((GObject*)self) )
	{
		try
		{
			sigc::slot_base* const slot = 
				Glib::SignalProxyNormal::data_to_slot(data);

			if(slot)
				(*static_cast<SlotType*>(slot))(
					Glib::wrap(pos)
				);
		}
		catch(...)
		{
			Glib::exception_handlers_invoke();
		}
	}
}

const Glib::SignalProxyInfo SourceBuffer_signal_marker_updated_info = 
{
	"marker_updated",
	(GCallback) &SourceBuffer_signal_marker_updated_callback,
	(GCallback) &SourceBuffer_signal_marker_updated_callback
};

}

Gtk::SourceBuffer::SourceBuffer()
 : Gtk::TextBuffer(Glib::ConstructParams(sourcebuffer_class_.init()) )
{
}

Gtk::SourceBuffer::SourceBuffer(const Glib::RefPtr<SourceLanguage>& language)
 : Gtk::TextBuffer(Glib::ConstructParams(sourcebuffer_class_.init()) )
{
	set_language(language);
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

Glib::RefPtr<Gtk::SourceBuffer>
Gtk::SourceBuffer::create(const Glib::RefPtr<SourceLanguage>& language)
{
	return Glib::RefPtr<SourceBuffer>(new SourceBuffer(language) );
}

/*Glib::RefPtr<Gtk::SourceBuffer>
Gtk::SourceBuffer::create(const Glib::RefPtr<TagTable>& tag_table)
{
	return Glib::RefPtr<SourceBuffer>(new SourceBuffer(tag_table) );
}*/

void Gtk::SourceBuffer::on_can_undo(bool can_undo)
{
	BaseClassType* const base = static_cast<BaseClassType*>(
		g_type_class_peek_parent(G_OBJECT_GET_CLASS(gobject_))
	);

	if(base && base->can_undo)
		(*base->can_undo)(gobj(), can_undo ? TRUE : FALSE);
}

void Gtk::SourceBuffer::on_can_redo(bool can_redo)
{
	BaseClassType* const base = static_cast<BaseClassType*>(
		g_type_class_peek_parent(G_OBJECT_GET_CLASS(gobject_))
	);

	if(base && base->can_redo)
		(*base->can_redo)(gobj(), can_redo ? TRUE : FALSE);
}

void Gtk::SourceBuffer::on_highlight_updated(const iterator& begin,
                                             const iterator& end)
{
	BaseClassType* const base = static_cast<BaseClassType*>(
		g_type_class_peek_parent(G_OBJECT_GET_CLASS(gobject_))
	);

	if(base && base->highlight_updated)
		(*base->highlight_updated)(
			gobj(),
			const_cast<GtkTextIter*>(begin.gobj()),
			const_cast<GtkTextIter*>(end.gobj())
		);
}

void Gtk::SourceBuffer::on_marker_updated(const iterator& pos)
{
	BaseClassType* const base = static_cast<BaseClassType*>(
		g_type_class_peek_parent(G_OBJECT_GET_CLASS(gobject_))
	);

	if(base && base->marker_updated)
		(*base->marker_updated)(
			gobj(),
			const_cast<GtkTextIter*>(pos.gobj())
		);
}

Glib::SignalProxy1<void, bool> Gtk::SourceBuffer::signal_can_undo()
{
	return Glib::SignalProxy1<void, bool>(
		this,
		&SourceBuffer_signal_can_undo_info
	);
}

Glib::SignalProxy1<void, bool> Gtk::SourceBuffer::signal_can_redo()
{
	return Glib::SignalProxy1<void, bool>(
		this,
		&SourceBuffer_signal_can_redo_info
	);
}

Glib::SignalProxy2<
	void,
	const Gtk::SourceBuffer::iterator&,
	const Gtk::SourceBuffer::iterator&
> Gtk::SourceBuffer::signal_highlight_updated()
{
	return Glib::SignalProxy2<void, const iterator&, const iterator&>(
		this,
		&SourceBuffer_signal_highlight_updated_info
	);
}

Glib::SignalProxy1<
	void,
	const Gtk::SourceBuffer::iterator&
> Gtk::SourceBuffer::signal_marker_updated()
{
	return Glib::SignalProxy1<void, const iterator&>(
		this,
		&SourceBuffer_signal_marker_updated_info
	);
}

bool Gtk::SourceBuffer::get_check_brackets() const
{
	return gtk_source_buffer_get_check_brackets(
		const_cast<GtkSourceBuffer*>(gobj())
	) == TRUE;
}

void Gtk::SourceBuffer::set_check_brackets(bool check_brackets)
{
	return gtk_source_buffer_set_check_brackets(
		gobj(),
		check_brackets ? TRUE : FALSE
	);
}

/*void Gtk::SourceBuffer::set_bracket_match_style(
	const Glib::RefPtr<Gtk::SourceTagStyle>& style
)
{
	gtk_source_buffer_set_bracket_match_style(
		const_cast<GtkSourceBuffer*>(gobj() ),
		style->gobj()
	);
}*/

bool Gtk::SourceBuffer::get_highlight() const
{
	return gtk_source_buffer_get_highlight(
		const_cast<GtkSourceBuffer*>(gobj())
	) == TRUE;
}

void Gtk::SourceBuffer::set_highlight(bool highlight)
{
	gtk_source_buffer_set_highlight(
		gobj(),
		highlight ? TRUE : FALSE
	);
}

gint Gtk::SourceBuffer::get_max_undo_levels() const
{
	return gtk_source_buffer_get_max_undo_levels(
		const_cast<GtkSourceBuffer*>(gobj())
	);
}

void Gtk::SourceBuffer::set_max_undo_levels(gint max_undo_levels)
{
	gtk_source_buffer_set_max_undo_levels(
		gobj(),
		max_undo_levels
	);	
}

Glib::RefPtr<Gtk::SourceLanguage> Gtk::SourceBuffer::get_language() const
{
	GtkSourceBuffer* self = const_cast<GtkSourceBuffer*>(gobj() );
	if(gtk_source_buffer_get_language(self) == NULL)
		return Glib::RefPtr<Gtk::SourceLanguage>(NULL);

	return Glib::wrap(gtk_source_buffer_get_language(self), true);
}

void
Gtk::SourceBuffer::set_language(const Glib::RefPtr<SourceLanguage> language)
{
	// Add new reference to the language because the C instance holds a
	// new reference now.
	if(language)
	{
		// TODO: Is this required?
		g_object_ref(G_OBJECT(language->gobj()) );

		gtk_source_buffer_set_language(
			gobj(),
			language->gobj()
		);
	}
	else
	{
		gtk_source_buffer_set_language(
			gobj(),
			NULL
		);
	}
}

gunichar Gtk::SourceBuffer::get_escape_char() const
{
	return gtk_source_buffer_get_escape_char(
		const_cast<GtkSourceBuffer*>(gobj())
	);
}

void Gtk::SourceBuffer::set_escape_char(gunichar escape_char)
{
	gtk_source_buffer_set_escape_char(
		gobj(),
		escape_char
	);
}

bool Gtk::SourceBuffer::can_undo() const
{
	return gtk_source_buffer_can_undo(
		const_cast<GtkSourceBuffer*>(gobj())
	);
}

bool Gtk::SourceBuffer::can_redo() const
{
	return gtk_source_buffer_can_redo(
		const_cast<GtkSourceBuffer*>(gobj())
	);
}

void Gtk::SourceBuffer::undo()
{
	gtk_source_buffer_undo(gobj() );
}

void Gtk::SourceBuffer::redo()
{
	gtk_source_buffer_redo(gobj() );
}

void Gtk::SourceBuffer::begin_not_undoable_action()
{
	gtk_source_buffer_begin_not_undoable_action(gobj() );
}

void Gtk::SourceBuffer::end_not_undoable_action()
{
	gtk_source_buffer_end_not_undoable_action(gobj() );
}

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
