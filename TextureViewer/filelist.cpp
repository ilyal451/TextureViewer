/*
The Texture Viewer Project, http://imagetools.itch.io/texview
Copyright (c) 2011-2024 Ilya Lyutin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "defs.h"
#include "globals.h"

#include <commctrl.h>
#include <shlobj.h>

#include <stdio.h>
#include <string.h>

#include "controls.h"

#include "../shared/utils.h"

#include "format.h"
#include "context.h"

#include "viewer.h"
#include "filelist.h"

#include "resource.h"

// the file list window


wchar_t* ConstructMenuItem( wchar_t* pszItem, int eCmd, wchar_t* buffer );


#define IDC_FILE_LIST_LIST 1001
#define IDC_FILE_LIST_MENUBTN 1002

CFileList* g_pFileList;

int g_iFileListLeft;
int g_iFileListTop;
int g_iFileListWidth;
int g_iFileListHeight;


class CExportFileList
{
public:
	wchar_t* m_pszFileName;
	bool m_bUnicode;
	bool m_bIncludePath;
	bool m_bMarkedOnly;
	CExportFileList();
	static BOOL CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static bool Show( HWND hWndParent );
	void WM_InitDlg(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_Command(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void Exit(bool bApply);
private:
	HWND m_hWnd;
	void Browse( void );
};


CExportFileList* g_pExportFileList;


CExportFileList::CExportFileList()
{
	m_pszFileName = AllocStringW( L"" );
	m_bUnicode = false;
	m_bIncludePath = false;
	m_bMarkedOnly = false;
}


void CExportFileList::Browse( void )
{
	static wchar_t szFileName[MAX_PATH];
	OPENFILENAME ofn;

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.hInstance = g_hInst;
	ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if ( GetSaveFileName(&ofn) )
	{
		if (ofn.nFilterIndex == 1)
		{
			// append .txt if not appended
			wchar_t* pszExt = GetExtension(szFileName);
			if (pszExt == NULL)
			{
				wcscat(szFileName, L".txt");
			}
		}

		SendDlgItemMessage(m_hWnd, IDC_FILE_NAME, WM_SETTEXT, 0, (LPARAM)szFileName);
	}
}


void CExportFileList::Exit(bool bApply)
{
	//if (bApply)
	{
		wchar_t buffer[MAX_PATH];
		SendDlgItemMessage(m_hWnd, IDC_FILE_NAME, WM_GETTEXT, MAX_PATH, (LPARAM)buffer);
		FreeString( m_pszFileName );
		m_pszFileName = AllocStringW( buffer );

		m_bUnicode = SendDlgItemMessage(m_hWnd, IDC_UNICODE, BM_GETCHECK, 0, NULL) == BST_CHECKED ? true : false;
		m_bIncludePath = SendDlgItemMessage(m_hWnd, IDC_INCLUDE_PATH, BM_GETCHECK, 0, NULL) == BST_CHECKED ? true : false;
		m_bMarkedOnly = SendDlgItemMessage(m_hWnd, IDC_MARKED_ONLY, BM_GETCHECK, 0, NULL) == BST_CHECKED ? true : false;
	}

	EndDialog( m_hWnd, bApply ? IDOK : IDCANCEL );
}


void CExportFileList::WM_InitDlg(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_hWnd = hWnd;

	SendDlgItemMessage(m_hWnd, IDC_FILE_NAME, WM_SETTEXT, 0, (LPARAM)m_pszFileName);
	SendDlgItemMessage(m_hWnd, IDC_UNICODE, BM_SETCHECK, m_bUnicode ? BST_CHECKED : BST_UNCHECKED, NULL);
	SendDlgItemMessage(m_hWnd, IDC_INCLUDE_PATH, BM_SETCHECK, m_bIncludePath ? BST_CHECKED : BST_UNCHECKED, NULL);
	SendDlgItemMessage(m_hWnd, IDC_MARKED_ONLY, BM_SETCHECK, m_bMarkedOnly ? BST_CHECKED : BST_UNCHECKED, NULL);

}


void CExportFileList::WM_Command(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) == BN_CLICKED)
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
			Exit(true);
			break;
		case IDCANCEL:
			Exit(false);
			break;
		case IDC_BROWSE:
			Browse();
			break;
		}
	}
}


BOOL CALLBACK CExportFileList::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			g_pExportFileList->WM_InitDlg(hWnd, wParam, lParam);
			break;

		case WM_COMMAND:
			g_pExportFileList->WM_Command(hWnd, wParam, lParam);
			break;

		case WM_CLOSE:
			g_pExportFileList->Exit(false);
			break;

		default:
			return 0;
	}

	return 1;
}


bool CExportFileList::Show( HWND hWndParent )
{
	if ( g_pExportFileList == NULL )
	{
		g_pExportFileList = new CExportFileList();
	}
	return DialogBox( NULL, MAKEINTRESOURCE( IDD_EXPORT_FILE_LIST ), hWndParent, CExportFileList::DlgProc ) == IDOK;
}


enum FileListCmd
{
	FLCMD_MARK_SELECTED = 1,
	FLCMD_UNMARK_SELECTED,
	FLCMD_MARK_ALL,
	FLCMD_UNMARK_ALL,
	FLCMD_MARK_INVERT,
	FLCMD_SHELL_COPY,
	FLCMD_SHELL_CUT,
	FLCMD_SHELL_DELETE,
	FLCMD_SHELL_PROP,
	FLCMD_COPY_NAME,
	FLCMD_EXPORT_LIST_ANSI,
	FLCMD_EXPORT_LIST_UNICODE,
	FLCMD_EXPORT_MARKED_ANSI,
	FLCMD_EXPORT_MARKED_UNICODE,
	FLCMD_REMOVE_FROM_LIST,
	FLCMD_SELECT_MARKED,
};


CFileList::CFileList(void)
{
	g_pFileList = this;

	m_hWnd = NULL;
	m_hList = NULL;
}


CFileList::~CFileList()
{
	//
}


HWND CFileList::GetWnd(void)
{
	return m_hWnd;
}


void CFileList::Clear(void)
{
	ListView_DeleteAllItems(m_hList);
}


void CFileList::SetNumItems(int n)
{
	ListView_SetItemCount(m_hList, n);
}


void CFileList::AddItem(const wchar_t* pszFileName, int iFile)
{
	LV_ITEM lvi;
	int i;

	i = ListView_GetItemCount(m_hList);

	// should strictly match, overwise we would get desynced
	if (i != iFile)
	{
		__DEBUG_BREAK;
	}

	ZeroMemory(&lvi, sizeof(LV_ITEM));
	lvi.mask = LVIF_TEXT;
	lvi.iItem = i;
	lvi.iSubItem = 0;
	lvi.pszText = (LPWSTR)pszFileName;
	lvi.cchTextMax = 0;

	ListView_InsertItem(m_hList, &lvi);
}


void CFileList::UpdateList(void)
{
	ListView_SetColumnWidth(m_hList, 0, -1);
}


void CFileList::SetSel(int i)
{
	// unselect all first
	ListView_SetItemState(m_hList, -1, 0, LVIS_SELECTED);

	// then select and scroll to it
	ListView_EnsureVisible(m_hList, i, FALSE);
	ListView_SetItemState(m_hList, i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	//ListView_SetItemState(m_hList, i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	//ListView_Update(m_hList, i);

	/*
	// XXX: a fix to make range select work properly
	//( it's still buggy... )
	-- too buggy...
	RECT rect;
	ListView_GetItemRect( m_hList, i, &rect, LVIR_LABEL );
	int x = rect.left + ( rect.right - rect.left ) / 2;
	int y = rect.top + ( rect.bottom - rect.top ) / 2;
	SendMessage( m_hList, WM_LBUTTONDOWN, 0, MAKELPARAM( x, y ) );
	*/
}


