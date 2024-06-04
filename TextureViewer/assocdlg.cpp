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

#include <stdio.h>

#include "../shared/utils.h"

#include "format.h"

#include "resource.h"


// XXX: doesn't work on Win10

// XXX: do we really need to backup?
// for example, some progs do not restore previous associations when des-associating

wchar_t g_szSoftwareClasses[] = L"Software\\Classes";
wchar_t g_szAssocClassName[] = L"TexView.File";
wchar_t g_szAssocProgName[] = L"Texture Viewer";
wchar_t g_szAssocContentType[] = L"image/xyz";
wchar_t g_szAssocPerceivedType[] = L"image";
wchar_t g_szAssocOldValueName[] = L"TexViewBackup";

HWND g_hAssocDlg;


bool IsAssociated(const wchar_t* pszExt)
{
	wchar_t szKey[256];
	HKEY hKey;
	DWORD disp;
	DWORD iSize;
	bool b;

	b = false;

	wsprintf(szKey, L"%s\\.%s", g_szSoftwareClasses, pszExt);

	if (RegCreateKeyEx(HKEY_CURRENT_USER, szKey, NULL, NULL, NULL, KEY_READ, NULL, &hKey, &disp) == ERROR_SUCCESS)
	{
        iSize = sizeof(wbuffer);

        if (RegQueryValueEx(hKey, NULL, NULL, NULL, (BYTE*)wbuffer, &iSize) == ERROR_SUCCESS)
        {
            //buffer[iSize/2] = '\0';

			if (FWStrEq(wbuffer, g_szAssocClassName))
			{
				b = true;
			}
        }

		RegCloseKey(hKey);
	}

	return b;
}


