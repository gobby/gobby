/* gobby - A GTKmm driven libobby client
 * Copyright (C) 2005, 2006 0x539 dev group
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

#ifndef _GOBBY_JOINPROGRESSDIALOG_HPP_
#define _GOBBY_JOINPROGRESSDIALOG_HPP_

#include <gtkmm/colorbutton.h>
#include <obby/error.hpp>
#include "buffer_def.hpp"
#include "progressdialog.hpp"
#include "colorsel.hpp"
#include "config.hpp"

namespace Gobby
{

class JoinProgressDialog: public ProgressDialog
{
public:
	JoinProgressDialog(Gtk::Window& parent,
	                   Config::ParentEntry& config_entry,
	                   const Glib::ustring& hostname,
			   unsigned int port,
	                   const net6::address* addr,
	                   const Glib::ustring& username,
	                   const Gdk::Color& color);

	/** Never call this function twice because the auto_ptr of the
	 * JoinDialog will be reset to NULL after having transferred the data
	 * to the caller.
	 */
	std::auto_ptr<ClientBuffer> get_buffer();

private:
	typedef ClientBuffer::connection_settings connection_settings;

	class Prompt: public Gtk::Dialog
	{
	protected:
		Prompt(Gtk::Window& parent,
		       const Glib::ustring& title,
		       const Glib::ustring& info,
		       const Gtk::StockID& icon);

		void set_custom_widget(Widget& widget);

		Gtk::Table m_table;
		Gtk::Label m_info;
		Gtk::Image m_icon;
	};

	class NamePrompt: public Prompt
	{
	public:
		NamePrompt(Gtk::Window& parent,
		           const Glib::ustring& initial_name);

		Glib::ustring get_name() const;
	protected:
		void on_change();

		const Glib::ustring m_initial_name;

		Gtk::HBox m_box;
		Gtk::Label m_label;
		Gtk::Entry m_entry;
	};

	class ColorPrompt: public Prompt
	{
	public:
		ColorPrompt(Gtk::Window& parent,
		            Config::ParentEntry& config_entry,
		            const Gdk::Color& initial_color);

		Gdk::Color get_color() const;
	protected:
		ColorButton m_button;
	};

	class SessionPasswordPrompt: public Prompt
	{
	public:
		SessionPasswordPrompt(Gtk::Window& parent);

		Glib::ustring get_password() const;
	protected:
		void on_change();

		Gtk::HBox m_box;
		Gtk::Label m_label;
		Gtk::Entry m_entry;
	};

	class UserPasswordPrompt: public Prompt
	{
	public:
		UserPasswordPrompt(Gtk::Window& parent,
		                   const Glib::ustring& initial_name);

		Glib::ustring get_name() const;
		Glib::ustring get_password() const;
	protected:
		void on_change();

		const Glib::ustring m_initial_name;

		Gtk::Table m_table;
		Gtk::Label m_lbl_name;
		Gtk::Label m_lbl_password;
		Gtk::Entry m_ent_name;
		Gtk::Entry m_ent_password;
	};

	virtual void on_thread(Thread& thread);
	virtual void on_done();

	void on_welcome();
	void on_login_failed(obby::login::error error);

	bool on_prompt_name(connection_settings& settings);
	bool on_prompt_colour(connection_settings& settings);
	bool on_prompt_global_password(connection_settings& settings);
	bool on_prompt_user_password(connection_settings& settings);

	void on_sync_init(unsigned int count);
	void on_sync_final();
	void on_close();

	void display_error(const Glib::ustring& message);

	virtual void on_response(int response_id);

	Config::ParentEntry& m_config_entry;

	Glib::ustring m_hostname;
	unsigned int m_port;
	std::auto_ptr<net6::address> m_address;
	Glib::ustring m_username;
	Gdk::Color m_color;

	Glib::ustring m_error;

	std::auto_ptr<ClientBuffer> m_buffer;

	// Got done signal from connection thread
	bool m_got_done;
	// Got welcome packet
	bool m_got_welcome;
};

} // namespace Gobby

#endif // _GOBBY_JOINPROGRESSDIALOG_HPP_
