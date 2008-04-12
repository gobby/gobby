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

#ifndef _GOBBY_DRAGDROP_HPP_
#define _GOBBY_DRAGDROP_HPP_

#include <sigc++/trackable.h>

namespace Gobby
{

class Window;

/** Class that handles Drag+Drop in the main application window.
 */

class DragDrop: public sigc::trackable
{
public:
	DragDrop(Window& window);
	~DragDrop();

protected:
	Window& m_window;
	void* m_handle;
};

}

#endif // _GOBBY_DRAGDROP_HPP_
