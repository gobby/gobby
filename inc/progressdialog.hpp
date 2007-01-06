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

#ifndef _GOBBY_PROGRESSDIALOG_HPP_
#define _GOBBY_PROGRESSDIALOG_HPP_

#include <gtkmm/dialog.h>
#include <gtkmm/label.h>
#include <gtkmm/progressbar.h>

namespace Gobby
{

class ProgressDialog : public Gtk::Dialog
{
public:
	ProgressDialog(const Glib::ustring& title, Gtk::Window& parent);
	~ProgressDialog();

	void set_status_text(const Glib::ustring& text);
	void set_progress_fraction(double progress);
	void progress_pulse();

	void work();
protected:
	virtual void on_thread();

	virtual void on_work();
	virtual void on_done();

	virtual void on_response(int response_id);

	Gtk::Label m_lbl_state;
	Gtk::ProgressBar m_progress;

	Glib::Thread* m_thread;

	Glib::Dispatcher m_disp_work;
	Glib::Dispatcher m_disp_done;

	Gtk::Window& m_parent;

	int m_quit;
private:
	bool on_idle();
	void thread_entry();
};

}

#endif // _GOBBY_HOSTPROGRESSDIALOG_HPP_
