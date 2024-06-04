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

#include <stdio.h>
#include <math.h>

#include "../shared/utils.h"
#include "../shared/rect.h"

#include "viewer.h"

#include "viewport.h"


extern HINSTANCE g_hInst;


CViewport::CViewport(void)
{
	m_hWnd = NULL;
	m_hWndParent = NULL;
	m_pDevice = NULL;

	m_bImageValid = false;

	m_nTilesHorz = 1;
	m_nTilesVert = 1;

	ZeroMemory(&m_rectImage, sizeof(RECT));
	ZeroMemory(&m_rectViewport, sizeof(RECT));
    m_bScrolling = false;

	m_bColorPicking = false;

	m_bClipImage = true;
	m_iClipImageExtent = 0;
}


CViewport::~CViewport()
{

}


CBaseRenderer* CreateGDIRenderer( void );


int CViewport::WM_Create(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_hWnd = hWnd;
	m_hWndParent = ((CREATESTRUCT*)lParam)->hwndParent;

	/*
	switch (g_eDevice)
	{
	case DEVICE_GDI:
		m_pDevice = new CGDIRenderer();
		break;
	}
	*/
	m_pDevice = CreateGDIRenderer();

	if (m_pDevice != NULL)
	{
		if (m_pDevice->Init(hWnd))
		{
			RECT rect;

			GetClientRect(hWnd, &rect);
			CopyRect(&m_rectImage, &rect);
			CopyRect(&m_rectViewport, &rect);

			return 0;
		}
	}

	delete this; // ???

	return 1;
}


void CViewport::WM_Destroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (m_pDevice != NULL)
	{
		delete m_pDevice;
	}
}


void CViewport::SetPixelsCallbackProc(PIXELSCALLBACK pfn, void* param)
{
	m_pDevice->SetPixelsCallbackProc(pfn, param);
}


void CViewport::SetBackgroundColor(COLORREF clr)
{
	m_pDevice->SetBackgroundColor(clr);
}


void CViewport::SetChannelMode(int eMode)
{
	m_pDevice->SetChannelMode(eMode);
}


void CViewport::SetAlphaColor(COLORREF clr)
{
	m_pDevice->SetAlphaColor(clr);
}


void CViewport::SetAlphaOpacity(int iOpacity)
{
	m_pDevice->SetAlphaOpacity(iOpacity);
}


void CViewport::SetShowBorder(bool bShow)
{
	m_pDevice->SetShowBorder(bShow);
}


void CViewport::SetBorderColor(COLORREF clr)
{
	m_pDevice->SetBorderColor(clr);
}


void CViewport::SetTileMode(int eMode)
{
	m_pDevice->SetTileMode(eMode);
}


void CViewport::SetNumTilesHorz(int nTiles)
{
	m_nTilesHorz = nTiles;
	m_pDevice->SetNumTilesHorz(nTiles);
}


void CViewport::SetNumTilesVert(int nTiles)
{
	m_nTilesVert = nTiles;
	m_pDevice->SetNumTilesVert(nTiles);
}


void CViewport::SetMinFilter(int eFilter)
{
	m_pDevice->SetMinFilter(eFilter);
}


void CViewport::SetMagFilter(int eFilter)
{
	m_pDevice->SetMagFilter(eFilter);
}


void CViewport::SetClipImage(bool bClip)
{
	m_bClipImage = bClip;

	if (m_bClipImage)
	{
		SetImagePos(NULL, NULL); // just update
	}
}


void CViewport::SetClipImageExtent(int iExtent)
{
	m_iClipImageExtent = iExtent;

	if (m_bClipImage)
	{
		SetImagePos(NULL, NULL); // just update
	}
}


void CViewport::SetImage(int iWidth, int iHeight, int flags, Gamma_t* pcs, bool bValid)
{
	m_bImageValid = bValid;
	m_iImageWidth = iWidth;
	m_iImageHeight = iHeight;

	m_pDevice->SetImage(iWidth, iHeight, flags, pcs, bValid);
}

// XXX: why do we have two functions for the same purpose?
void CViewport::ResetImage(void)
{
	m_bImageValid = false;
	m_iImageWidth = 0;
	m_iImageHeight = 0;

	m_pDevice->ResetImage();
}


