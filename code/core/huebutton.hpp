/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _GOBBY_HUE_BUTTON_HPP_
#define _GOBBY_HUE_BUTTON_HPP_

#include <gtkmm/colorbutton.h>
#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>

#include <memory>

namespace Gobby
{

// TODO: This should go to libinftextgtk as InfTextGtkHueButton,
// inheriting directly from GtkButton to provide a clean API.
class HueButton: public Gtk::ColorButton
{
public:
	HueButton(GtkColorButton* cobject,
	          const Glib::RefPtr<Gtk::Builder>& builder);
	HueButton(const Glib::ustring& title);

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

	std::auto_ptr<Gtk::Dialog> m_dialog;
	GtkWidget* m_hue_chooser;

	double m_saturation;
	double m_value;
};

}
	
#endif // _GOBBY_HUE_BUTTON_HPP_
