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

#include "sourceview/sourcelanguagesmanager.hpp"
#include "sourceview/private/sourcelanguagesmanager_p.hpp"

// Initialize static member
Gtk::SourceLanguagesManager::CppClassType
	Gtk::SourceLanguagesManager::sourcelanguagesmanager_class_;

const Glib::Class& Gtk::SourceLanguagesManager_Class::init()
{
	if(!gtype_)
	{
		class_init_func_ =
			&SourceLanguagesManager_Class::class_init_function;
		register_derived_type(gtk_source_languages_manager_get_type() );
	}

	return *this;
}

void Gtk::SourceLanguagesManager_Class::class_init_function(void* g_class,
                                                            void* class_data)
{
	BaseClassType* const klass = static_cast<BaseClassType*>(g_class);
	CppClassParent::class_init_function(klass, class_data);
}

Glib::ObjectBase* Gtk::SourceLanguagesManager_Class::wrap_new(GObject* o)
{
	return new SourceLanguagesManager(
		reinterpret_cast<GtkSourceLanguagesManager*>(o)
	);
}

Gtk::SourceLanguagesManager::SourceLanguagesManager()
 : Glib::Object(Glib::ConstructParams(sourcelanguagesmanager_class_.init()) )
{
}

Gtk::SourceLanguagesManager::SourceLanguagesManager(
	const Glib::ConstructParams& construct_params
)
 : Glib::Object(construct_params)
{
}

Gtk::SourceLanguagesManager::SourceLanguagesManager(
	GtkSourceLanguagesManager* castitem
)
 : Glib::Object(reinterpret_cast<GObject*>(castitem) )
{
}

Gtk::SourceLanguagesManager::~SourceLanguagesManager()
{
}

GType Gtk::SourceLanguagesManager::get_type()
{
	return sourcelanguagesmanager_class_.init().get_type();
}

GType Gtk::SourceLanguagesManager::get_base_type()
{
	return gtk_source_languages_manager_get_type();
}

GtkSourceLanguagesManager* Gtk::SourceLanguagesManager::gobj_copy()
{
	reference();
	return gobj();
}

Glib::RefPtr<Gtk::SourceLanguagesManager>
Gtk::SourceLanguagesManager::create()
{
	return Glib::RefPtr<SourceLanguagesManager>(new SourceLanguagesManager);
}

Glib::SListHandle<Glib::RefPtr<Gtk::SourceLanguage> >
Gtk::SourceLanguagesManager::get_available_languages() const
{
	return Glib::SListHandle<Glib::RefPtr<Gtk::SourceLanguage> >(
		const_cast<GSList*>(
			gtk_source_languages_manager_get_available_languages(
				const_cast<GtkSourceLanguagesManager*>(gobj())
			)
		),
		Glib::OWNERSHIP_NONE
	);
}

Glib::RefPtr<Gtk::SourceLanguage>
Gtk::SourceLanguagesManager::get_language_from_mime_type(
	const Glib::ustring& mime_type
) const
{
	return Glib::wrap(
		gtk_source_languages_manager_get_language_from_mime_type(
			const_cast<GtkSourceLanguagesManager*>(gobj()),
			mime_type.c_str()
		),
		true 
	);
			
}

Glib::RefPtr<Gtk::SourceLanguagesManager>
Glib::wrap(GtkSourceLanguagesManager* object, bool take_copy)
{
	return Glib::RefPtr<Gtk::SourceLanguagesManager>(
		dynamic_cast<Gtk::SourceLanguagesManager*>(Glib::wrap_auto(
			reinterpret_cast<GObject*>(object),
			take_copy
		) )
	);
}
