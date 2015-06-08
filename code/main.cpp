/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2015 Armin Burgmeier <armin@arbur.net>
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

#include "features.hpp"
#include "application.hpp"

int main(int argc, char* argv[])
{
	Glib::RefPtr<Gio::Application> application =
		Gobby::Application::create();

	const int result = application->run(argc, argv);

	// In GTK+ 3.12, there is a reference leak present which makes us
	// end up with one too much reference on the application object.
	// This bug was fixed for GTK+ 3.14.
	// https://bugzilla.gnome.org/show_bug.cgi?id=730383
	//
	// We need the application object to be finalized correctly, so that
	// its destructor runs and the preferences are serialized to disk.
	// Therefore, this hack makes sure that the application object ends up
	// with the expected reference count at this point.
	if(G_OBJECT(application->gobj())->ref_count > 1)
		application->unreference();

	return result;
}
