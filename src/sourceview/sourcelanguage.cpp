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

#include "sourceview/sourcelanguage.hpp"
#include "sourceview/private/sourcelanguage_p.hpp"

// Initialize static member
Gtk::SourceLanguage::CppClassType Gtk::SourceLanguage::sourcelanguage_class_;

const Glib::Class& Gtk::SourceLanguage_Class::init()
{
	if(!gtype_)
	{
		class_init_func_ = &SourceLanguage_Class::class_init_function;
		register_derived_type(gtk_source_language_get_type() );
	}

	return *this;
}

void Gtk::SourceLanguage_Class::class_init_function(void* g_class,
                                                  void* class_data)
{
	BaseClassType* const klass = static_cast<BaseClassType*>(g_class);
	CppClassParent::class_init_function(klass, class_data);

	// Hier irgendwelche Signale.
}

Glib::ObjectBase* Gtk::SourceLanguage_Class::wrap_new(GObject* o)
{
	return new SourceLanguage(reinterpret_cast<GtkSourceLanguage*>(o) );
}

Gtk::SourceLanguage::SourceLanguage()
 : Glib::Object(Glib::ConstructParams(sourcelanguage_class_.init()))
{
}

Gtk::SourceLanguage::SourceLanguage(
	const Glib::ConstructParams& construct_params
)
 : Glib::Object(construct_params)
{
}

Gtk::SourceLanguage::SourceLanguage(GtkSourceLanguage* castitem)
 : Glib::Object(reinterpret_cast<GObject*>(castitem) )
{
}

/*Gtk::SourceLanguage::SourceLanguage(Glib::RefPtr<TagTable>& tag_table)
 : Gtk::TextLanguage(Glib::ConstructParams(
	sourcebuffer_class_.init(),
	"tag_table",
	Glib::unwrap(tag_table),
	(char*) 0
   ))
{
}*/

Gtk::SourceLanguage::~SourceLanguage()
{
}

GType Gtk::SourceLanguage::get_type()
{
	return sourcelanguage_class_.init().get_type();
}

GType Gtk::SourceLanguage::get_base_type()
{
	return gtk_source_language_get_type();
}

GtkSourceLanguage* Gtk::SourceLanguage::gobj_copy()
{
	reference();
	return gobj();
}

Glib::ustring Gtk::SourceLanguage::get_id() const
{
	return gtk_source_language_get_id(
		const_cast<GtkSourceLanguage*>(gobj())
	);
}

Glib::ustring Gtk::SourceLanguage::get_name() const
{
	return gtk_source_language_get_name(
		const_cast<GtkSourceLanguage*>(gobj())
	);
}

Glib::ustring Gtk::SourceLanguage::get_section() const
{
	return gtk_source_language_get_section(
		const_cast<GtkSourceLanguage*>(gobj())
	);
}

gunichar Gtk::SourceLanguage::get_escape_char() const
{
	return gtk_source_language_get_escape_char(
		const_cast<GtkSourceLanguage*>(gobj())
	);
}

Glib::SListHandle<Glib::ustring> Gtk::SourceLanguage::get_mime_types() const
{
	return Glib::SListHandle<Glib::ustring>(
		gtk_source_language_get_mime_types(
			const_cast<GtkSourceLanguage*>(gobj())
		),
		Glib::OWNERSHIP_DEEP
	);
}

void Gtk::SourceLanguage::set_mime_types(
	const Glib::SListHandle<Glib::ustring>& mime_types
)
{
	gtk_source_language_set_mime_types(
		gobj(),
		mime_types.data()
	);
}

Glib::RefPtr<Gtk::SourceLanguage>
Glib::wrap(GtkSourceLanguage* object, bool take_copy)
{
	// The Code below does not work - don't know why
	return Glib::RefPtr<Gtk::SourceLanguage>(new Gtk::SourceLanguage(GTK_SOURCE_LANGUAGE( (GObject*)(object))));
/*	return Glib::RefPtr<Gtk::SourceLanguage>(
		dynamic_cast<Gtk::SourceLanguage*>(Glib::wrap_auto(
			reinterpret_cast<GObject*>(object),
			take_copy
		) )
	);*/
}
