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

#ifndef _GOBBY_HELP_COMMANDS_HPP_
#define _GOBBY_HELP_COMMANDS_HPP_

#include "core/applicationactions.hpp"

#include <gtkmm/window.h>
#include <gtkmm/aboutdialog.h>
#include <sigc++/trackable.h>

#include <memory>

namespace Gobby
{

class HelpCommands: public sigc::trackable
{
public:
	HelpCommands(Gtk::Application& application,
	             const ApplicationActions& actions);

protected:
	void on_contents();

	void on_about();
	void on_about_response(int response_id);

	Gtk::Application& m_application;

	std::unique_ptr<Gtk::AboutDialog> m_about_dialog;
};

}

#endif // _GOBBY_HELP_COMMANDS_HPP_
