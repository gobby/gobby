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

#ifndef _GOBBY_SOURCEVIEW_SOURCEVIEW_HPP_
#define _GOBBY_SOURCEVIEW_SOURCEVIEW_HPP_

/** C++ Wrapper for GtkSourceView.
 */

#include <gtkmm/textview.h>
#include <gtksourceview/gtksourceview.h>
#include "sourceview/sourcebuffer.hpp"

typedef struct _GtkSourceView GtkSourceView;
typedef struct _GtkSourceViewClass GtkSourceViewClass;

namespace Gtk
{

class SourceView_Class;

class SourceView : public TextView
{
public:
	typedef SourceView CppObjectType;
	typedef SourceView_Class CppClassType;
	typedef GtkSourceView BaseObjectType;
	typedef GtkSourceViewClass BaseClassType;

	virtual ~SourceView();

private:
	friend class SourceView_Class;
	static CppClassType sourceview_class_;

	// noncopyable
	SourceView(const SourceView& other);
	SourceView& operator=(const SourceView& other);

protected:
	explicit SourceView(const Glib::ConstructParams& construct_params);
	explicit SourceView(GtkSourceView* castitem);

public:
	static GType get_type() G_GNUC_CONST;
	static GType get_base_type() G_GNUC_CONST;

	GtkSourceView* gobj()
       		{ return reinterpret_cast<GtkSourceView*>(gobject_); }
	const GtkSourceView* gobj() const
       		{ return reinterpret_cast<GtkSourceView*>(gobject_); }

protected:
	// Default Signal handlers
	// ...
public:
	SourceView();
	explicit SourceView(const Glib::RefPtr<SourceBuffer>& buffer);

	Glib::RefPtr<SourceBuffer> get_buffer();
	Glib::RefPtr<const SourceBuffer> get_buffer() const;
	void set_buffer(const Glib::RefPtr<SourceBuffer> buffer);

	// TODO: Properties fuer das Zeug und so(?)
	bool get_show_line_numbers() const;
	void set_show_line_numbers(bool show_line_numbers);
};

}

namespace Glib
{
	Gtk::SourceView* wrap(GtkSourceView* object, bool take_copy = false);
}

#endif // _GOBBY_SOURCEVIEW_SOURCEVIEW_HPP_
