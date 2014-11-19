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

#ifndef _GOBBY_APPLICATION_HPP_
#define _GOBBY_APPLICATION_HPP_

#include "window.hpp"

#include <gtkmm/application.h>

namespace Gobby
{

class Application: public Gtk::Application
{
public:
	static Glib::RefPtr<Application> create();

protected:
	int on_handle_local_options(
		const Glib::RefPtr<Glib::VariantDict>& options_dict);
	virtual void on_startup();

	virtual void on_activate();
	virtual void on_open(const type_vec_files& files,
	                     const Glib::ustring& hint);

	void handle_error(const std::string& message);

	class Data;
	std::auto_ptr<Data> m_data;

	Application(); // loads all the stuff, mostly what main() currently does? Should actually go in startup after GTK is initialized, so we can show error dialogs. Except really really basic stuff that goes in Application ctor. Then creates window...
	std::auto_ptr<Gtk::Window> m_window;
	Gobby::Window* m_gobby_window;
	std::vector<Glib::ustring> m_startup_hostnames;

	// next step: remove header, and create menu model, maybe in another src file... with gresource...
};

}

#endif // _GOBBY_APPLICATION_HPP_
