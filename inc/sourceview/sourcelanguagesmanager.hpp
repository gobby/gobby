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

#ifndef _GOBBY_SOURCEVIEW_SOURCELANGUAGESMANAGER_HPP_
#define _GOBBY_SOURCEVIEW_SOURCELANGUAGESMANAGER_HPP_

/** C++ Wrapper for GtkSourceLanguagesManager.
 */

#include <glibmm/object.h>
#include <glibmm/slisthandle.h>
#include <gtksourceview/gtksourcelanguagesmanager.h>
#include "sourceview/sourcelanguage.hpp"

typedef struct _GtkSourceLanguagesManager GtkSourceLanguagesManager;
typedef struct _GtkSourceLanguagesManagerClass GtkSourceLanguagesManagerClass;

namespace Gtk
{

class SourceLanguagesManager_Class;

class SourceLanguagesManager : public Glib::Object
{
public:
	typedef SourceLanguagesManager CppObjectType;
	typedef SourceLanguagesManager_Class CppClassType;
	typedef GtkSourceLanguagesManager BaseObjectType;
	typedef GtkSourceLanguagesManagerClass BaseClassType;

	virtual ~SourceLanguagesManager();

private:
	friend class SourceLanguagesManager_Class;
	static CppClassType sourcelanguagesmanager_class_;

	// noncopyable
	SourceLanguagesManager(const SourceLanguagesManager& other);
	SourceLanguagesManager& operator=(const SourceLanguagesManager& other);

protected:
	SourceLanguagesManager();
	explicit SourceLanguagesManager(
		const Glib::ConstructParams& construct_params
	);
	explicit SourceLanguagesManager(GtkSourceLanguagesManager* castitem);

public:
	static GType get_type() G_GNUC_CONST;
	static GType get_base_type() G_GNUC_CONST;

	GtkSourceLanguagesManager* gobj() {
		return reinterpret_cast<GtkSourceLanguagesManager*>(gobject_);
	}

	const GtkSourceLanguagesManager* gobj() const {
		return reinterpret_cast<GtkSourceLanguagesManager*>(gobject_);
	}

	GtkSourceLanguagesManager* gobj_copy();

public:
	Glib::SListHandle<Glib::RefPtr<SourceLanguage> > 
		get_available_languages() const;

	Glib::RefPtr<SourceLanguage>
		get_language_from_mime_type(
			const Glib::ustring& mime_type
		) const;

	Glib::SListHandle<Glib::ustring> get_lang_files_dirs() const;
};

}

namespace Glib
{
	Glib::RefPtr<Gtk::SourceLanguagesManager> wrap(
		GtkSourceLanguagesManager* object,
		bool take_copy = false
	);
}

#endif // _GOBBY_SOURCEVIEW_SOURCELANGUAGESMANAGER_HPP_
