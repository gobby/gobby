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

#ifndef _GOBBY_SOURCEVIEW_SOURCEBUFFER_P_HPP_
#define _GOBBY_SOURCEVIEW_SOURCEBUFFER_P_HPP_

#include <gtkmm/private/textbuffer_p.h>
#include <glibmm/class.h>

namespace Gtk
{

class SourceBuffer_Class : public Glib::Class
{
public:
	friend class SourceBuffer;

	typedef SourceBuffer CppObjectType;
	typedef GtkSourceBuffer BaseObjectType;
	typedef GtkSourceBufferClass BaseClassType;
	typedef TextBuffer_Class CppClassParent;
	typedef GtkTextBufferClass BaseClassParent;

	const Glib::Class& init();

	static void class_init_function(void* g_class, void* class_data);

	static Glib::ObjectBase* wrap_new(GObject*);

protected:
	// Callbacks (default signal handlers):
	// These will call the *_impl member methods, which will then class the
	// existing signal callbacks, if any.
	// You could prevent the original default signal handlers being called
	// by overriding the *_impl method.
	// TODO: .

	// Callbacks (virtual functions):
};

}

#endif // _GOBBY_SOURCEVIEW_SOURCEBUFFER_P_HPP_
