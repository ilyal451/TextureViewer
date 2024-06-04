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

#define UNICODE
#include <windows.h>
#include "controls.h"
#include "../shared/utils.h"


//
// Color Box
//

class CColorBox
{
public:
	CColorBox(void);
	~CColorBox();
	void WM_Create(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_Destroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_Paint(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_LButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void SetColor(COLORREF clr);
	COLORREF GetColor(void);
	void PickColor(void);
private:
	HWND m_hWnd;
	HBRUSH m_hColor;
	CHOOSECOLOR m_cc;
	COLORREF m_aclrCustom[16];
};


CColorBox::CColorBox(void)
{
	m_hColor = NULL;
	ZeroMemory(&m_cc, sizeof(CHOOSECOLOR));
	ZeroMemory(m_aclrCustom, 16 * sizeof(COLORREF));
}


CColorBox::~CColorBox()
{
	//
}


void CColorBox::WM_Create(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_hWnd = hWnd;
	SetColor(0x00FFFFFF);
	m_cc.lStructSize = sizeof(CHOOSECOLOR);
	m_cc.hwndOwner = hWnd;
	m_cc.lpCustColors = m_aclrCustom;
	m_cc.Flags = CC_FULLOPEN | CC_RGBINIT;
}


void CColorBox::WM_Destroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (m_hColor != NULL)
	{
		DeleteObject(m_hColor);
	}
}


void CColorBox::WM_Paint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rect;

	hDC = BeginPaint(m_hWnd, &ps);

	GetClientRect(m_hWnd, &rect);
	rect.left += 2;
	rect.right -= 2;
	rect.top += 2;
	rect.bottom -= 2;
	FillRect(hDC, &rect, m_hColor);

	EndPaint(m_hWnd, &ps);
}


void CColorBox::WM_LButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	PickColor();
}


void CColorBox::SetColor(COLORREF clr)
{
	m_cc.rgbResult = clr;
	if (m_hColor != NULL)
	{
		DeleteObject(m_hColor);
	}
	m_hColor = CreateSolidBrush(m_cc.rgbResult);

	InvalidateRect(m_hWnd, NULL, 0);
}


COLORREF CColorBox::GetColor(void)
{
	return m_cc.rgbResult;
}


void CColorBox::PickColor(void)
{
	if (ChooseColor(&m_cc))
	{
		SetColor(m_cc.rgbResult);
	}
}


LRESULT CALLBACK ColorBoxWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CColorBox* pcb = (CColorBox*)GetWindowLong(hWnd, 0);

	switch(uMsg)
	{
	case WM_PAINT:
		pcb->WM_Paint(hWnd, wParam, lParam);
		break;
	case WM_LBUTTONUP:
		pcb->WM_LButtonUp(hWnd, wParam, lParam);
		break;
	case WM_CREATE:
		pcb = new CColorBox();
		SetWindowLong(hWnd, 0, (LONG)pcb);
		pcb->WM_Create(hWnd, wParam, lParam);
		break;
	case WM_DESTROY:
		pcb->WM_Destroy(hWnd, wParam, lParam);
		delete pcb;
		break;
	case CB_SETCOLOR:
		pcb->SetColor((COLORREF)wParam);
		break;
	case CB_GETCOLOR:
		return (LRESULT)pcb->GetColor();
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}


//
// Flat Button
//


class CFlatButton
{
public:
	CFlatButton(void);
	~CFlatButton();
	void WM_Create(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_Destroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_Paint(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_LButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void SetText(wchar_t* psz);
private:
	HWND m_hWndParent;
	int m_iId;
	HWND m_hWnd;
	wchar_t* m_pszText;
};


CFlatButton::CFlatButton(void)
{
	m_pszText = NULL;
}


CFlatButton::~CFlatButton()
{
	//
}


void CFlatButton::WM_Create(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	CREATESTRUCT* pcs = (CREATESTRUCT*)lParam;

	m_hWndParent = pcs->hwndParent;
	m_iId = (int)pcs->hMenu;
	m_hWnd = hWnd;
	m_pszText = (pcs->lpszName != NULL)? AllocStringW(pcs->lpszName): NULL;
}


void CFlatButton::WM_Destroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (m_pszText != NULL)
	{
		FreeString(m_pszText);
	}
}


void CFlatButton::WM_Paint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rect;

	hDC = BeginPaint(m_hWnd, &ps);

	if (m_pszText != NULL)
	{
		SetBkMode(hDC, TRANSPARENT);
		GetClientRect(m_hWnd, &rect);
		DrawText(hDC, m_pszText, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	}

	EndPaint(m_hWnd, &ps);
}


void CFlatButton::WM_LButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	SendMessage(m_hWndParent, WM_COMMAND, MAKEWPARAM(m_iId, BN_CLICKED), (LPARAM)m_hWnd);
}


void CFlatButton::SetText(wchar_t* psz)
{
	if (m_pszText != NULL)
	{
		FreeString(m_pszText);
		m_pszText = NULL;
	}

	if (psz != NULL)
	{
		m_pszText = AllocStringW(psz);
	}
}


LRESULT CALLBACK FlatButtonWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CFlatButton* pcb = (CFlatButton*)GetWindowLong(hWnd, 0);

	switch(uMsg)
	{
	case WM_PAINT:
		pcb->WM_Paint(hWnd, wParam, lParam);
		break;
	case WM_LBUTTONUP:
		pcb->WM_LButtonUp(hWnd, wParam, lParam);
		break;
	case WM_CREATE:
		pcb = new CFlatButton();
		SetWindowLong(hWnd, 0, (LONG)pcb);
		pcb->WM_Create(hWnd, wParam, lParam);
		break;
	case WM_DESTROY:
		pcb->WM_Destroy(hWnd, wParam, lParam);
		delete pcb;
		break;
	case FBM_SETTEXT:
		pcb->SetText((wchar_t*)lParam);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}


//
// Init
//

bool InitControls(HINSTANCE hInst)
{
	WNDCLASSEX wc;

	memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_GLOBALCLASS;
	wc.lpfnWndProc = ColorBoxWndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = 4;
	wc.hInstance = hInst;
	wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = COLORBOXCLASS;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIconSm = 0;

	if (!RegisterClassEx(&wc))
	{
		goto quit;
	}

	memset(&wc, 0, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_GLOBALCLASS;
	wc.lpfnWndProc = FlatButtonWndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = 4;
	wc.hInstance = hInst;
	wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = FLATBUTTONCLASS;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIconSm = 0;

	if (!RegisterClassEx(&wc))
	{
		goto quit;
	}

	return true;

quit:

	return false;
}

