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

#include <gtkmm/stock.h>
#include <gtkmm/main.h>
#include "common.hpp"
#include "progressdialog.hpp"

Gobby::ProgressDialog::ProgressDialog(const Glib::ustring& title,
                                      Gtk::Window& parent)
 : Gtk::Dialog(title, parent, true, true), 
   m_lbl_state("", Gtk::ALIGN_CENTER), m_parent(parent)
{
	get_vbox()->pack_start(m_lbl_state, Gtk::PACK_SHRINK);
	get_vbox()->pack_start(m_progress, Gtk::PACK_SHRINK);
	get_vbox()->set_spacing(5);

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

	set_border_width(10);
	set_resizable(false);

	Glib::signal_idle().connect(
		sigc::mem_fun(*this, &ProgressDialog::on_idle) );
	m_disp_work.connect(
		sigc::mem_fun(*this, &ProgressDialog::on_work) );
	m_disp_done.connect(
		sigc::mem_fun(*this, &ProgressDialog::on_done) );

	show_all();
}

Gobby::ProgressDialog::~ProgressDialog()
{
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

void Gobby::ProgressDialog::work()
{
	m_disp_work.emit();
}

void Gobby::ProgressDialog::on_thread()
{
	// Exit thread if we have to quit
	if(g_atomic_int_get(&m_quit) != 0)
		throw Glib::Thread::Exit();
}

void Gobby::ProgressDialog::on_work()
{
}

void Gobby::ProgressDialog::on_done()
{
	m_thread->join();
	m_thread = NULL;
}

void Gobby::ProgressDialog::on_response(int response_id)
{
	// Is the thread working?
	if(m_thread)
	{
		// Set text
		m_lbl_state.set_text(_("Waiting for thread to finish...") );
		// Show text
		while(Gtk::Main::events_pending() )
			Gtk::Main::iteration();
		// Tell thread to quit
		g_atomic_int_add(&m_quit, 1);
		// Wait for the thread
		m_thread->join();
		m_thread = NULL;
	}

	// Response
	Gtk::Dialog::on_response(response_id);
}

bool Gobby::ProgressDialog::on_idle()
{
	// Launch the worker thread
	m_quit = 0;
	m_thread = Glib::Thread::create(
		sigc::mem_fun(*this, &ProgressDialog::thread_entry),
		true
	);

	return false;
}

void Gobby::ProgressDialog::thread_entry()
{
	on_thread();
	m_disp_done.emit();
}