bool CFileList::IsMarked(int i)
{
	return ListView_GetCheckState(m_hList, i) != 0;
}


void CFileList::SetMarked(int i, bool bMarked)
{
	ListView_SetCheckState(m_hList, i, bMarked);
}


void CFileList::SetSelectionMarked(bool bMarked)
{
	int iItem;
	int nSelected;
	int i;

	//iItem = ListView_GetSelectionMark(m_hList) - 1;
	iItem = -1;
	nSelected = ListView_GetSelectedCount(m_hList);

	for (i = 0; i < nSelected; i++)
	{
		iItem = ListView_GetNextItem(m_hList, iItem, LVNI_SELECTED);

		SetMarked(iItem, bMarked);
	}
}


void CFileList::ToggleSelectionMarked(void)
{
	int iItem = ListView_GetNextItem(m_hList, -1, LVNI_SELECTED);

	if (iItem != -1)
	{
		bool bMarked = ListView_GetCheckState(m_hList, iItem) != 0;

		SetSelectionMarked(!bMarked);
	}
}


void CFileList::SetAllMarked(bool bMarked)
{
	SetMarked(-1, bMarked);
}


void CFileList::MarkInvert(void)
{
	int nItems = ListView_GetItemCount(m_hList);
	int i;

	for (i = 0; i < nItems; i++)
	{
		SetMarked(i, !IsMarked(i));
	}
}