void CViewport::UpdatePixels(int flags)
{
	m_pDevice->UpdatePixels(flags);
}


void CViewport::Update(void)
{
	m_pDevice->Update();
}


void CViewport::SetViewportRect(RECT* prect)
{
    RECT rectImage;
    int iOldWidth;
    int iOldHeight;
    int iWidth;
    int iHeight;
    int x;
    int y;

    iOldWidth = RectWidth(&m_rectViewport);
    iOldHeight = RectHeight(&m_rectViewport);
    iWidth = RectWidth(prect);
    iHeight = RectHeight(prect);

    // todo: check
    CopyRect(&m_rectViewport, prect);

    GetImageRect(&rectImage);
    x = (iWidth / 2) - (iOldWidth / 2);
    y = (iHeight / 2) - (iOldHeight / 2);
    OffsetRect(&rectImage, x, y);

    SetImageRect(&rectImage);
}


void CViewport::GetViewportRect(RECT* prect)
{
	CopyRect(prect, &m_rectViewport);
}


void CViewport::GetViewportPos(POINT* pptSize, POINT* pptOrigin)
{
    RECT rectViewport;

    GetViewportRect(&rectViewport);
    RectToPos(&rectViewport, pptSize, pptOrigin);
}


//#define MAX_SHORT 0x7FFF
//#define MIN_SHORT (-MAX_SHORT)

void CViewport::SetImageRect(RECT* prectImage)
{
    RECT rectViewport;
    int iWidth;
    int iHeight;

    if (m_bClipImage)
	{
		iWidth = RectWidth(prectImage);
		iHeight = RectHeight(prectImage);

		GetViewportRect(&rectViewport);

        rectViewport.left += m_iClipImageExtent;
        rectViewport.right -= m_iClipImageExtent;
        rectViewport.top += m_iClipImageExtent;
        rectViewport.bottom -= m_iClipImageExtent;

        if (iWidth <= RectWidth(&rectViewport))
		{
            prectImage->left = (rectViewport.left + (RectWidth(&rectViewport) / 2)) - (iWidth / 2);
            prectImage->right = prectImage->left + iWidth;
        }
		else
		{
            if (prectImage->left > rectViewport.left)
			{
                prectImage->left = rectViewport.left;
                prectImage->right = prectImage->left + iWidth;
            }
			else if (prectImage->right < rectViewport.right)
			{
                prectImage->right = rectViewport.right;
                prectImage->left = prectImage->right - iWidth;
            }
        }

        if (iHeight <= RectHeight(&rectViewport))
		{
            prectImage->top = (rectViewport.top + (RectHeight(&rectViewport) / 2)) - (iHeight / 2);
            prectImage->bottom = prectImage->top + iHeight;
        }
		else
		{
            if (prectImage->top > rectViewport.top)
			{
                prectImage->top = rectViewport.top;
                prectImage->bottom = prectImage->top + iHeight;
            }
			else if (prectImage->bottom < rectViewport.bottom)
			{
                prectImage->bottom = rectViewport.bottom;
                prectImage->top = prectImage->bottom - iHeight;
            }
        }
    }

    CopyRect(&m_rectImage, prectImage);

	m_pDevice->SetImageRect(&m_rectImage);

	UpdateScroll();

}


void CViewport::GetImageRect(RECT* prect)
{
    CopyRect(prect, &m_rectImage);
}


void CViewport::SetImagePos(POINT* pptSize, POINT* pptOrigin)
{
    RECT rectImage;
    POINT ptSize;
    POINT ptOrigin;

    GetImagePos(&ptSize, &ptOrigin);

    if (pptSize != NULL)
	{
        ptSize.x = pptSize->x;
        ptSize.y = pptSize->y;
    }

    if (pptOrigin != NULL)
	{
        ptOrigin.x = pptOrigin->x;
        ptOrigin.y = pptOrigin->y;
    }

    rectImage.left = ptOrigin.x - (ptSize.x / 2);
    rectImage.top = ptOrigin.y - (ptSize.y / 2);
    rectImage.right = rectImage.left + ptSize.x;
    rectImage.bottom = rectImage.top + ptSize.y;

    SetImageRect(&rectImage);
}


void CViewport::GetImagePos(POINT* pptSize, POINT* pptOrigin)
{
    RECT rectImage;

    GetImageRect(&rectImage);
    RectToPos(&rectImage, pptSize, pptOrigin);
}


