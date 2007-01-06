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
	class Thread
	{
	public:
		typedef sigc::slot<void, Thread&> entry_slot;

		Thread();
		~Thread();

		void launch(const entry_slot& entry_func);

		/** Tells the thread to quit as soon as possible. This happens
		 * if the user closed the dialog without waiting for the data
		 * the thread is computing. This means that the user wants to
		 * abort the operation.
		 */
		void quit();

		/** Returns TRUE if the thread has to quit. This may be used
		 * by the thread's entry point to determinate whether to go on
		 * computing some data or whether to return.
		 */
		bool quitting();

		/** Makes sure that the calling thread is this worker thread.
		 */
		void assert_running() const;

		/** Locks the thread's mutex. The thread itself must do this
		 * while requesting any data from the dialog object because the
		 * dialog may get destroyed if the user closes the dialog
		 * while the thread is still working.
		 */
		void lock();

		/** Unlocks the mutex mentioned above.
		 */
		void unlock();

		Glib::Dispatcher& done_event();
		Glib::Dispatcher& work_event();

	protected:
		Glib::Mutex m_mutex;
		Glib::Thread* m_thread;
		entry_slot m_entry_func;
		Glib::Dispatcher m_disp_done;
		Glib::Dispatcher m_disp_work;
		bool m_quit;
	private:
		void on_thread_entry();
	};

	ProgressDialog(const Glib::ustring& title, Gtk::Window& parent);
	virtual ~ProgressDialog();

	void set_status_text(const Glib::ustring& text);
	void set_progress_fraction(double progress);
	void progress_pulse();
protected:
	/** Emits the work dispatcher of the thread to tell the main thread
	 * that the thread has not hung up or something.
	 * @param thread Thread whose mutex to lock. We cannot just use
	 * m_thread because the dialog may already be destroyed.
	 */
	void work(Thread& thread);

	/** Locks the thread's mutex (as mentioned above) and throws
	 * Glib::Thread::Exit if the thread's quitting flag has been set.
	 * This causes the worker thread to exit if the dialog has been closed.
	 */
	void lock(Thread& thread);

	/** Unlocks the thread's mutex.
	 */
	void unlock(Thread& thread);

	virtual void on_thread(Thread& thread) = 0;

	virtual void on_work();
	virtual void on_done();

	virtual void on_response(int response_id);

	Gtk::Label m_lbl_state;
	Gtk::ProgressBar m_progress;

	Thread* m_thread;

	Gtk::Window& m_parent;

private:
	bool on_idle();
};

}

#endif // _GOBBY_HOSTPROGRESSDIALOG_HPP_