void CFileList::SelectMarked(void)
{
	int nItems = ListView_GetItemCount(m_hList);
	int i;

	// clear the selection
	ListView_SetItemState(m_hList, -1, 0, LVIS_SELECTED);

	for (i = 0; i < nItems; i++)
	{
		if (IsMarked(i))
		{
			ListView_SetItemState(m_hList, i, LVIS_SELECTED, LVIS_SELECTED);
		}
	}
}


int CFileList::CountMarked( void )
{
	int nItems = ListView_GetItemCount(m_hList);
	int i;

	int nMarked = 0;

	// clear the selection
	ListView_SetItemState(m_hList, -1, 0, LVIS_SELECTED);

	for (i = 0; i < nItems; i++)
	{
		if (IsMarked(i))
		{
			nMarked++;
		}
	}

	return nMarked;
}



//
// shell
//


// purpose?
bool CFileList::CheckCanPerformFileOperation(void)
{
	return true;
}


LPITEMIDLIST CreateRelativePIDL(LPITEMIDLIST pidlAbs)
{
	LPITEMIDLIST pidlLast = NULL;
	BYTE* p = (BYTE*)pidlAbs;

	for (;;)
	{
		LPITEMIDLIST pidl = (LPITEMIDLIST)p;

		if (pidl->mkid.cb == 0)
		{
			break;
		}

		pidlLast = pidl;

		p += pidl->mkid.cb;
	}

	if (pidlLast == NULL)
	{
		return NULL;
	}

	int iSize = pidlLast->mkid.cb + (sizeof(short) * 2); // 2 mkid.cb
	LPITEMIDLIST pidl = (LPITEMIDLIST)CoTaskMemAlloc(iSize);
	memcpy((void*)pidl, pidlLast, iSize);

	return pidl;
}


