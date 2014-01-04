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

#include "util/asyncoperation.hpp"

#include <glibmm/main.h>

#include <cassert>
#include <cstdio>

Gobby::AsyncOperation::Handle::Handle(AsyncOperation& operation):
	m_operation(&operation)
{
}

Gobby::AsyncOperation::Handle::~Handle()
{
	if(m_operation)
	{
		if(!m_operation->m_finished)
			cancel();

		m_operation->m_handle = NULL;
	}
}

void Gobby::AsyncOperation::Handle::cancel()
{
	assert(m_operation);
	assert(m_operation->m_finished == false);

	m_operation->m_finished = true;
}

Gobby::AsyncOperation::AsyncOperation():
	m_thread(NULL), m_handle(NULL), m_finished(false)
{
}

Gobby::AsyncOperation::~AsyncOperation()
{
	if(m_handle)
		m_handle->m_operation = NULL;
}

std::auto_ptr<Gobby::AsyncOperation::Handle> 
Gobby::AsyncOperation::start(std::auto_ptr<AsyncOperation> operation)
{
	assert(operation->m_thread == NULL);
	assert(operation->m_handle == NULL);
	assert(operation->m_finished == false);

	AsyncOperation* op = operation.release();

	std::auto_ptr<Handle> handle(new Handle(*op));
	op->m_handle = handle.get();

	op->m_thread = Glib::Thread::create(
		sigc::mem_fun(*op, &AsyncOperation::thread_run), true);

	return handle;
}

void Gobby::AsyncOperation::thread_run()
{
	run();

	Glib::signal_idle().connect(
		sigc::mem_fun(*this, &AsyncOperation::done));
}

bool Gobby::AsyncOperation::done()
{
	if(!m_finished)
	{
		// m_handle DTOR cancels the operation
		g_assert(m_handle != NULL);

		m_finished = true;
		finish();
	}

	if(m_handle)
		m_handle->m_operation = NULL;
	delete this;

	return false;
}
