/* Gobby - GTK-based collaborative text editor
 * Copyright (C) 2008-2014 Armin Burgmeier <armin@arbur.net>
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
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "dialogs/password-dialog.hpp"
#include "core/browser.hpp"
#include "core/noteplugin.hpp"
#include "util/gtk-compat.hpp"
#include "util/file.hpp"
#include "util/uri.hpp"
#include "util/i18n.hpp"

#include <atkmm/relationset.h>

#include <gtkmm/stock.h>
#include <gtkmm/image.h>

#include <libinfinity/server/infd-directory.h>
#include <libinfinity/client/infc-browser.h>

gint compare_func(GtkTreeModel* model, GtkTreeIter* first,
                  GtkTreeIter* second, gpointer user_data)
{
	gint result;
	InfBrowser* br_one;
	InfBrowser* br_two;
	InfBrowserIter* bri_one;
	InfBrowserIter* bri_two;
	GtkTreeIter parent;

	result = 0;
	if(gtk_tree_model_iter_parent(model, &parent, first))
	{
		g_assert(gtk_tree_model_iter_parent(model, &parent, second));

		gtk_tree_model_get(
			model, first,
			INF_GTK_BROWSER_MODEL_COL_BROWSER, &br_one,
			INF_GTK_BROWSER_MODEL_COL_NODE, &bri_one,
			-1);
		gtk_tree_model_get(
			model, second,
			INF_GTK_BROWSER_MODEL_COL_BROWSER, &br_two,
			INF_GTK_BROWSER_MODEL_COL_NODE, &bri_two,
			-1);

		if(inf_browser_is_subdirectory(br_one, bri_one) &&
		   !inf_browser_is_subdirectory(br_two, bri_two))
		{
			result = -1;
		}
		else if(!inf_browser_is_subdirectory(br_one, bri_one) &&
		        inf_browser_is_subdirectory(br_two, bri_two))
		{
			result = 1;
		}

		g_object_unref(br_one);
		g_object_unref(br_two);
		inf_browser_iter_free(bri_one);
		inf_browser_iter_free(bri_two);
	}

	if(!result)
	{
		gchar* name_one;
		gchar* name_two;

		gtk_tree_model_get(
			model, first,
			INF_GTK_BROWSER_MODEL_COL_NAME, &name_one,
			-1);
		gtk_tree_model_get(
			model, second,
			INF_GTK_BROWSER_MODEL_COL_NAME, &name_two,
			-1);

		gchar* one = g_utf8_casefold(name_one, -1);
		gchar* two = g_utf8_casefold(name_two, -1);

		result = g_utf8_collate(one, two);

		g_free(name_one);
		g_free(name_two);
		g_free(one);
		g_free(two);
	}

	return result;
}

Gobby::Browser::Browser(Gtk::Window& parent,
                        StatusBar& status_bar,
                        ConnectionManager& connection_manager):
	m_parent(parent),
	m_status_bar(status_bar),
	m_connection_manager(connection_manager),

	m_expander(_("_Direct Connection"), true),
	m_hbox(false, 6),
	m_label_hostname(_("Host Name:")),
	m_entry_hostname(config_filename("recent_hosts"), 5)
{
	m_label_hostname.show();
	m_entry_hostname.get_entry()->signal_activate().connect(
		sigc::mem_fun(*this, &Browser::on_hostname_activate));
	m_entry_hostname.show();

	m_hbox.pack_start(m_label_hostname, Gtk::PACK_SHRINK);
	m_hbox.pack_start(m_entry_hostname, Gtk::PACK_EXPAND_WIDGET);
	m_hbox.show();

	m_expander.add(m_hbox);
	m_expander.show();
	m_expander.property_expanded().signal_changed().connect(
		sigc::mem_fun(*this, &Browser::on_expanded_changed));

	m_browser_store = inf_gtk_browser_store_new(
		connection_manager.get_io(),
		connection_manager.get_communication_manager());
	
	m_sort_model = inf_gtk_browser_model_sort_new(
		INF_GTK_BROWSER_MODEL(m_browser_store));
	gtk_tree_sortable_set_default_sort_func(
		GTK_TREE_SORTABLE(m_sort_model), compare_func, NULL, NULL);

	if(m_connection_manager.get_discovery() != NULL)
	{
		inf_gtk_browser_store_add_discovery(
			m_browser_store,
			m_connection_manager.get_discovery());
	}

	Glib::ustring known_hosts_file = config_filename("known_hosts");

	m_cert_checker = inf_gtk_certificate_manager_new(
		parent.gobj(), m_connection_manager.get_xmpp_manager(),
		known_hosts_file.c_str());

	m_browser_view =
		INF_GTK_BROWSER_VIEW(
			inf_gtk_browser_view_new_with_model(
				INF_GTK_BROWSER_MODEL(m_sort_model)));

	gtk_widget_show(GTK_WIDGET(m_browser_view));
	gtk_container_add(GTK_CONTAINER(m_scroll.gobj()),
	                  GTK_WIDGET(m_browser_view));
	m_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	m_scroll.set_shadow_type(Gtk::SHADOW_IN);
	m_scroll.show();

	g_signal_connect(
		m_browser_store,
		"set-browser",
		G_CALLBACK(&on_set_browser_static),
		this
	);

	g_signal_connect(
		m_browser_view,
		"activate",
		G_CALLBACK(&on_activate_static),
		this
	);

	set_spacing(6);
	pack_start(m_scroll, Gtk::PACK_EXPAND_WIDGET);
	pack_start(m_expander, Gtk::PACK_SHRINK);

	init_accessibility();

	set_focus_child(m_expander);
}

Gobby::Browser::~Browser()
{
	g_object_unref(m_browser_store);
	g_object_unref(m_sort_model);
	g_object_unref(m_cert_checker);
}

void Gobby::Browser::init_accessibility()
{
	Glib::RefPtr<Atk::RelationSet> relation_set;
	Glib::RefPtr<Atk::Relation> relation;
	std::vector<Glib::RefPtr<Atk::Object> > targets;

	// Associate the hostname label with the corresponding entry field.
	Glib::RefPtr<Atk::Object> entry_hostname_acc =
		m_entry_hostname.get_accessible();
	Glib::RefPtr<Atk::Object> label_hostname_acc =
		m_label_hostname.get_accessible();

	relation_set = entry_hostname_acc->get_relation_set();
	targets.push_back(label_hostname_acc);

	relation = Atk::Relation::create(targets, Atk::RELATION_LABELLED_BY);
	relation_set->set_add(relation);
}

bool Gobby::Browser::get_selected(InfBrowser** browser,
                                  InfBrowserIter* iter)
{
	GtkTreeIter tree_iter;
	if(!inf_gtk_browser_view_get_selected(m_browser_view, &tree_iter))
		return false;

	InfBrowser* tmp_browser;
	InfBrowserIter* tmp_iter;

	gtk_tree_model_get(
		GTK_TREE_MODEL(m_sort_model), &tree_iter,
		INF_GTK_BROWSER_MODEL_COL_BROWSER, &tmp_browser,
		-1);

	if(tmp_browser == NULL)
		return false;

	InfBrowserStatus browser_status;
	g_object_get(G_OBJECT(tmp_browser), "status", &browser_status, NULL);
	if(browser_status != INF_BROWSER_OPEN)
	{
		g_object_unref(tmp_browser);
		return true;
	}

	gtk_tree_model_get(
		GTK_TREE_MODEL(m_sort_model), &tree_iter,
		INF_GTK_BROWSER_MODEL_COL_NODE, &tmp_iter,
		-1);

	*browser = tmp_browser;
	*iter = *tmp_iter;

	inf_browser_iter_free(tmp_iter);
	g_object_unref(tmp_browser);

	return true;
}

void Gobby::Browser::set_selected(InfBrowser* browser,
                                  const InfBrowserIter* iter)
{
	GtkTreeIter tree_iter;

	gboolean has_iter = inf_gtk_browser_model_browser_iter_to_tree_iter(
		INF_GTK_BROWSER_MODEL(m_sort_model),
		browser, iter, &tree_iter);
	g_assert(has_iter == TRUE);

	inf_gtk_browser_view_set_selected(m_browser_view, &tree_iter);
}

InfBrowser* Gobby::Browser::connect_to_host(const InfIpAddress* address,
                                            guint port,
                                            unsigned int device_index,
                                            const std::string& hostname)
{
	// Check whether we do have such a connection already:
	InfXmppConnection* xmpp = m_connection_manager.make_connection(
		address, port, device_index, hostname);

	// Should have thrown otherwise:
	g_assert(xmpp != NULL);

	// TODO: Remove erroneous entry with same name, if any, before
	// adding.

	InfBrowser* browser = inf_gtk_browser_store_add_connection(
		m_browser_store, INF_XML_CONNECTION(xmpp),
		hostname.c_str());

	return browser;
}

void Gobby::Browser::add_browser(InfBrowser* browser,
                                 const char* name)
{
	inf_gtk_browser_store_add_browser(
		m_browser_store, browser, name);
}

void Gobby::Browser::on_expanded_changed()
{
	if(m_expander.get_expanded())
	{
		if(GtkCompat::is_realized(m_entry_hostname))
		{
			m_entry_hostname.grab_focus();
		}
		else
		{
			m_entry_hostname.signal_realize().connect(
				sigc::mem_fun(m_entry_hostname,
					&Gtk::Entry::grab_focus));
		}
	}
}

void Gobby::Browser::on_set_browser(GtkTreeIter* iter,
                                    InfBrowser* old_browser,
                                    InfBrowser* new_browser)
{
	if(new_browser)
	{
		if(INFC_IS_BROWSER(new_browser))
		{
			InfcBrowser* browser = INFC_BROWSER(new_browser);
			infc_browser_add_plugin(browser, Plugins::C_TEXT);
			infc_browser_add_plugin(browser, Plugins::C_CHAT);
		}
		else if(INFD_IS_DIRECTORY(new_browser))
		{
			InfdDirectory* directory =
				INFD_DIRECTORY(new_browser);
			infd_directory_add_plugin(directory, Plugins::D_TEXT);
		}
	}
}

void Gobby::Browser::on_activate(GtkTreeIter* iter)
{
	InfBrowser* browser;
	InfBrowserIter* browser_iter;

	gtk_tree_model_get(GTK_TREE_MODEL(m_sort_model), iter,
	                   INF_GTK_BROWSER_MODEL_COL_BROWSER, &browser,
	                   INF_GTK_BROWSER_MODEL_COL_NODE, &browser_iter,
	                   -1);

	m_signal_activate.emit(browser, browser_iter);

	inf_browser_iter_free(browser_iter);
	g_object_unref(browser);
}

void Gobby::Browser::on_hostname_activate()
{
	Glib::ustring str = m_entry_hostname.get_entry()->get_text();
	if(str.empty()) return;

	m_entry_hostname.commit();
	m_entry_hostname.get_entry()->set_text("");

	m_signal_connect.emit(str);
}
