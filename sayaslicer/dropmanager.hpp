#pragma once
#if _WIN32

#include <imgui.h>
#include <oleidl.h>
#include <codecvt>

// Mostly copy pasted from https://github.com/ocornut/imgui/issues/2602
class DropManager : public IDropTarget
{
public:
	bool isDragged = false;
	std::string droppedFile = "";

	ULONG AddRef() { return 1; }
	ULONG Release() { return 0; }

	// we handle drop targets, let others know
	HRESULT QueryInterface(REFIID riid, void** ppvObject)
	{
		if (riid == IID_IDropTarget)
		{
			*ppvObject = this;
			return S_OK;
		}

		*ppvObject = NULL;
		return E_NOINTERFACE;
	}

	// occurs when we drag files into our applications view
	HRESULT DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
	{
		// TODO: check whether we can handle this type of object at all and set *pdwEffect &= DROPEFFECT_NONE if not;
		isDragged = true;
		ImGui::GetIO().MouseDown[0] = true;

		*pdwEffect &= DROPEFFECT_COPY;
		return S_OK;
	}

	// occurs when we drag files out from our applications view
	HRESULT DragLeave() {
		isDragged = false;
		return S_OK;
	}

	// occurs when we drag the mouse over our applications view whilst carrying files (post Enter, pre Leave)
	HRESULT DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
	{
		ImGui::GetIO().MousePos = ImVec2(pt.x, pt.y);

		*pdwEffect &= DROPEFFECT_COPY;
		return S_OK;
	}

	// occurs when we release the mouse button to finish the drag-drop operation
	HRESULT Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
	{
		FORMATETC fmte = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM stgm;

		if (SUCCEEDED(pDataObj->GetData(&fmte, &stgm)))
		{
			HDROP hdrop = (HDROP)stgm.hGlobal;
			const UINT filescount = DragQueryFileW(hdrop, 0xFFFFFFFF, NULL, 0);
			if (filescount != 1) {
				std::cout << "Please drag only one file!" << std::endl;
			}
			else {
				const UINT bufsize = DragQueryFileW(hdrop, 0, NULL, 0);
				std::wstring str;
				str.resize(bufsize + 1);
				if (DragQueryFileW(hdrop, 0, &str[0], bufsize + 1))
				{
					std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
					droppedFile = conv.to_bytes(str);
				}
			}

			ReleaseStgMedium(&stgm);
			isDragged = false;
		}
		ImGui::GetIO().MouseDown[0] = false;

		*pdwEffect &= DROPEFFECT_COPY;
		return S_OK;
	}
};

#endif
