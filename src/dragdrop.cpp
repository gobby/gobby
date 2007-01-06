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

#include <string>
#include <vector>

#ifdef WIN32
#include <windows.h>
#include <gdk/gdkwin32.h>
#endif

#include "dragdrop.hpp"
#include "window.hpp"

namespace
{
#ifdef WIN32
	HWND hwnd_from_window(Gtk::Window& window)
	{
		return reinterpret_cast<HWND>(
			GDK_WINDOW_HWND(window.get_window()->gobj() )
		);
	}

	class DropTarget: public IDropTarget
	{
	public:
		DropTarget(Gobby::Window& window);

		HRESULT __stdcall QueryInterface(REFIID iid,
		                                 void** ppvObject);
		ULONG __stdcall AddRef();
		ULONG __stdcall Release();

		HRESULT __stdcall DragEnter(IDataObject* pDataObject,
		                            DWORD grfKeyState,
		                            POINTL pt,
		                            DWORD* pdwEffect);
		HRESULT __stdcall DragOver(DWORD grfKeyState,
		                           POINTL pt,
		                           DWORD* pdwEffect);
		HRESULT __stdcall DragLeave();
		HRESULT __stdcall Drop(IDataObject* pDataObject,
		                       DWORD grfKeyState,
		                       POINTL pt,
		                       DWORD* pdwEffect);

	protected:
		DWORD drop_effect(DWORD allowed);

		Gobby::Window& m_window;

		long m_refcount;
		bool m_allow_drop;
	};

	DropTarget::DropTarget(Gobby::Window& window):
		m_window(window), m_refcount(1), m_allow_drop(false)
	{
	}

	HRESULT __stdcall DropTarget::QueryInterface(REFIID iid,
	                                             void** ppvObject)
	{
		if(iid == IID_IDropTarget || iid == IID_IUnknown)
		{
			AddRef();
			*ppvObject = this;
			return S_OK;
		}
		else
		{
			*ppvObject = NULL;
			return E_NOINTERFACE;
		}
	}

	ULONG __stdcall DropTarget::AddRef()
	{
		return InterlockedIncrement(&m_refcount);
	}

	ULONG __stdcall DropTarget::Release()
	{
		LONG count = InterlockedDecrement(&m_refcount);

		if(count == 0)
		{
			delete this;
			return 0;
		}

		return count;
	}

	HRESULT __stdcall DropTarget::DragEnter(IDataObject* pDataObject,
	                                        DWORD grfKeyState,
	                                        POINTL pt,
	                                        DWORD* pdwEffect)
	{
		FORMATETC fmtetc = {
			CF_HDROP,
			0,
			DVASPECT_CONTENT,
			-1,
			TYMED_HGLOBAL
		};

		m_allow_drop = pDataObject->QueryGetData(&fmtetc) == S_OK;
		*pdwEffect = drop_effect(*pdwEffect);

		return S_OK;
	}

	HRESULT __stdcall DropTarget::DragOver(DWORD grfKeyState,
	                                       POINTL pt,
	                                       DWORD* pdwEffect)
	{
		*pdwEffect = drop_effect(*pdwEffect);
		return S_OK;
	}

	HRESULT __stdcall DropTarget::DragLeave()
	{
		return S_OK;
	}

	HRESULT __stdcall DropTarget::Drop(IDataObject* pDataObject,
	                                   DWORD grfKeyState,
	                                   POINTL pt,
	                                   DWORD* pdwEffect)
	{
		*pdwEffect = drop_effect(*pdwEffect);

		if(m_allow_drop == false)
			return S_OK;

		FORMATETC fmtetc = {
			CF_HDROP,
			0,
			DVASPECT_CONTENT,
			-1,
			TYMED_FILE
		};

		if(pDataObject->QueryGetData(&fmtetc) != S_OK)
			return S_OK;

		STGMEDIUM stgmed;
		if(pDataObject->GetData(&fmtetc, &stgmed) != S_OK)
			return S_OK;

		HDROP drop = static_cast<HDROP>(GlobalLock(stgmed.hGlobal) );

		GlobalUnlock(stgmed.hGlobal);

		UINT file_count = DragQueryFileA(drop, 0xffffffff, NULL, 0);
		for(UINT i = 0; i < file_count; ++ i)
		{
			UINT size = DragQueryFileA(drop, i, NULL, 0);
			char* buf = new char[size + 1];
			DragQueryFileA(drop, i, buf, size + 1);

			m_window.open_local_file(buf);
			delete[] buf;
		}

		ReleaseStgMedium(&stgmed);
		return S_OK;
	}

	DWORD DropTarget::drop_effect(DWORD allowed)
	{
		allowed &= DROPEFFECT_COPY;
		if(!m_allow_drop || !allowed)
			allowed = DROPEFFECT_NONE;

		return allowed;
	}

#else
	void drag_data_received_unix(const Glib::RefPtr<Gdk::DragContext>& ctx,
	                             int x,
	                             int y,
	                             const Gtk::SelectionData& data,
	                             guint info,
	                             guint time,
				     Gobby::Window& window)
	{
		// We only accepted uri lists
		if(data.get_target() != "text/uri-list")
		{
			throw std::logic_error(
				"<anonymous>::drag_data_received_unix"
			);
		}

		std::vector<std::string> files = data.get_uris();
		for(std::vector<std::string>::iterator iter = files.begin();
		    iter != files.end();
		    ++ iter)
		{
			std::string filename;
			try
			{
				filename = Glib::filename_from_uri(*iter);
			}
			catch(Glib::ConvertError& e)
			{
				Gtk::MessageDialog dlg(
					window,
					e.what(),
					false,
					Gtk::MESSAGE_ERROR,
					Gtk::BUTTONS_OK,
					true
				);

				dlg.run();
			}

			window.open_local_file(filename);
		}
	}
#endif
}

Gobby::DragDrop::DragDrop(Window& window):
	m_window(window), m_handle(NULL)
{
#ifdef WIN32
	// TODO: Error checking
	OleInitialize(NULL);
	DropTarget* drop_target = new DropTarget(m_window);
	RegisterDragDrop(hwnd_from_window(window), drop_target);

	m_handle = drop_target;
#else
	std::list<Gtk::TargetEntry> targets;
	targets.push_back(Gtk::TargetEntry("text/uri-list") );
	window.drag_dest_set(targets);

	window.signal_drag_data_received().connect(
		sigc::bind(
			sigc::ptr_fun(&drag_data_received_unix),
			sigc::ref(window)
		)
	);
#endif
}

Gobby::DragDrop::~DragDrop()
{
#ifdef WIN32
	static_cast<DropTarget*>(m_handle)->Release();
	RevokeDragDrop(hwnd_from_window(m_window) );
	OleUninitialize();
#else
	m_window.drag_dest_unset();
#endif
}