void CFileList::InvokeShellCommand(char* pszCmd)
{
	int nItems = ListView_GetItemCount(m_hList);
	int nFiles = 0;
	int i;

	for (i = 0; i < nItems; i++)
	{
		if (IsMarked(i))
		{
			nFiles++;
		}
	}

	if (nFiles > 0)
	{
		LPITEMIDLIST* apidl = (LPITEMIDLIST*)malloc(sizeof(LPITEMIDLIST) * nFiles);

		if (apidl != NULL)
		{
			int iFile = 0;
			wchar_t szSrc[MAX_PATH];
			LPITEMIDLIST pidl;
			SFGAOF sfgao;
			HRESULT hr;

			for (i = 0; i < nFiles; i++)
			{
				apidl[i] = NULL;
			}

			for (i = 0; i < nItems; i++)
			{
				if (IsMarked(i))
				{
					wsprintf(szSrc, L"%s\\%s", g_pContext->GetSourcePath(), g_pContext->GetFileName(i));

					// XXX: NOTE: ensure there are no '/' slashes
					hr = SHParseDisplayName(szSrc, NULL, &pidl, 0, &sfgao);

					if (SUCCEEDED(hr))
					{
						apidl[iFile++] = CreateRelativePIDL(pidl);

						CoTaskMemFree(pidl);
					}
					else
					{
						MessageBox(m_hWnd, L"An error occurred while retrieving PIDL. Operation cancelled.", L"Shell Error", MB_ICONHAND | MB_OK);
						goto skip;
					}
				}
			}

			IShellFolder* pDesktop = NULL;

			hr = SHGetDesktopFolder(&pDesktop);

			if (pDesktop)
			{
				ULONG nEaten;
				LPITEMIDLIST pidlDsktop;

				wchar_t szSrcPath[MAX_PATH];
				wcscpy( szSrcPath, g_pContext->GetSourcePath() ); // better do this, because ParseDisplayName doesn't state that the pointer is const

				hr = pDesktop->ParseDisplayName(NULL, NULL, szSrcPath, &nEaten, &pidlDsktop, NULL);
				
				if (hr == S_OK)
				{
					IShellFolder* pFolder = NULL;

					pDesktop->BindToObject(pidlDsktop, NULL, IID_IShellFolder, (void**)&pFolder);
					
					if (pFolder)
					{
						IContextMenu* pMenu = NULL;

						pFolder->GetUIObjectOf(NULL, nFiles, (LPCITEMIDLIST*)apidl, IID_IContextMenu, NULL, (void**)&pMenu);
						
						if (pMenu)
						{
							HMENU hMenu = CreatePopupMenu();

							if (hMenu)
							{
								pMenu->QueryContextMenu(hMenu, 0, 1, 0x7FFF, CMF_DEFAULTONLY);
								
								POINT pt;
								GetCursorPos(&pt);

								CMINVOKECOMMANDINFOEX info = { 0 };
								info.cbSize = sizeof(info);
								info.fMask = CMIC_MASK_PTINVOKE;
								if (GetKeyState(VK_CONTROL) < 0)
								{
									info.fMask |= CMIC_MASK_CONTROL_DOWN;
								}
								if (GetKeyState(VK_SHIFT) < 0)
								{
									info.fMask |= CMIC_MASK_SHIFT_DOWN;
								}
								info.hwnd = m_hWnd;
								info.lpVerb = pszCmd;
								info.nShow = SW_SHOWNORMAL;
								info.ptInvoke = pt;
								pMenu->InvokeCommand((LPCMINVOKECOMMANDINFO)&info);

								DestroyMenu(hMenu);
							}

							pMenu->Release();
						}

						pFolder->Release();
					}

					CoTaskMemFree(pidlDsktop);
				}

				pDesktop->Release();
			}

skip:

			for (i = 0; i < nFiles; i++)
			{
				if (apidl[i])
				{
					CoTaskMemFree(apidl[i]);
				}
			}

			free(apidl);
		}
	}
}


void CFileList::ShellCopy(void)
{
	if (CheckCanPerformFileOperation())
	{
		InvokeShellCommand("copy");	
	}
}


void CFileList::ShellCut(void)
{
	if (CheckCanPerformFileOperation())
	{
		InvokeShellCommand("cut");	
	}
}


void CFileList::ShellDelete(void)
{
	if (CheckCanPerformFileOperation())
	{
		InvokeShellCommand("delete");	
	}
}


void CFileList::ShellProperties(void)
{
	if (CheckCanPerformFileOperation())
	{
		InvokeShellCommand("properties");	
	}
}


static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	// If the BFFM_INITIALIZED message is received
	// set the path to the start path.
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
		{
			if (NULL != lpData)
			{
				SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
			}
		}
	}

	return 0; // The function should always return 0.
}


bool BrowseForFolder(HWND hWnd, wchar_t* pszDir)
{
	static BROWSEINFO bi;
	LPITEMIDLIST pidl;

	bi.hwndOwner = hWnd;
	bi.lpszTitle = NULL;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)pszDir;

	pidl = SHBrowseForFolder(&bi);

	if (pidl != NULL)
	{
		SHGetPathFromIDList(pidl, pszDir);

		return true;
	}

	return false;
}


wchar_t g_szFileOpDestDir[MAX_PATH];


