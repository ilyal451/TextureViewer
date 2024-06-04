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

#include <math.h>

#include "../shared/utils.h"
#include "../shared/rect.h"
#include "../shared/plibnative.h"

#include "renderer.h"


// the GDI renderer (currently the only option)


extern PLibNativeFuncs_t* g_plibnativefuncs;



//
// CBitmap
//


struct CBitmap
{
	HBITMAP h;
	BYTE* data; // a DIB section
	int iWidth;
	int iHeight;
	bool IsValid(void) { return (h != NULL); }
	void Alloc(HDC hDC, int w, int h);
	void Free(void);
};

void CBitmap::Alloc(HDC hDC, int nw, int nh)
{
	BITMAPINFO bmi;

	iWidth = nw;
	iHeight = nh;

	ZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = iWidth;
	bmi.bmiHeader.biHeight = -iHeight; // reverse order
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = iWidth * iHeight * 4;

	h = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (void**)&data, NULL, 0);
}


void CBitmap::Free(void)
{
	DeleteObject(h);
	h = NULL;
	data = NULL;
	iWidth = 0;
	iHeight = 0;
}




//
// CGDIRenderer
//


class CGDIRenderer: public CBaseRenderer
{

public:

	CGDIRenderer();
	~CGDIRenderer();

	bool Init(HWND hWnd);
	void SetPixelsCallbackProc(PIXELSCALLBACK pfn, void* param);
	void SetWindowSize(int x, int y);
	void Paint(void);
	void Update(void);
	void SetImage(int iWidth, int iHeight, int flags, Gamma_t* pcs, bool bValid);
	void ResetImage(void);
	void UpdatePixels(int flags);
	void SetImageRect(RECT* prect);
	void SetBackgroundColor(COLORREF clr);
	void SetChannelMode(int eMode);
	void SetAlphaColor(COLORREF clr);
	void SetAlphaOpacity(int iOpacity);
	void SetShowBorder(bool bShow);
	void SetBorderColor(COLORREF clr);
	void SetTileMode(int eMode);
	void SetNumTilesHorz(int nTiles);
	void SetNumTilesVert(int nTiles);
	void SetMinFilter(int eMode);
	void SetMagFilter(int eMode);

private:

	HWND m_hWnd;
	HDC m_hDC;

	HRGN m_hrgnA;
	HRGN m_hrgnB;
	HRGN m_hrgnUpdate;
	HRGN m_hrgnImage;

	PIXELSCALLBACK m_pfnReadPixels;
	void* m_pReadPixelsParam;

	bool m_bReallocImage;
	bool m_bRebuildImage;
	bool m_bInvalidateImage;
	bool m_bInvalidateWindow;

	HDC m_hBitmapDC;
	HDC m_hShrunkBitmapDC;
	HDC m_hBackBufferDC;
	HBITMAP m_hEmptyBitmap;

	CBitmap m_Bitmap;
	CBitmap m_ShrunkBitmap; // the version of the bitmap shrunk to the current zoom level
	CBitmap m_BackBuffer; // we copy to it

	struct Image_sl {
		int iWidth;
		int iHeight;
		int flags;
		bool bValid;
		bool IsValid(void) { return bValid; }
	} m_Image;

	RECT m_rectImage;
	RECT m_rectClamp;

	int m_iWindowWidth;
	int m_iWindowHeight;

	HBRUSH m_hBackgroundBrush;
	int m_eChannelMode;
	bool m_bOverlayAlpha;
	COLORREF m_clrAlpha;
	int m_iAlphaOpacity;
	bool m_bShowBorder;
	HBRUSH m_hBorderBrush;
	int m_eTileMode;
	int m_nTilesHorz;
	int m_nTilesVert;
    int m_eStretchModeMin;
    int m_eStretchModeMag;
	int m_iBlurControl;

	void StretchBitmap(HDC hDC, RECT* prect, RECT* prectBitmap);
	void ReallocImage(void);
    void RebuildImage(void);
	bool IsPrestretchRequired(void);
	void UpdateShrunkBitmap(bool bForceRepaint);
	void UpdateClamp(RECT* prect);
	int GetTileWidth(void);
	int GetTileHeight(void);
	void InvalidateImage(void);
	void InvalidateWindow(void);
	void GetFrameRect(RECT* prect);
};