void CViewport::ResetOrigin(void)
{
	POINT pt;

	GetViewportPos(NULL, &pt);
	SetImagePos(NULL, &pt);
}


bool CViewport::IsWithinImageBorders(POINT* ppt)
{
	// XXX
	return ((ppt->x >= m_rectImage.left) && (ppt->x < m_rectImage.right) && (ppt->y >= m_rectImage.top) && (ppt->y < m_rectImage.bottom));
}


void CViewport::Zoom(POINT* pptSize)
{
    POINT ptOldSize;
    POINT ptOldOrigin;
	POINT ptNewSize;
	POINT ptNewOrigin;
	POINT ptZoomOrigin;

    GetImagePos(&ptOldSize, &ptOldOrigin);
    ptNewSize.x = pptSize->x;
    ptNewSize.y = pptSize->y;

	GetViewportPos(NULL, &ptZoomOrigin);

    if ((ptZoomOrigin.x != ptOldOrigin.x) && (ptOldSize.x != ptNewSize.x))
	{
        ptNewOrigin.x = (int)((float)(ptOldOrigin.x - ptZoomOrigin.x) * ((float)ptNewSize.x / (float)ptOldSize.x)) + ptZoomOrigin.x;
    }
	else
	{
		ptNewOrigin.x = ptOldOrigin.x;
	}

    if ((ptZoomOrigin.y != ptOldOrigin.y) && (ptOldSize.y != ptNewSize.y))
	{
        ptNewOrigin.y = (int)((float)(ptOldOrigin.y - ptZoomOrigin.y) * ((float)ptNewSize.y / (float)ptOldSize.y)) + ptZoomOrigin.y;
    }
	else
	{
		ptNewOrigin.y = ptOldOrigin.y;
	}

	if (m_bScrolling)
	{
		// modify the scroll origin
		m_ptScrollOrigin.x -= (ptNewOrigin.x - ptOldOrigin.x);
		m_ptScrollOrigin.y -= (ptNewOrigin.y - ptOldOrigin.y);
	}

	SetImagePos(&ptNewSize, &ptNewOrigin);
}


void CViewport::WM_Size(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int x;
	int y;
	RECT rect;
	
	x = (short)LOWORD(lParam);
	y = (short)HIWORD(lParam);

	m_pDevice->SetWindowSize(x, y);

	rect.left = 0;
	rect.right = x;
	rect.top = 0;
	rect.bottom = y;
	
	SetViewportRect(&rect);
}


void CViewport::UpdateScroll(void)
{
	SCROLLINFO si;
	int iViewportWidth;
	int iViewportHeight;
	int iImageWidth;
	int iImageHeight;

	iViewportWidth = RectWidth(&m_rectViewport);
	iViewportHeight = RectHeight(&m_rectViewport);
	iImageWidth = RectWidth(&m_rectImage);
	iImageHeight = RectHeight(&m_rectImage);

	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = iImageWidth - 1;
	si.nPage = iViewportWidth;
	si.nPos = -m_rectImage.left;
	si.nTrackPos = 0;
	SetScrollInfo(m_hWnd, SB_HORZ, &si, TRUE);

	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = iImageHeight - 1;
	si.nPage = iViewportHeight;
	si.nPos = -m_rectImage.top;
	si.nTrackPos = 0;
	SetScrollInfo(m_hWnd, SB_VERT, &si, TRUE);
}


void CViewport::WM_HScroll(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int iPos;
	
	switch(LOWORD(wParam))
	{
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		iPos = HIWORD(wParam);
		break;
	case SB_LINEUP:
		iPos = GetScrollPos(hWnd, SB_HORZ) - 10;
		break;
	case SB_LINEDOWN:
		iPos = GetScrollPos(hWnd, SB_HORZ) + 10;
		break;
	case SB_PAGEUP:
		iPos = GetScrollPos(hWnd, SB_HORZ) - RectWidth(&m_rectViewport);
		break;
	case SB_PAGEDOWN:
		iPos = GetScrollPos(hWnd, SB_HORZ) + RectWidth(&m_rectViewport);
		break;
	default:
		return;
	}

	RECT rect;
	GetImageRect(&rect);
	int iWidth = RectWidth(&rect);
	rect.left = -iPos;
	rect.right = rect.left + iWidth;
	SetImageRect(&rect);
}