void CFileList::CopyName(void)
{
	int nSelected = ListView_GetSelectedCount(m_hList);
	if ( nSelected )
	{
		if (OpenClipboard(m_hWnd))
		{
			int iTotalLen = 0;
			int iItem = -1;
			int i;

			for (i = 0; ;)
			{
				iItem = ListView_GetNextItem(m_hList, iItem, LVNI_SELECTED);
				const wchar_t* pszFileName = g_pContext->GetFileName(iItem);

				iTotalLen += ( int )wcslen(pszFileName);

				i++;

				if (i < nSelected)
				{
					iTotalLen += 2;
					
					continue;
				}

				break;
			}

			int iSize = (iTotalLen + 1) * 2;
			HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, iSize);
			wchar_t* buffer = (wchar_t*)GlobalLock(hMem);
			int iPos = 0;
			iItem = -1;

			for (i = 0; ;)
			{
				iItem = ListView_GetNextItem(m_hList, iItem, LVNI_SELECTED);
				const wchar_t* pszFileName = g_pContext->GetFileName(iItem);
				int iLen = ( int )wcslen(pszFileName);

				memcpy(&buffer[iPos], pszFileName, iLen * 2);
				iPos += iLen;

				i++;

				if (i < nSelected)
				{
					buffer[iPos++] = '\r';
					buffer[iPos++] = '\n';
					
					continue;
				}

				break;
			}

			GlobalUnlock(hMem);

			EmptyClipboard();

			SetClipboardData(CF_UNICODETEXT, hMem);

			CloseClipboard();
		}
	}
}


void CFileList::ExportList(bool bMarked, bool bUnicode)
{
	if ( CExportFileList::Show( g_hWnd ) )
	{
		if ( *g_pExportFileList->m_pszFileName == 0 )
		{
			return;
		}

		int nItems = ListView_GetItemCount(m_hList);

		wchar_t szFileName[MAX_PATH];

		if ( g_pExportFileList->m_bUnicode )
		{
			FILE* output = _wfopen(g_pExportFileList->m_pszFileName, L"wb");
			if (output)
			{
				unsigned short bom = 0xFEFF;
				fwrite(&bom, 2, 1, output);

				short nl[2] = { '\r', '\n' };

				for (int i = 0; i < nItems; i++)
				{
					if (!g_pExportFileList->m_bMarkedOnly || IsMarked(i))
					{
						if ( g_pExportFileList->m_bIncludePath )
						{
							wsprintfW( szFileName, L"%s\\%s", g_pContext->GetSourcePath(), g_pContext->GetFileName(i) );
						}
						else
						{
							wcscpy( szFileName, g_pContext->GetFileName(i) );
						}
						int iLen = ( int )wcslen(szFileName);
						fwrite(szFileName, iLen * 2, 1, output);
						fwrite(nl, 4, 1, output);
					}
				}

				fclose(output);
			}
		}
		else
		{
			FILE* output = _wfopen(g_pExportFileList->m_pszFileName, L"wt");

			if (output)
			{
				for (int i = 0; i < nItems; i++)
				{
					if (!g_pExportFileList->m_bMarkedOnly || IsMarked(i))
					{
						if ( g_pExportFileList->m_bIncludePath )
						{
							wsprintfW( szFileName, L"%s\\%s", g_pContext->GetSourcePath(), g_pContext->GetFileName(i) );
						}
						else
						{
							wcscpy( szFileName, g_pContext->GetFileName(i) );
						}
						fprintf(output, "%S\n", szFileName);
					}
				}

				fclose(output);
			}
		}
	}
}


void CFileList::RemoveMarkedFromList(void)
{
	int nItems = ListView_GetItemCount(m_hList);
	int iCurrent = g_pContext->GetFile();
	int i;

	// start off from the end of the list, this way we avoid shifting the items everytime we delete one
	for (i = nItems-1; i >= 0; i--)
	{
		if (IsMarked(i))
		{
			ListView_DeleteItem(m_hList, i);
			g_pContext->RemoveFile(i);
			if (iCurrent > i)
			{
				iCurrent--;
			}
		}
	}

	g_pTextureViewer->SetFile(iCurrent);
}


WNDPROC g_pfnOldListWndProc;

LRESULT CALLBACK CFileList::ListWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// so, we're just interested in the keyboard messages, just redirecting them to the parent
	// this is useful to change the files without bringing the focus back to the viewport
	if ((uMsg == WM_KEYDOWN) || (uMsg == WM_KEYUP) || (uMsg == WM_CHAR) || (uMsg == 0x020A))
	{
		//HWND hParent = GetParent(hWnd);
		//CFileList* pWnd = (CFileList*)GetWindowLong(hParent, 0);

		return SendMessage(GetParent(hWnd), uMsg, wParam, lParam);
	}
	
	return CallWindowProc(g_pfnOldListWndProc, hWnd, uMsg, wParam, lParam);
}