void SetAssoc(wchar_t* pszExt)
{
	wchar_t szKey[MAX_PATH];
	HKEY hKey;
	DWORD disp;
	DWORD iSize;

	if (!IsAssociated(pszExt))
	{
		wsprintf(szKey, L"%s\\.%s", g_szSoftwareClasses, pszExt);

		if (RegCreateKeyEx(HKEY_CURRENT_USER, szKey, NULL, NULL, NULL, KEY_READ + KEY_WRITE, NULL, &hKey, &disp) == ERROR_SUCCESS)
		{
			//
			// do backup
			//

			iSize = sizeof(wbuffer);

			if (RegQueryValueEx(hKey, NULL, NULL, NULL, (BYTE*)wbuffer, &iSize) == ERROR_SUCCESS)
			{
				//buffer[iSize/2] = '\0';
				
				if (RegQueryValueEx(hKey, g_szAssocOldValueName, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
				{
					RegSetValueEx(hKey, g_szAssocOldValueName, NULL, REG_SZ, (BYTE*)wbuffer, iSize);
				}
			}

			//
			// link to our class
			//

			iSize = ( int )wcslen(g_szAssocClassName);
			RegSetValueEx(hKey, NULL, NULL, REG_SZ, (BYTE*)g_szAssocClassName, (iSize + 1) * sizeof(TCHAR));

			RegCloseKey(hKey);

			//SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
		}
	}
}


void RemoveAssoc(wchar_t* pszExt)
{
	wchar_t szKey[MAX_PATH];
	HKEY hKey;
	DWORD disp;
	DWORD iSize;

	if (IsAssociated(pszExt))
	{
		wsprintf(szKey, L"%s\\.%s", g_szSoftwareClasses, pszExt);

		if (RegCreateKeyEx(HKEY_CURRENT_USER, szKey, NULL, NULL, NULL, KEY_READ + KEY_WRITE, NULL, &hKey, &disp) == ERROR_SUCCESS)
		{
			iSize = sizeof(wbuffer);

			if (RegQueryValueEx(hKey, g_szAssocOldValueName, NULL, NULL, (BYTE*)wbuffer, &iSize) == ERROR_SUCCESS)
			{
				RegSetValueEx(hKey, NULL, NULL, REG_SZ, (BYTE*)wbuffer, iSize);
				RegDeleteValue(hKey, g_szAssocOldValueName);
			}
			else
			{
				RegDeleteValue(hKey, NULL);
			}

			RegCloseKey(hKey);

			//SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
		}
	}
}


void UpdateOpenWith( void )
{
	wchar_t szKey[MAX_PATH];
	HKEY hKey;
	DWORD disp;
	DWORD iSize;

	for (int i = 0; i < GetNumFormats(); i++)
	{
		int nExtensions = GetFormat(i)->GetNumExtensions();

		for (int j = 0; j < nExtensions; j++)
		{
			const wchar_t* pszExt = GetFormat(i)->GetExt(j);
			
			wsprintf(szKey, L"%s\\.%s", g_szSoftwareClasses, pszExt);

			if (RegCreateKeyEx(HKEY_CURRENT_USER, szKey, NULL, NULL, NULL, KEY_READ + KEY_WRITE, NULL, &hKey, &disp) == ERROR_SUCCESS)
			{
				HKEY hOpenWithProgIds;
				if (RegCreateKeyEx(hKey, L"OpenWithProgIds", NULL, NULL, NULL, KEY_READ + KEY_WRITE, NULL, &hOpenWithProgIds, &disp) == ERROR_SUCCESS)
				{
					iSize = ( int )wcslen(g_szAssocClassName);
					if (RegSetValueEx(hOpenWithProgIds, g_szAssocClassName, NULL, REG_NONE, NULL, 0) == ERROR_SUCCESS)
					{

					}

					RegCloseKey(hOpenWithProgIds);
				}

				RegCloseKey(hKey);
			}
		}
	}
}


void RegisterAssocClass(void)
{
	wchar_t szKey[MAX_PATH];
	HKEY hKey;
	HKEY hSubKey;
	DWORD disp;
	DWORD iSize;
	wchar_t szCmdLine[MAX_PATH+8];

	wsprintf(szKey, L"%s\\%s", g_szSoftwareClasses, g_szAssocClassName);

	if (RegCreateKeyEx(HKEY_CURRENT_USER, szKey, NULL, NULL, NULL, KEY_READ + KEY_WRITE, NULL, &hKey, &disp) == ERROR_SUCCESS)
	{
		iSize = ( int )wcslen(g_szAssocProgName);
		RegSetValueEx(hKey, NULL, NULL, REG_SZ, (BYTE*)g_szAssocProgName, (iSize + 1) * sizeof(TCHAR));
		iSize = ( int )wcslen(g_szAssocContentType);
		RegSetValueEx(hKey, L"ContentType", NULL, REG_SZ, (BYTE*)g_szAssocContentType, (iSize + 1) * sizeof(TCHAR));
		iSize = ( int )wcslen(g_szAssocPerceivedType);
		RegSetValueEx(hKey, L"PerceivedType", NULL, REG_SZ, (BYTE*)g_szAssocPerceivedType, (iSize + 1) * sizeof(TCHAR));

		if (RegCreateKeyEx(hKey, L"Shell\\Open\\Command", NULL, NULL, NULL, KEY_READ + KEY_WRITE, NULL, &hSubKey, &disp) == ERROR_SUCCESS)
		{
			iSize = wsprintf(szCmdLine, L"\"%s\" \"%%1\"", GetAppExePath());
			RegSetValueEx(hSubKey, NULL, NULL, REG_SZ, (BYTE*)szCmdLine, (iSize + 1) * sizeof(TCHAR));

			RegCloseKey(hSubKey);
		}

		RegCloseKey(hKey);
	}
}





/*
class CAssoc
{
public:
	void Init(void);
	void RegisterAssocClass(void);
	void Add(char* pszExt);
	void Remove(char* pszExt);
	bool IsAssociated(char* pszExt);
private:

	HWND m_hWnd;
};
*/

BOOL CALLBACK UpdateAssocEnumChildProc(HWND hWnd, LPARAM lParam)
{
	wchar_t szExt[MAX_PATH];
	bool bAssoc;

	if (GetWindowLong(hWnd, GWL_USERDATA) == 1)
	{
		GetWindowText(hWnd, szExt, MAX_PATH);

		if ((szExt[0] == '*') && (szExt[1] == '.'))
		{
			bAssoc = (SendMessage(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED);
			
			if (bAssoc)
			{
				SetAssoc(&szExt[2]);
			}
			else
			{
				RemoveAssoc(&szExt[2]);
			}
		}
	}

	return TRUE;
}


void UpdateAssoc(HWND hWnd)
{
	EnumChildWindows(hWnd, UpdateAssocEnumChildProc, NULL);
}


BOOL CALLBACK AssocDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
    case WM_INITDIALOG:
        // 
        break;

	case 0x020A: // WM_MOUSEWHEEL
		SendDlgItemMessage(hWnd, IDC_EXTPANEL, 0x020A, wParam, lParam);
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				UpdateAssoc(hWnd);
			case IDCANCEL:
				SendMessage(hWnd, WM_CLOSE, NULL, NULL);
				break;
			}
		}
		break;

    case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
        break;

    default:
        return 0;
    }

    return 1;
}


