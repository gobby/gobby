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

#ifndef _GOBBY_SOURCEVIEW_SOURCELANGUAGE_HPP_
#define _GOBBY_SOURCEVIEW_SOURCELANGUAGE_HPP_

/** C++ Wrapper for GtkSourceLanguage.
 */

#include <glibmm/object.h>
#include <glibmm/slisthandle.h>
#include <gtksourceview/gtksourcelanguage.h>

typedef struct _GtkSourceLanguage GtkSourceLanguage;
typedef struct _GtkSourceLanguageClass GtkSourceLanguageClass;

namespace Gtk
{

class SourceLanguage_Class;

class SourceLanguage : public Glib::Object
{
public:
	typedef SourceLanguage CppObjectType;
	typedef SourceLanguage_Class CppClassType;
	typedef GtkSourceLanguage BaseObjectType;
	typedef GtkSourceLanguageClass BaseClassType;

	virtual ~SourceLanguage();

private:
	friend class SourceLanguage_Class;

	// noncopyable
	SourceLanguage(const SourceLanguage& other);
	SourceLanguage& operator=(const SourceLanguage& other);

//protected:
// TODO: SourceLanguage() is needed by Glib::wrap(), but should be protected,
// though...
public:
	static CppClassType sourcelanguage_class_;
	SourceLanguage();
	explicit SourceLanguage(const Glib::ConstructParams& construct_params);
	explicit SourceLanguage(GtkSourceLanguage* castitem);

public:
	static GType get_type() G_GNUC_CONST;
	static GType get_base_type() G_GNUC_CONST;

	GtkSourceLanguage* gobj()
       		{ return reinterpret_cast<GtkSourceLanguage*>(gobject_); }
	const GtkSourceLanguage* gobj() const
       		{ return reinterpret_cast<GtkSourceLanguage*>(gobject_); }
	GtkSourceLanguage* gobj_copy();

public:
	Glib::ustring get_id() const;
	Glib::ustring get_name() const;
	Glib::ustring get_section() const;

//	Glib::SListHandle<SourceTag> get_tags() const;
	gunichar get_escape_char() const;

	Glib::SListHandle<Glib::ustring> get_mime_types() const;
	void set_mime_types(const Glib::SListHandle<Glib::ustring>& mime_types);

	/*Glib::RefPtr<SourceStyleScheme> get_style_scheme() const;
	void set_style_scheme(const Glib::RefPtr<SourceStyleScheme> scheme);

	Glib::RefPtr<SourceTagStyle> get_tag_style(const Glib::ustring& tag_id);
	void set_tag_style(const Glib::ustring& tag_id,
	                   const Glib::RefPtr<SourceTagStyle> style);

	Glib::RefPtr<SourceTagStyle>
	get_tag_default_style(const Glib::ustring& get_id);*/
};

}

namespace Glib
{
	Glib::RefPtr<Gtk::SourceLanguage> wrap(GtkSourceLanguage* object,
	                                       bool take_copy = false);
}

#endif // _GOBBY_SOURCEVIEW_SOURCELANGUAGE_HPP_