void CViewport::WM_VScroll(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int iPos;
	
	switch(LOWORD(wParam))
	{
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		iPos = HIWORD(wParam);
		break;
	case SB_LINEUP:
		iPos = GetScrollPos(hWnd, SB_VERT) - 10;
		break;
	case SB_LINEDOWN:
		iPos = GetScrollPos(hWnd, SB_VERT) + 10;
		break;
	case SB_PAGEUP:
		iPos = GetScrollPos(hWnd, SB_VERT) - RectHeight(&m_rectViewport);
		break;
	case SB_PAGEDOWN:
		iPos = GetScrollPos(hWnd, SB_VERT) + RectHeight(&m_rectViewport);
		break;
	default:
		return;
	}

	RECT rect;
	GetImageRect(&rect);
	int iHeight = RectHeight(&rect);
	rect.top = -iPos;
	rect.bottom = rect.top + iHeight;
	SetImageRect(&rect);
}


bool CViewport::GetPixelAt( POINT* pptPoint, POINT* pptPixel )
{
	if (m_bImageValid)
	{
		//if (IsWithinImageBorders(pptPoint))
		{
			RECT rect;
			GetImageRect(&rect);

			int x = (int)floor(((double)(m_iImageWidth * m_nTilesHorz) / RectWidth(&rect)) * (pptPoint->x - rect.left));
			int y = (int)floor(((double)(m_iImageHeight * m_nTilesVert) / RectHeight(&rect)) * (pptPoint->y - rect.top));

			if ( m_nTilesHorz > 1 )
			{
				pptPixel->x = x % m_iImageWidth;
			}
			else
			{
				pptPixel->x = x;
			}

			if ( m_nTilesVert > 1 )
			{
				pptPixel->y = y % m_iImageHeight;
			}
			else
			{
				pptPixel->y = y;
			}

			return IsWithinImageBorders(pptPoint);
		}
	}

	return false;
}


void CViewport::WM_MouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    int x, y;

	x = (short)LOWORD(lParam);
	y = (short)HIWORD(lParam);

    if (m_bScrolling)
	{
		POINT ptOrigin;
		
		GetImagePos(NULL, &ptOrigin);
		ptOrigin.x += x - (m_ptScrollOrigin.x + ptOrigin.x);
		ptOrigin.y += y - (m_ptScrollOrigin.y + ptOrigin.y);
		SetImagePos(NULL, &ptOrigin);

		GetImagePos(NULL, &ptOrigin); // need to reevaluate, because it can be clipped
        m_ptScrollOrigin.x = x - ptOrigin.x;
        m_ptScrollOrigin.y = y - ptOrigin.y;
    }
	else if ( m_bColorPicking )
	{
		//
	}

	// added to support x,y coords display
	//if (m_bImageValid)
	{
		POINT pt = { x, y };
		POINT ptPixel;
		
		if (GetPixelAt(&pt, &ptPixel))
		{
			g_pTextureViewer->SetCoords(true, ptPixel.x, ptPixel.y);
		}
		else
		{
			g_pTextureViewer->SetCoords(false, 0, 0);
		}
	}
}


void CViewport::BeginScroll(void)
{
    RECT rect;

	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(m_hWnd, &pt);
	int x = pt.x;
	int y = pt.y;

    if (/*m_bReady*/ true)
	{
		GetViewportRect(&rect);

        if ((x >= rect.left) && (x < rect.right) && 
            (y >= rect.top) && (y < rect.bottom))
		{
			//GetImageRect(&rect);
            //m_ptScrollOrigin.x = x - rect.left;
            //m_ptScrollOrigin.y = y - rect.top;
			POINT ptOrigin;
			GetImagePos(NULL, &ptOrigin);
            m_ptScrollOrigin.x = x - ptOrigin.x;
            m_ptScrollOrigin.y = y - ptOrigin.y;

            SetCapture(m_hWnd);
            m_bScrolling = true;
        }
    }
}


void CViewport::EndScroll(void)
{
    if (m_bScrolling)
	{
        ReleaseCapture();
        m_bScrolling = false;
    }
}