CGDIRenderer::CGDIRenderer(void)
{
	m_hWnd = NULL;
	m_hDC = NULL;
	m_hrgnA = NULL;
	m_hrgnB = NULL;
	m_hrgnUpdate = NULL;
	m_hrgnImage = NULL;

	m_pfnReadPixels = NULL;

	m_bReallocImage = false;
	m_bRebuildImage = false;
	m_bInvalidateImage = false;
	m_bInvalidateWindow = false;

	m_hBitmapDC = NULL;
	m_hShrunkBitmapDC = NULL;
	m_hBackBufferDC = NULL;
	m_hEmptyBitmap = NULL;
	ZeroMemory(&m_ShrunkBitmap, sizeof(m_ShrunkBitmap));
	ZeroMemory(&m_Bitmap, sizeof(m_Bitmap));
	ZeroMemory(&m_BackBuffer, sizeof(m_BackBuffer));
	ZeroMemory(&m_Image, sizeof(m_Image));
	ZeroMemory(&m_rectImage, sizeof(RECT));
	ZeroMemory(&m_rectClamp, sizeof(RECT));

	m_iWindowWidth = 0;
	m_iWindowHeight = 0;

	m_hBackgroundBrush = NULL;
	m_eChannelMode = CM_RGB;
	m_bOverlayAlpha = true;
	m_clrAlpha = 0x00FF00FF;
	m_iAlphaOpacity = 255;
	m_bShowBorder = true;
	m_hBorderBrush = NULL;
	m_nTilesHorz = 1;
	m_nTilesVert = 1;
	m_eStretchModeMin = HALFTONE;
	m_eStretchModeMag = COLORONCOLOR;
	m_iBlurControl = 1;
}


CGDIRenderer::~CGDIRenderer(void)
{
	if (m_hBitmapDC != NULL)
	{
		DeleteDC(m_hBitmapDC);
	}

	if (m_hShrunkBitmapDC != NULL)
	{
		DeleteDC(m_hShrunkBitmapDC);
	}

	if (m_hBackBufferDC != NULL)
	{
		DeleteDC(m_hBackBufferDC);
	}

	if (m_hEmptyBitmap != NULL)
	{
		DeleteObject(m_hEmptyBitmap);
	}

	if (m_ShrunkBitmap.IsValid())
	{
		m_ShrunkBitmap.Free();
	}

	if (m_Bitmap.IsValid())
	{
		m_Bitmap.Free();
	}

	if (m_BackBuffer.IsValid())
	{
		m_BackBuffer.Free();
	}

	if (m_hrgnA != NULL)
	{
		DeleteObject(m_hrgnA);
	}

	if (m_hrgnB != NULL)
	{
		DeleteObject(m_hrgnB);
	}

	if (m_hrgnUpdate != NULL)
	{
		DeleteObject(m_hrgnUpdate);
	}

	if (m_hrgnImage != NULL)
	{
		DeleteObject(m_hrgnImage);
	}

	if (m_hBackgroundBrush != NULL)
	{
		DeleteObject(m_hBackgroundBrush);
	}

	if (m_hBorderBrush != NULL)
	{
		DeleteObject(m_hBorderBrush);
	}
}


bool CGDIRenderer::Init(HWND hWnd)
{
	m_hWnd = hWnd;
	m_hDC = GetDC(hWnd);

	m_hBitmapDC = CreateCompatibleDC(m_hDC);
	m_hShrunkBitmapDC = CreateCompatibleDC(m_hDC);
	m_hBackBufferDC = CreateCompatibleDC(m_hDC);
	m_hEmptyBitmap = CreateCompatibleBitmap(m_hDC, 0, 0);
	SelectObject(m_hBitmapDC, m_hEmptyBitmap);

	GetClientRect(hWnd, &m_rectImage);
	CopyRect(&m_rectClamp, &m_rectImage);
	m_iWindowWidth = RectWidth(&m_rectImage);
	m_iWindowHeight = RectHeight(&m_rectImage);

	m_hrgnA = CreateRectRgn(0, 0, 1, 1);
	m_hrgnB = CreateRectRgn(0, 0, 1, 1);
	m_hrgnUpdate = CreateRectRgn(0, 0, 1, 1);
	m_hrgnImage = CreateRectRgn(0, 0, 1, 1);

	m_hBackgroundBrush = CreateSolidBrush(0x00000000);
	m_hBorderBrush = CreateSolidBrush(0x00FFFFFF);

	return true;
}


