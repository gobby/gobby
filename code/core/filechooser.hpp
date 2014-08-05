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

#ifndef _GOBBY_FILE_CHOOSER_HPP_
#define _GOBBY_FILE_CHOOSER_HPP_

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/window.h>

namespace Gobby
{

// This class manages a common "current folder uri" for file chooser
// dialogs.
class FileChooser
{
public:
	class Dialog: public Gtk::FileChooserDialog
	{
	public:
		Dialog(Gobby::FileChooser& chooser, Gtk::Window& parent,
		       const Glib::ustring& title,
		       Gtk::FileChooserAction action);
		~Dialog();

	protected:
		Gobby::FileChooser& m_chooser;
	};

	FileChooser();

	const std::string& get_current_folder_uri() const;
	void set_current_folder_uri(const std::string& uri);

private:
	std::string m_current_folder_uri;
};

}

#endif // _GOBBY_FILE_CHOOSER_HPP_