void CViewport::SetColorPicking( bool b )
{
	if ( m_bColorPicking != b )
	{
		m_bColorPicking = b;
		HCURSOR hc2;
		if ( m_bColorPicking )
		{
			SetCapture( m_hWnd );
			HCURSOR hc = LoadCursor( NULL, IDC_CROSS );
			hc2 = SetCursor( hc );
		}
		else
		{
			SetCursor( LoadCursor( g_hInst, IDC_ARROW ) );
			ReleaseCapture();
		}
	}
}


void CViewport::WM_LButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(m_hWnd, &pt);
	int x = pt.x;
	int y = pt.y;

	if ( m_bColorPicking )
	{
		GetCursorPos(&m_ptPickStart);
		ScreenToClient(m_hWnd, &m_ptPickStart);
		return;
	}

	if (GetKeyState(VK_SHIFT) < 0)
	{
		g_pTextureViewer->DragDrop();
	}
	else
	{
		BeginScroll();
	}
}


void CViewport::WM_LButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if ( m_bColorPicking )
	{
		POINT ptPickEnd;
		GetCursorPos(&ptPickEnd);
		ScreenToClient(m_hWnd, &ptPickEnd);
		// no rect currently, but make sure it didn't move
		if ( m_ptPickStart.x == ptPickEnd.x && m_ptPickStart.y == ptPickEnd.y )
		{
			POINT ptPixel;
			if ( GetPixelAt( &ptPickEnd, &ptPixel ) )
			{
				Rect_t rect = { ptPixel.x, ptPixel.y, ptPixel.x + 1, ptPixel.y + 1 };
				g_pTextureViewer->PickColor( true, &rect );
			}
			else
			{
				g_pTextureViewer->PickColor( false, NULL );
			}
		}
	}
	else
	{
		if (m_bScrolling)
		{
			EndScroll();
		}
	}
}


void CViewport::WM_MButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{

}


void CViewport::WM_MButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{

}


void CViewport::WM_RButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if ( m_bColorPicking )
	{
		//
	}
	else
	{
		SendMessage(m_hWndParent, WM_RBUTTONDOWN, wParam, lParam);
	}
}


void CViewport::WM_RButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if ( m_bColorPicking )
	{
		// quit the mode
		g_pTextureViewer->FinishColorPicking();
	}
	else
	{
		SendMessage(m_hWndParent, WM_RBUTTONUP, wParam, lParam);
	}
}


void CViewport::WM_Paint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_pDevice->Paint();
}


LRESULT CALLBACK ViewportWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CViewport* pViewport = (CViewport*)GetWindowLong(hWnd, 0);
    LRESULT lResult = NULL;

    switch (uMsg)
	{
    case WM_MOUSEMOVE:
        pViewport->WM_MouseMove(hWnd, wParam, lParam);
        break;

	case WM_PAINT:
        pViewport->WM_Paint(hWnd, wParam, lParam);
        break;

    case WM_SIZE:
        pViewport->WM_Size(hWnd, wParam, lParam);
        break;
	
	case WM_HSCROLL:
		pViewport->WM_HScroll(hWnd, wParam, lParam);
		break;

	case WM_VSCROLL:
		pViewport->WM_VScroll(hWnd, wParam, lParam);
		break;

    case WM_LBUTTONDOWN:
        pViewport->WM_LButtonDown(hWnd, wParam, lParam);
        break;

    case WM_LBUTTONUP:
        pViewport->WM_LButtonUp(hWnd, wParam, lParam);
        break;

    case WM_MBUTTONDOWN:
        pViewport->WM_MButtonDown(hWnd, wParam, lParam);
        break;

    case WM_MBUTTONUP:
        pViewport->WM_MButtonUp(hWnd, wParam, lParam);
        break;

    case WM_RBUTTONDOWN:
        pViewport->WM_RButtonDown(hWnd, wParam, lParam);
        break;

    case WM_RBUTTONUP:
        pViewport->WM_RButtonUp(hWnd, wParam, lParam);
        break;

    case WM_CREATE:
		pViewport = new CViewport();
		SetWindowLong(hWnd, 0, (LONG)pViewport);
		lResult = pViewport->WM_Create(hWnd, wParam, lParam);
        break;

    case WM_DESTROY:
		pViewport->WM_Destroy(hWnd, wParam, lParam);
		delete pViewport;
        break;

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return lResult;
}