void CGDIRenderer::SetPixelsCallbackProc(PIXELSCALLBACK pfn, void* param)
{
	m_pfnReadPixels = pfn;
	m_pReadPixelsParam = param;
}


void CGDIRenderer::SetWindowSize(int w, int h)
{
	m_iWindowWidth = w;
	m_iWindowHeight = h;

	if (m_BackBuffer.IsValid())
	{
		SelectObject(m_hBackBufferDC, m_hEmptyBitmap);
		m_BackBuffer.Free();
	}

	RECT rect;

	GetClientRect(m_hWnd, &rect);

	m_BackBuffer.Alloc(m_hBackBufferDC, rect.right, rect.bottom);
	SelectObject(m_hBackBufferDC, m_BackBuffer.h);
}


void CGDIRenderer::Paint(void)
{
	HDC hDC;
	PAINTSTRUCT ps;
	POINT ptSize;
	POINT ptOrigin;
	RECT rectFrame;
	RECT rectView;
	float xStart;
	float xEnd;
	float yStart;
	float yEnd;
	float xStep;
	float yStep;
	int x;
	int y;

	GetUpdateRgn(m_hWnd, m_hrgnUpdate, FALSE); // must preceede to the call to BeginPaint
	
	hDC = BeginPaint(m_hWnd, &ps);

	SetBkMode(m_hBackBufferDC, OPAQUE);
	
	RectToPos(&m_rectImage, &ptSize, &ptOrigin);
	GetFrameRect(&rectFrame);
	GetClientRect(m_hWnd, &rectView);

	CBitmap* pSrc;

	if (m_ShrunkBitmap.IsValid())
	{
		pSrc = &m_ShrunkBitmap;
	}
	else if (m_Bitmap.IsValid())
	{
		pSrc = &m_Bitmap;
	}
	else
	{
		pSrc = NULL;
	}

	if (pSrc != NULL)
	{
		SelectObject(m_hBitmapDC, pSrc->h);

		RECT rectBitmap;
		SetRect(&rectBitmap, 0, 0, pSrc->iWidth, pSrc->iHeight);

		// background
		if (m_hBackgroundBrush != NULL)
		{
			SetRectRgn(m_hrgnImage, rectFrame.left, rectFrame.top, rectFrame.right, rectFrame.bottom);
			//  CombineRgn(m_hrgnB, m_hrgnUpdate, m_hrgnImage, RGN_DIFF);
			CombineRgn(m_hrgnA, m_hrgnImage, m_hrgnUpdate, RGN_OR);
			CombineRgn(m_hrgnB, m_hrgnImage, m_hrgnA, RGN_XOR); // exclude the image
			
			FillRgn(m_hBackBufferDC, m_hrgnB, m_hBackgroundBrush);
		}

		// Draw the border
		if (m_bShowBorder)
		{
			if (m_hBorderBrush != NULL)
			{
				SetRectRgn(m_hrgnA, m_rectImage.left, m_rectImage.top, m_rectImage.right, m_rectImage.bottom);
				SetRectRgn(m_hrgnB, rectFrame.left, rectFrame.top, rectFrame.right, rectFrame.bottom);
				CombineRgn(m_hrgnImage, m_hrgnA, m_hrgnB, RGN_XOR);

				FillRgn(m_hBackBufferDC, m_hrgnImage, m_hBorderBrush);
			}
		}

		xStep = (float)ptSize.x / m_nTilesHorz;
		yStep = (float)ptSize.y / m_nTilesVert;

		bool bCopyPixels;

		// shrink of stretch?
		if (((int)xStep <= m_Image.iWidth) || ((int)yStep <= m_Image.iHeight))
		{
			bCopyPixels = true; // no stretch (already have)
		}
		else
		{
			bCopyPixels = false;
		}

		yStart = (float)m_rectImage.top;

		for (y = 0; y < m_nTilesVert; y++)
		{
			xStart = (float)m_rectImage.left;
			yEnd = yStart + yStep;
			
			// TODO: clip to rcpaint better
			if ((yStart < rectView.bottom) && (yEnd > rectView.top))
			{
				for (x = 0; x < m_nTilesHorz; x++)
				{
					xEnd = xStart + xStep;

					if ((xStart < rectView.right) && (xEnd > rectView.left))
					{
						rect_t rectDst;
						rectDst.left = (int)xStart;
						rectDst.top = (int)yStart;
						rectDst.right = (int)xEnd;
						rectDst.bottom = (int)yEnd;
						rect_t rectSrc;
						rectSrc.left = 0;
						rectSrc.top = 0;
						rectSrc.right = pSrc->iWidth;
						rectSrc.bottom = pSrc->iHeight;

						if (bCopyPixels)
						{
							g_plibnativefuncs->pfnCopyPixels((uint_t*)m_BackBuffer.data, rectView.right, rectView.bottom, &rectDst, (uint_t*)pSrc->data, pSrc->iWidth, pSrc->iHeight, &rectSrc);
						}
						else
						{
							g_plibnativefuncs->pfnStretchPixelsPoint((uint_t*)m_BackBuffer.data, rectView.right, rectView.bottom, &rectDst, (uint_t*)pSrc->data, pSrc->iWidth, pSrc->iHeight, &rectSrc);
						}
					}

					xStart = xEnd;
				}
			}

			yStart = yEnd;
		}

		SelectObject(m_hBitmapDC, m_hEmptyBitmap); // fix
	}
	else
	{
		wchar_t szErrorStr[] = L"Error loading image.";
		
		// just fill the background
		FillRgn(m_hBackBufferDC, m_hrgnUpdate, m_hBackgroundBrush);

		SetBkMode(m_hBackBufferDC, TRANSPARENT);
		DrawText(m_hBackBufferDC,   szErrorStr, 
						(int)wcslen(szErrorStr), 
						&rectView, 
						DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}

	// present
	BitBlt(hDC, 0, 0, rectView.right, rectView.bottom, m_hBackBufferDC, 0, 0, SRCCOPY);

	EndPaint(m_hWnd, &ps);
}


void CGDIRenderer::ReallocImage(void)
{
	if (!m_Bitmap.IsValid() ||
		( !m_Image.IsValid() ) || // a special case if no image present (just clear the bitmap)
		(m_Image.iWidth != m_Bitmap.iWidth) || (m_Image.iHeight != m_Bitmap.iHeight))
	{
		if (m_Bitmap.IsValid())
		{
			m_Bitmap.Free();
			// XXX:
			if (m_ShrunkBitmap.IsValid())
			{
				m_ShrunkBitmap.Free();
			}
		}

		if ( m_Image.IsValid() )
		{
			m_Bitmap.Alloc(m_hDC, m_Image.iWidth, m_Image.iHeight);
		}
	}
}


bool CGDIRenderer::IsPrestretchRequired(void)
{
	//if (m_bSmartScale)
	if ((GetTileWidth() < m_Bitmap.iWidth) || (GetTileHeight() < m_Bitmap.iHeight))
	{
		return true;
	}

	return false;
}


void CGDIRenderer::UpdateShrunkBitmap(bool bForceRepaint)
{
	int iWidth;
	int iHeight;

	iWidth = GetTileWidth();
	iHeight = GetTileHeight();

	bool bRepaint = (bForceRepaint)? true: ((m_ShrunkBitmap.iWidth != iWidth) || (m_ShrunkBitmap.iHeight != iHeight));
	bool bPrestretchRequired = IsPrestretchRequired();

	if (!m_ShrunkBitmap.IsValid() ||
		!m_Bitmap.IsValid() ||
		(m_ShrunkBitmap.iWidth != iWidth) || (m_ShrunkBitmap.iHeight != iHeight) || 
		!bPrestretchRequired)
	{
		if (m_ShrunkBitmap.IsValid())
		{
			m_ShrunkBitmap.Free();
		}

		if (m_Bitmap.IsValid())
		{
			if (bPrestretchRequired)
			{
				m_ShrunkBitmap.Alloc(m_hDC, iWidth, iHeight);
			}
		}
	}

	if (m_ShrunkBitmap.IsValid() && bRepaint)
	{
		if (true)
		{
			if (m_eStretchModeMin == SM_LINEAR)
			{
				g_plibnativefuncs->pfnShrinkPixelsLinear(
					m_ShrunkBitmap.data,
					m_ShrunkBitmap.iWidth,
					m_ShrunkBitmap.iHeight,
					m_Bitmap.data,
					m_Bitmap.iWidth,
					m_Bitmap.iHeight,
					SHARPEN_NONE,
					m_iBlurControl);
			}
			else if (m_eStretchModeMin == SM_LINEAR_SHARPEN)
			{
				g_plibnativefuncs->pfnShrinkPixelsLinear(
					m_ShrunkBitmap.data,
					m_ShrunkBitmap.iWidth,
					m_ShrunkBitmap.iHeight,
					m_Bitmap.data,
					m_Bitmap.iWidth,
					m_Bitmap.iHeight,
					SHARPEN_X4,
					m_iBlurControl);
			}
			else if (m_eStretchModeMin == SM_LINEAR_SHARPEN_X8)
			{
				g_plibnativefuncs->pfnShrinkPixelsLinear(
					m_ShrunkBitmap.data,
					m_ShrunkBitmap.iWidth,
					m_ShrunkBitmap.iHeight,
					m_Bitmap.data,
					m_Bitmap.iWidth,
					m_Bitmap.iHeight,
					SHARPEN_X8A,
					m_iBlurControl);
			}
			else
			{
				g_plibnativefuncs->pfnShrinkPixelsPoint(
					m_ShrunkBitmap.data,
					m_ShrunkBitmap.iWidth,
					m_ShrunkBitmap.iHeight,
					m_Bitmap.data,
					m_Bitmap.iWidth,
					m_Bitmap.iHeight
					);

			}
		}
	}
}


void CGDIRenderer::RebuildImage(void)
{
	if (m_Bitmap.IsValid())
	{
		if (m_pfnReadPixels != NULL)
		{
			int nPixels = m_Bitmap.iWidth * m_Bitmap.iHeight;

			// obtain the pixels
			m_pfnReadPixels(m_Bitmap.data, m_pReadPixelsParam);

			// apply the channel mode
			if (m_eChannelMode == CM_RGB)
			{
				// do nothing
			}
			else if (m_eChannelMode == CM_RGBA) 
			{
				g_plibnativefuncs->pfnOverlayAlpha(m_Bitmap.data, nPixels, m_clrAlpha, m_iAlphaOpacity);
			}
			else if (m_eChannelMode == CM_RED)
			{
				g_plibnativefuncs->pfnExtractRed(m_Bitmap.data, nPixels);
			}
			else if (m_eChannelMode == CM_GREEN)
			{
				g_plibnativefuncs->pfnExtractGreen(m_Bitmap.data, nPixels);
			}
			else if (m_eChannelMode == CM_BLUE)
			{
				g_plibnativefuncs->pfnExtractBlue(m_Bitmap.data, nPixels);
			}
			else if (m_eChannelMode == CM_ALPHA)
			{
				g_plibnativefuncs->pfnExtractAlpha(m_Bitmap.data, nPixels);
			}
		}
	}

	UpdateShrunkBitmap(true);
}


void CGDIRenderer::Update(void)
{
	if (m_bReallocImage)
	{
		ReallocImage();
		m_bReallocImage = false;
		m_bRebuildImage = true;
	}

	if (m_bRebuildImage)
	{
		RebuildImage();
		m_bRebuildImage = false;
		m_bInvalidateImage = true;
	}

	if (m_bInvalidateWindow)
	{
		InvalidateWindow();
		m_bInvalidateWindow = false;
		m_bInvalidateImage = false;
	}
	
	if (m_bInvalidateImage)
	{
		InvalidateImage();
		m_bInvalidateImage = false;
	}
}


void CGDIRenderer::SetImage(int iWidth, int iHeight, int flags, Gamma_t* pcs, bool bValid)
{
	//if (!bValid || !m_Image.bValid)
	//{
	//	m_bInvalidateWindow = true;
	//}
	// XXX: there is no better way...
	m_bInvalidateWindow = true;

	if ( bValid )
	{
		m_Image.iWidth = iWidth;
		m_Image.iHeight = iHeight;
		m_Image.flags = flags;
		m_Image.bValid = true;
	}
	else
	{
		m_Image.iWidth = 0;
		m_Image.iHeight = 0;
		m_Image.flags = 0;
		m_Image.bValid = false;
	}

	m_bReallocImage = true;
}


void CGDIRenderer::ResetImage(void)
{
	SetImage(0,0,0,NULL,false);

	// XXX: force update?
	// Update();
	// or just free the bits?
	ReallocImage();
}


void CGDIRenderer::UpdatePixels(int flags)
{
	m_bRebuildImage = true;
}


void CGDIRenderer::SetImageRect(RECT* prect)
{
	RECT rectWindow;

	if (!EqualRect(prect, &m_rectImage))
	{
		if (EqualRectSize(prect, &m_rectImage))
		{
			GetClientRect(m_hWnd, &rectWindow);

			ScrollWindowEx(
				m_hWnd, 
				prect->left - m_rectImage.left,
				prect->top - m_rectImage.top,
				&rectWindow, 
				NULL, 
				NULL, //m_hrgnUpdate, 
				NULL, 
				SW_INVALIDATE
				);

			CopyRect(&m_rectImage, prect);
		}
		else
		{
			InvalidateImage();
			CopyRect(&m_rectImage, prect);
			InvalidateImage();

			UpdateShrunkBitmap(false);
		}
	}
}



void CGDIRenderer::SetBackgroundColor(COLORREF clr)
{
	if (m_hBackgroundBrush != NULL)
	{
		DeleteObject(m_hBackgroundBrush);
	}

	m_hBackgroundBrush = CreateSolidBrush(clr);
	m_bInvalidateWindow = true;
}


void CGDIRenderer::SetChannelMode(int eMode)
{
	m_eChannelMode = eMode;
	m_bRebuildImage = true;
}


void CGDIRenderer::SetAlphaColor(COLORREF clr)
{
	COLORREF clrBGR = ((clr & 0x00FF0000) >> 16) | (clr & 0x0000FF00) | ((clr & 0x000000FF) << 16); // RGB -> BGR
	
	m_clrAlpha = clrBGR;
	m_bRebuildImage = true;
}


void CGDIRenderer::SetAlphaOpacity(int iOpacity)
{
	m_iAlphaOpacity = iOpacity;
	m_bRebuildImage = true;
}


void CGDIRenderer::SetShowBorder(bool bShow)
{
	m_bShowBorder = bShow;
	m_bInvalidateWindow = true;
}


void CGDIRenderer::SetBorderColor(COLORREF clr)
{
	if (m_hBorderBrush != NULL)
	{
		DeleteObject(m_hBorderBrush);
	}

	m_hBorderBrush = CreateSolidBrush(clr);
	m_bInvalidateImage = true;
}


void CGDIRenderer::SetTileMode(int eMode)
{
	m_eTileMode = eMode;
	m_bInvalidateImage = true;
}


void CGDIRenderer::SetNumTilesHorz(int n)
{
	m_nTilesHorz = n;
	m_bInvalidateImage = true;
}


void CGDIRenderer::SetNumTilesVert(int n)
{
	m_nTilesVert = n;
	m_bInvalidateImage = true;
}


void CGDIRenderer::SetMinFilter(int eMode)
{
	m_eStretchModeMin = eMode;
	m_bRebuildImage = true;
	m_bInvalidateImage = true;
}


void CGDIRenderer::SetMagFilter(int eMode)
{
	m_eStretchModeMag = eMode;
	m_bRebuildImage = true;
	m_bInvalidateImage = true;
}


inline int CGDIRenderer::GetTileWidth(void)
{
	return RectWidth(&m_rectImage) / m_nTilesHorz;
}


inline int CGDIRenderer::GetTileHeight(void)
{
	return RectHeight(&m_rectImage) / m_nTilesVert;
}


void CGDIRenderer::GetFrameRect(RECT* prect)
{
	CopyRect(prect, &m_rectImage);
	
	if (m_bShowBorder)
	{
		prect->left -= 1;
		prect->top -= 1;
		prect->right += 1;
		prect->bottom += 1;
	}
}


void CGDIRenderer::InvalidateImage(void)
{
	RECT rect;

	GetFrameRect(&rect);
	InvalidateRect(m_hWnd, &rect, FALSE);
}


void CGDIRenderer::InvalidateWindow(void)
{
	InvalidateRect(m_hWnd, NULL, FALSE);
}


CBaseRenderer* CreateGDIRenderer( void )
{
	return new CGDIRenderer();
}



