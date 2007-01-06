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

#include <stdexcept>
#include <gtkmm/stock.h>
#include <gtkmm/main.h>
#include "common.hpp"
#include "progressdialog.hpp"

Gobby::ProgressDialog::Thread::Thread(Glib::Dispatcher& done_disp,
                                      Glib::Dispatcher& work_disp):
	m_thread(NULL), m_disp_done(done_disp),
	m_disp_work(work_disp), m_quit(false)
{
}

Gobby::ProgressDialog::Thread::~Thread()
{
}

void Gobby::ProgressDialog::Thread::launch(const entry_slot& entry_func)
{
	// TODO: sigc::bind to on_thread_entry
	m_entry_func = entry_func;

	lock();
	m_thread = Glib::Thread::create(
		sigc::mem_fun(*this, &Thread::on_thread_entry),
		false
	);
	unlock();
}

void Gobby::ProgressDialog::Thread::quit()
{
	lock();
	m_quit = true;
	unlock();
}

bool Gobby::ProgressDialog::Thread::quitting()
{
	return m_quit;
}

void Gobby::ProgressDialog::Thread::assert_running() const
{
	if(Glib::Thread::self() != m_thread)
	{
		throw std::logic_error(
			"Gobby::ProgressDialog::Thread::assert_running"
		);
	}
}

void Gobby::ProgressDialog::Thread::lock()
{
	m_mutex.lock();
}

void Gobby::ProgressDialog::Thread::unlock()
{
	m_mutex.unlock();
}

Glib::Dispatcher& Gobby::ProgressDialog::Thread::done_event()
{
	return m_disp_done;
}

Glib::Dispatcher& Gobby::ProgressDialog::Thread::work_event()
{
	return m_disp_work;
}

void Gobby::ProgressDialog::Thread::on_thread_entry()
{
	try
	{
		lock();
		unlock();

		// Call working function
		m_entry_func(*this);
	}
	catch(Glib::Thread::Exit& e)
	{
		// No need to throw e futher, the thread exits anyhow
	}

	// If the caller told us to quit we remove us silently, otherwise we
	// tell the caller that we have done what we were supposed to.
	if(m_quit)
	{
		// TODO: The dispatcher's destructor writes an odd message to
		// stdout, how to supress it?
		//  - armin, 10-04-2005
		delete this;
	}
	else
	{
		m_thread = NULL; // ???
		m_disp_done.emit();
	}
}

Gobby::ProgressDialog::ProgressDialog(const Glib::ustring& title,
                                      Gtk::Window& parent)
 : Gtk::Dialog(title, parent, true, true), 
   m_lbl_state("", Gtk::ALIGN_CENTER), m_thread(NULL),
   m_parent(parent)
{
	get_vbox()->pack_start(m_lbl_state, Gtk::PACK_SHRINK);
	get_vbox()->pack_start(m_progress, Gtk::PACK_SHRINK);
	get_vbox()->set_spacing(5);

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

	set_border_width(10);
	set_resizable(false);

	Glib::signal_idle().connect(
		sigc::mem_fun(*this, &ProgressDialog::on_idle) );

	show_all();
}

Gobby::ProgressDialog::~ProgressDialog()
{
	// Tell the thread to terminate itself when the dialog has been closed
	// before the thread has finsihed.
	if(m_thread != NULL)
	{
		m_thread->quit();

		m_conn_done.disconnect();
		m_conn_work.disconnect();
	}
}

void Gobby::ProgressDialog::set_status_text(const Glib::ustring& text)
{
	m_lbl_state.set_text(text);
}

void Gobby::ProgressDialog::set_progress_fraction(double progress)
{
	m_progress.set_fraction(progress);
}

void Gobby::ProgressDialog::progress_pulse()
{
	m_progress.pulse();
}

void Gobby::ProgressDialog::work(Thread& thread)
{
	// Make sure that the calling thread is the worker thread
	thread.assert_running();
	thread.work_event().emit();
}

void Gobby::ProgressDialog::lock(Thread& thread)
{
	// Make sure that the calling thread is the worker thread, the main
	// thread does not have to call this function because it is used by
	// the worker thread before querieng data from the dialog. The only
	// time the main thread is locking the mutex is when the user closes
	// the dialog.
	thread.assert_running();

	thread.lock();
	if(thread.quitting() )
	{
		// Exit from the thread if the dialog has been destroys, the
		// user is no more interested in what we have done (otherwise,
		// he would not have closed the dialog).
		thread.unlock();
		throw Glib::Thread::Exit();
	}
}

void Gobby::ProgressDialog::unlock(Thread& thread)
{
	// Make sure that the calling thread is the worker thread.
	thread.assert_running();
	thread.unlock();
}

void Gobby::ProgressDialog::on_work()
{
}

void Gobby::ProgressDialog::on_done()
{
	m_conn_done.disconnect();
	m_conn_work.disconnect();

	// Delete thread on termination
	delete m_thread;
	m_thread = NULL;
}

void Gobby::ProgressDialog::on_response(int response_id)
{
	// Tell the thread to terminate itself when the dialog has been closed
	// before the thread has finsihed
/*	if(m_thread != NULL)
	{
		m_thread->quit();
		m_thread = NULL;
	}*/

	// Response
	Gtk::Dialog::on_response(response_id);
}

bool Gobby::ProgressDialog::on_idle()
{
	// Create the worker thread
	m_thread = new Thread(m_disp_done, m_disp_work);

	// Connect dispatchers
	m_conn_done = m_thread->done_event().connect(
		sigc::mem_fun(*this, &ProgressDialog::on_done) );
	m_conn_work = m_thread->work_event().connect(
		sigc::mem_fun(*this, &ProgressDialog::on_work) );

	// Launch the thread
	m_thread->launch(sigc::mem_fun(*this, &ProgressDialog::on_thread) );
	return false;
}

