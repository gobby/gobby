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

#include "sourceview/sourceview.hpp"
#include "sourceview/private/sourceview_p.hpp"

// Initialize static member
Gtk::SourceView::CppClassType Gtk::SourceView::sourceview_class_;

const Glib::Class& Gtk::SourceView_Class::init()
{
	if(!gtype_)
	{
		class_init_func_ = &SourceView_Class::class_init_function;
		register_derived_type(gtk_source_view_get_type() );
	}

	return *this;
}

void Gtk::SourceView_Class::class_init_function(void* g_class, void* class_data)
{
	BaseClassType* const klass = static_cast<BaseClassType*>(g_class);
	CppClassParent::class_init_function(klass, class_data);

	// Hier irgendwelche Signale.
}

Glib::ObjectBase* Gtk::SourceView_Class::wrap_new(GObject* o)
{
	return manage(new SourceView( (GtkSourceView*)(o)));
}

Gtk::SourceView::SourceView()
 : Gtk::TextView(Glib::ConstructParams(sourceview_class_.init()))
{
	// gtkmm-magic creates a GtkTextBuffer instead of a GtkSourceBuffer
	// I could not figure out why, but this line fixes this issue...
	// - armin
	set_buffer(SourceBuffer::create() );
}

Gtk::SourceView::SourceView(const Glib::ConstructParams& construct_params)
 : Gtk::TextView(construct_params)
{
}

Gtk::SourceView::SourceView(GtkSourceView* castitem)
 : Gtk::TextView(reinterpret_cast<GtkTextView*>(castitem) )
{
}

Gtk::SourceView::SourceView(const Glib::RefPtr<SourceBuffer>& buffer)
 : Gtk::TextView(Glib::ConstructParams(sourceview_class_.init(), (char*)0))
{
	set_buffer(buffer);
}

Gtk::SourceView::~SourceView()
{
	destroy_();
}

GType Gtk::SourceView::get_type()
{
	return sourceview_class_.init().get_type();
}

GType Gtk::SourceView::get_base_type()
{
	return gtk_source_view_get_type();
}

Glib::RefPtr<Gtk::SourceBuffer> Gtk::SourceView::get_buffer()
{
	return Glib::RefPtr<Gtk::SourceBuffer>
		::cast_static(Gtk::TextView::get_buffer() );
}

Glib::RefPtr<const Gtk::SourceBuffer> Gtk::SourceView::get_buffer() const
{
	return Glib::RefPtr<const Gtk::SourceBuffer>
		::cast_static(Gtk::TextView::get_buffer() );
}

void Gtk::SourceView::set_buffer(const Glib::RefPtr<SourceBuffer> buffer)
{
	Gtk::TextView::set_buffer(buffer);
}

void Gtk::SourceView::set_show_line_numbers(bool show_line_numbers)
{
	gtk_source_view_set_show_line_numbers(
		gobj(),
		show_line_numbers ? TRUE : FALSE
	);
}

bool Gtk::SourceView::get_show_line_numbers() const
{
	return gtk_source_view_get_show_line_numbers(
		GTK_SOURCE_VIEW(gobject_) ) == TRUE;
}

void Gtk::SourceView::set_show_line_markers(bool show)
{
	gtk_source_view_set_show_line_markers(gobj(), show ? TRUE : FALSE);
}

bool Gtk::SourceView::get_show_line_markers() const
{
	return gtk_source_view_get_show_line_markers(
		GTK_SOURCE_VIEW(gobject_) ) == TRUE;
}

void Gtk::SourceView::set_tabs_width(guint width)
{
	gtk_source_view_set_tabs_width(gobj(), width);
}

guint Gtk::SourceView::get_tabs_width() const
{
	return gtk_source_view_get_tabs_width(GTK_SOURCE_VIEW(gobject_) );
}

void Gtk::SourceView::set_auto_indent(bool enable)
{
	gtk_source_view_set_auto_indent(gobj(), enable ? TRUE : FALSE);
}

bool Gtk::SourceView::get_auto_indent() const
{
	return gtk_source_view_get_auto_indent(
		GTK_SOURCE_VIEW(gobject_) ) == TRUE;
}
void Gtk::SourceView::set_insert_spaces_instead_of_tabs(bool enable)
{
	gtk_source_view_set_insert_spaces_instead_of_tabs(
		gobj(), enable ? TRUE : FALSE);
}

bool Gtk::SourceView::get_insert_spaces_instead_of_tabs() const
{
	return gtk_source_view_get_insert_spaces_instead_of_tabs(
		GTK_SOURCE_VIEW(gobject_) ) == TRUE;
}

void Gtk::SourceView::set_show_margin(bool show)
{
	gtk_source_view_set_show_margin(gobj(), show ? TRUE : FALSE);
}

bool Gtk::SourceView::get_show_margin() const
{
	return gtk_source_view_get_show_margin(
		GTK_SOURCE_VIEW(gobject_) ) == TRUE;
}

void Gtk::SourceView::set_highlight_current_line(bool show)
{
	gtk_source_view_set_highlight_current_line(gobj(), show ? TRUE : FALSE);
}

bool Gtk::SourceView::get_highlight_current_line() const
{
	return gtk_source_view_get_highlight_current_line(
		GTK_SOURCE_VIEW(gobject_) ) == TRUE;
}

void Gtk::SourceView::set_margin(guint margin)
{
	gtk_source_view_set_margin(gobj(), margin);
}

guint Gtk::SourceView::get_margin() const
{
	return gtk_source_view_get_margin(GTK_SOURCE_VIEW(gobject_) ) == TRUE;
}

void Gtk::SourceView::set_smart_home_end(bool enable)
{
	gtk_source_view_set_smart_home_end(gobj(), enable ? TRUE : FALSE);
}

bool Gtk::SourceView::get_smart_home_end() const
{
	return gtk_source_view_get_smart_home_end(
		GTK_SOURCE_VIEW(gobject_) ) == TRUE;
}

Gtk::SourceView* Glib::wrap(GtkSourceView* object, bool take_copy)
{
	return dynamic_cast<Gtk::SourceView*>(Glib::wrap_auto(
		reinterpret_cast<GObject*>(object),
		take_copy
	) );
}
