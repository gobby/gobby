/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008 Armin Burgmeier <armin@arbur.net>
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

#ifndef _GOBBY_HUE_BUTTON_HPP_
#define _GOBBY_HUE_BUTTON_HPP_

#include <gtkmm/colorbutton.h>
#include <gtkmm/dialog.h>

#include <memory>

namespace Gobby
{

// TODO: This should go to libinftextgtk as InfTextGtkHueButton,
// inheriting directly from GtkButton to provide a clean API.
class HueButton: public Gtk::ColorButton
{
public:
	HueButton(const Glib::ustring& title, Gtk::Window& parent);

	double get_hue() const;
	double get_saturation() const;
	double get_value() const;

	void set_hue(double hue);
	void set_saturation(double saturation);
	void set_value(double value);

protected:
	virtual void on_clicked();

	void on_parent_hide();
	void on_dialog_response(int response_id);

	Glib::ustring m_title;
	Gtk::Window& m_parent;

	std::auto_ptr<Gtk::Dialog> m_dialog;
	GtkWidget* m_hue_chooser;

	double m_saturation;
	double m_value;
};

}
	
#endif // _GOBBY_HUE_BUTTON_HPP_