#define EXTPANEL_ITEM_HEIGHT 20

void ExtPanelVScroll(HWND hWnd, WPARAM wParam)
{
	int iOldPos;
	int iOffset;
	int iPos;
	int iMin;
	int iMax;

	iOldPos = GetScrollPos(hWnd, SB_VERT);
	iOffset = 0;

	switch(LOWORD(wParam))
	{
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		iOffset = iOldPos - HIWORD(wParam);
		break;
	case SB_LINEUP:
	case SB_PAGEUP:
		iOffset = 1;
		break;
	case SB_LINEDOWN:
	case SB_PAGEDOWN:
		iOffset = -1;
		break;
	}

	if (iOffset != 0)
	{
		iPos = iOldPos - iOffset;
		GetScrollRange(hWnd, SB_VERT, &iMin, &iMax);
		if (iPos < iMin)
		{
			iPos = iMin;
			iOffset = iPos - iOldPos;
		}
		else if (iPos > iMax)
		{
			iPos = iMax;
			iOffset = iOldPos - iPos;
		}
		
		ScrollWindow(hWnd, 0, iOffset * EXTPANEL_ITEM_HEIGHT, NULL, NULL);
		SetScrollPos(hWnd, SB_VERT, iOldPos - iOffset, TRUE);
	}
}


void ExtPanelMWheel(HWND hWnd, WPARAM wParam)
{
	int iDelta;
	int eScrollCode;
	int n;
	int i;

	iDelta = (short)HIWORD(wParam) / 120;
	
	if (iDelta < 0)
	{
		eScrollCode = SB_PAGEDOWN;
	}
	else
	{
		eScrollCode = SB_PAGEUP;
	}
	
	n = abs(iDelta);
	
	for (i = 0; i < n; i++)
	{
		SendMessage(hWnd, WM_VSCROLL, MAKELONG(eScrollCode, 0), NULL);
	}
}


LRESULT CALLBACK ExtPanelWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_VSCROLL:
		ExtPanelVScroll(hWnd, wParam);
		break;
	case 0x020A: // WM_MOUSEWHEEL
		ExtPanelMWheel(hWnd, wParam);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}


void CreateAssocDlg(void)
{
	WNDCLASSEX wc;
	HWND hExtPanel;
	int iTotalExt;
	int i, j;

	// the window itself is created from a dialog resource
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = ExtPanelWndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = 0;
    wc.hInstance = g_hInst;
    wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = L"ExtPanelWndClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIconSm = 0;

    if (RegisterClassEx(&wc))
	{
		g_hAssocDlg = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_ASSOC), g_hWnd, AssocDialogProc);
		
		if (g_hAssocDlg != NULL)
		{
			hExtPanel = GetDlgItem(g_hAssocDlg, IDC_EXTPANEL);
			
			iTotalExt = 0;

			for (i = 0; i < GetNumFormats(); i++)
			{
				int nExtensions = GetFormat(i)->GetNumExtensions();

				for (j = 0; j < nExtensions; j++)
				{
					const wchar_t* pszExt;
					HWND hCB;

					pszExt = GetFormat(i)->GetExt(j);
					
					wsprintf(wbuffer, L"*.%s", pszExt);
					hCB = CreateWindow(L"BUTTON", wbuffer, BS_AUTOCHECKBOX | WS_VISIBLE | WS_CHILD, 10, 5 + (iTotalExt * EXTPANEL_ITEM_HEIGHT), 100, 15, hExtPanel, NULL, g_hInst, NULL);
					SetWindowLong(hCB, GWL_USERDATA, 1);
					
					if (IsAssociated(pszExt))
					{
						SendMessage(hCB, BM_SETCHECK, (WPARAM)BST_CHECKED, NULL);
					}
					
					iTotalExt++;
				}
			}

			RECT rect;
			int iScrollRange;

			GetClientRect(hExtPanel, &rect);
			iScrollRange = max(0, ((iTotalExt + 1) * EXTPANEL_ITEM_HEIGHT) - rect.bottom) / EXTPANEL_ITEM_HEIGHT;

			SetScrollRange(hExtPanel, SB_VERT, 0, iScrollRange, TRUE);
		}
	}
}



void InitAssocDlg(void)
{
	RegisterAssocClass();

	UpdateOpenWith();

	CreateAssocDlg();
}