int CFileList::Create(HWND hWnd, CREATESTRUCT* pcs)
{
	m_hWnd = hWnd;

	SendMessage(hWnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);

	LV_COLUMN lvc;
	//int iStyleEx;
	//int i, j;

	m_hList = CreateWindowEx(
		0,
		WC_LISTVIEW,
		NULL,
		WS_CHILD | WS_VISIBLE | LVS_REPORT/* | LVS_SINGLESEL*/ | LVS_SHOWSELALWAYS | 
			/*LVS_EDITLABELS | */LVS_ALIGNLEFT | LVS_NOCOLUMNHEADER | LVS_NOSORTHEADER,
		0,
		0,
		0,
		0,
		m_hWnd,
		(HMENU)IDC_FILE_LIST_LIST,
		g_hInst,
		NULL
		);

	int iStyleEx = /*LVS_EX_FULLROWSELECT | */LVS_EX_CHECKBOXES;
	SendMessage(m_hList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, (LPARAM)iStyleEx);

	ZeroMemory(&lvc, sizeof(LV_COLUMN));
	lvc.mask = LVCF_WIDTH;
	lvc.cx = 150;
	SendMessage(m_hList, LVM_INSERTCOLUMN, 0, (LPARAM)&lvc);

	// make a subclass of it
	g_pfnOldListWndProc = (WNDPROC)SetWindowLongPtr(m_hList, GWLP_WNDPROC, (LONG_PTR)ListWndProc);

	// menu button
	m_hMenuButton = CreateWindowEx(0, L"FLATBUTTON", L">", WS_CHILD | WS_VISIBLE, 0, 0, 1, 1, m_hWnd, (HMENU)IDC_FILE_LIST_MENUBTN, g_hInst, NULL);

	// menus

	//HMENU hShell = CreateMenu();
	wchar_t buffer[128];

	//HMENU hMarked = CreateMenu();
	//AppendMenu(hMarked, MF_STRING, FLCMD_SELECT_MARKED, L"Select");
	//AppendMenu(hMarked, MF_SEPARATOR, 0, NULL);
	//AppendMenu(hMarked, MF_STRING, FLCMD_SHELL_COPY, L"Copy");
	//AppendMenu(hMarked, MF_STRING, FLCMD_SHELL_CUT, L"Cut");
	//AppendMenu(hMarked, MF_STRING, FLCMD_SHELL_DELETE, L"Delete");
	//AppendMenu(hMarked, MF_SEPARATOR, 0, NULL);
	//AppendMenu(hMarked, MF_STRING, FLCMD_SHELL_PROP, L"Properties");
	//AppendMenu(hMarked, MF_SEPARATOR, 0, NULL);
	//AppendMenu(hMarked, MF_STRING, FLCMD_REMOVE_FROM_LIST, L"Remove from List");

	m_hMarkedGroupMenu = CreateMenu();
	//AppendMenu(m_hMarkedGroupMenu, MF_POPUP, (UINT_PTR)hMarked, L"Marked");
	//AppendMenu(m_hMarkedGroupMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(m_hMarkedGroupMenu, MF_STRING, FLCMD_MARK_ALL, L"Mark All");
	AppendMenu(m_hMarkedGroupMenu, MF_STRING, FLCMD_UNMARK_ALL, L"Unmark All");
	AppendMenu(m_hMarkedGroupMenu, MF_STRING, FLCMD_MARK_INVERT, L"Mark Invert");
	AppendMenu(m_hMarkedGroupMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(m_hMarkedGroupMenu, MF_STRING, FLCMD_SELECT_MARKED, L"Select");
	AppendMenu(m_hMarkedGroupMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(m_hMarkedGroupMenu, MF_STRING, FLCMD_SHELL_COPY, L"Copy");
	AppendMenu(m_hMarkedGroupMenu, MF_STRING, FLCMD_SHELL_CUT, L"Cut");
	AppendMenu(m_hMarkedGroupMenu, MF_STRING, FLCMD_SHELL_DELETE, L"Delete");
	AppendMenu(m_hMarkedGroupMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(m_hMarkedGroupMenu, MF_STRING, FLCMD_SHELL_PROP, L"Properties");
	AppendMenu(m_hMarkedGroupMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(m_hMarkedGroupMenu, MF_STRING, FLCMD_EXPORT_LIST_ANSI, L"Export List...");
	//AppendMenu(hMarked, MF_STRING, FLCMD_EXPORT_LIST_UNICODE, L"Export List (Unicode)...");
	AppendMenu(m_hMarkedGroupMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(m_hMarkedGroupMenu, MF_STRING, FLCMD_REMOVE_FROM_LIST, L"Remove from List");

	m_hItemMenu = CreateMenu();
	AppendMenu(m_hItemMenu, MF_STRING, FLCMD_MARK_SELECTED, ConstructMenuItem( L"Mark", CMD_MARK_FILE, buffer ));
	AppendMenu(m_hItemMenu, MF_STRING, FLCMD_UNMARK_SELECTED, ConstructMenuItem( L"Unmark", CMD_MARK_FILE, buffer ));
	AppendMenu(m_hItemMenu, MF_STRING, FLCMD_COPY_NAME, L"Copy Name");

	m_hPop = CreateMenu();
	AppendMenu(m_hPop, MF_POPUP, (UINT_PTR)m_hItemMenu, NULL);
	AppendMenu(m_hPop, MF_POPUP, (UINT_PTR)m_hMarkedGroupMenu, NULL);

	return 0;
}


void CFileList::Destroy(void)
{
	//
}

#define FL_HEADER_HEIGHT 21
void CFileList::Size(int xS, int yS, int flags)
{
	RECT rect;
	GetClientRect(m_hWnd, &rect);

	SetWindowPos(m_hList, NULL, 0, FL_HEADER_HEIGHT, rect.right, rect.bottom-FL_HEADER_HEIGHT, NULL);
	//ListView_SetColumnWidth( m_hList, 0, rect.right-rect.left-2 );

	SetWindowPos(m_hMenuButton, NULL, rect.right-21, 0, FL_HEADER_HEIGHT, FL_HEADER_HEIGHT, 0);
}


void CFileList::Command(int eCmd, int eNotifyCode)
{
	if (eNotifyCode == BN_CLICKED)
	{
		switch (eCmd)
		{
		case IDC_FILE_LIST_MENUBTN:
			RECT rect;
			GetWindowRect(m_hMenuButton, &rect);
			//EnableMenuItem(m_hMarkedGroupMenu, 0, MF_BYPOSITION	| ( CountMarked() ? MF_ENABLED : MF_GRAYED ) );
			TrackPopupMenu(m_hMarkedGroupMenu, 0, rect.right, rect.top, 0, m_hWnd, NULL);
			break;
		}
	}

	// XXX: notify code?
	switch (eCmd)
	{
	case FLCMD_MARK_SELECTED:
		SetSelectionMarked(true);
		break;
	case FLCMD_UNMARK_SELECTED:
		SetSelectionMarked(false);
		break;
	case FLCMD_MARK_ALL:
		SetAllMarked(true);
		break;
	case FLCMD_UNMARK_ALL:
		SetAllMarked(false);
		break;
	case FLCMD_MARK_INVERT:
		MarkInvert();
		break;
	case FLCMD_SELECT_MARKED:
		SelectMarked();
		break;
	case FLCMD_SHELL_COPY:
		ShellCopy();
		break;
	case FLCMD_SHELL_CUT:
		ShellCut();
		break;
	case FLCMD_SHELL_DELETE:
		ShellDelete();
		break;
	case FLCMD_SHELL_PROP:
		ShellProperties();
		break;
	case FLCMD_COPY_NAME:
		CopyName();
		break;
	case FLCMD_EXPORT_LIST_UNICODE:
		ExportList(false, true);
		break;
	case FLCMD_EXPORT_LIST_ANSI:
		ExportList(false, false);
		break;
	case FLCMD_EXPORT_MARKED_UNICODE:
		ExportList(true, true);
		break;
	case FLCMD_EXPORT_MARKED_ANSI:
		ExportList(true, false);
		break;
	case FLCMD_REMOVE_FROM_LIST:
		RemoveMarkedFromList();
		break;
	}
}


void CFileList::Notify(int iCtrl, NMHDR* pnmhdr)
{
	if (iCtrl == IDC_FILE_LIST_LIST)
	{
		NM_LISTVIEW* pnml = (NM_LISTVIEW*)pnmhdr;

		if ((pnmhdr->code == NM_CLICK) || (pnmhdr->code == NM_DBLCLK))
		{
			LV_HITTESTINFO lvhti;

			GetCursorPos(&lvhti.pt);
			ScreenToClient(m_hList, &lvhti.pt);

			SendMessage(m_hList, LVM_HITTEST, 0, (LPARAM)&lvhti);

			if (lvhti.flags & LVHT_ONITEMLABEL)
			{
				if ((pnmhdr->code == NM_DBLCLK) || (pnmhdr->code == NM_CLICK)) // so only on double clk
				{
					if (pnmhdr->code == NM_CLICK)
					{
						if ((GetKeyState(VK_SHIFT) < 0) || (GetKeyState(VK_CONTROL) < 0))
						{
							return;
						}
					}

					g_pTextureViewer->SetFile(pnml->iItem);
				}
			}
			else if (lvhti.flags & LVHT_ONITEMSTATEICON)
			{
				bool bChecked = ListView_GetCheckState(m_hList, lvhti.iItem) != 0;

				SetMarked(lvhti.iItem, bChecked);
			}
		}
		else if (pnmhdr->code == NM_RCLICK)
		{
			POINT pt;
			GetCursorPos(&pt);

			TrackPopupMenu(m_hItemMenu, 0, pt.x, pt.y, 0, m_hWnd, NULL);
		}
	}
}


void CFileList::Close(void)
{
	SendMessage(g_hWnd, WM_COMMAND, CMD_FILE_LIST, NULL);
}

/*
void CFileList::RButtonUp(int fKeys, int x, int y)
{
	POINT pt;
	GetCursorPos(&pt);

	TrackPopupMenu(m_hItemMenu, 0, pt.x, pt.y, 0, m_hWnd, NULL);
}
*/


LRESULT CALLBACK CFileList::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CFileList* pWnd = (CFileList*)GetWindowLong(hWnd, 0);
	LRESULT lResult = NULL;

	switch (uMsg)
	{

	case WM_SIZE:
		pWnd->Size((short)LOWORD(lParam), (short)HIWORD(lParam), (int)wParam);
		break;

	case WM_COMMAND:
		pWnd->Command(LOWORD(wParam), HIWORD(wParam));
		break;

	case WM_NOTIFY:
		pWnd->Notify((int)wParam, (NMHDR*)lParam);
		break;

	case WM_KEYDOWN:
	case WM_KEYUP:
	case 0x020A:
		lResult = SendMessage(g_hWnd, uMsg, wParam, lParam);
		break;

	case WM_CLOSE:
		pWnd->Close();
		break;

	case WM_CREATE:
		pWnd = new CFileList();
		SetWindowLong(hWnd, 0, (LONG)pWnd);
		lResult = pWnd->Create(hWnd, (CREATESTRUCT*)lParam);
		break;

	case WM_DESTROY:
		pWnd->Destroy();
		delete pWnd;
		break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return lResult;
}



void InitFileList(HWND hWndParent)
{
	const wchar_t* pszWindowClass = L"VCtrl_FileList";
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = CFileList::WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(void*);
	wc.hInstance = g_hInst;
	wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = pszWindowClass;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIconSm = 0;

	if ( !RegisterClassEx(&wc) )
	{
		__DEBUG_BREAK;
	}

	HWND hWnd = CreateWindowEx(
		WS_EX_TOOLWINDOW,
		pszWindowClass,
		L"Files",
		//WS_CAPTION | WS_THICKFRAME | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
		WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU | WS_THICKFRAME,
		g_iFileListLeft,
		g_iFileListTop,
		g_iFileListWidth,
		g_iFileListHeight,
		hWndParent,
		NULL,
		g_hInst,
		NULL
		);
}

