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

#pragma once

#include "renderer.h"

class CViewport
{
public:

	CViewport();
	~CViewport();

    int WM_Create(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_Destroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_Size(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_Paint(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_HScroll(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_VScroll(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_MouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_LButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_LButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_MButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_MButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_RButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_RButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);

	void SetPixelsCallbackProc(PIXELSCALLBACK pfn, void* param);

	void SetBackgroundColor(COLORREF clr);
	void SetChannelMode(int eMode);
	void SetAlphaColor(COLORREF clr);
	void SetAlphaOpacity(int iOpacity);
	void SetShowBorder(bool bShow);
	void SetBorderColor(COLORREF clr);
	void SetTileMode(int eMode);
	void SetNumTilesHorz(int nTiles);
	void SetNumTilesVert(int nTiles);
	void SetMinFilter(int eFilter);
	void SetMagFilter(int eFilter);
	void SetClipImage(bool bClip);
	void SetClipImageExtent(int iExtent);

	void SetImage(int iWidth, int iHeight, int flags, Gamma_t* pcs, bool bValid);
	void ResetImage(void);
	void UpdatePixels(int flags);

	void Update(void);
	//void ResizeImageRect(POINT* pptSize);
	void Zoom(POINT* pptSizeNew);
	void ResetOrigin(void);
    void BeginScroll(void);
    void EndScroll(void);

	bool GetPixelAt( POINT* pptPoint, POINT* pptPixel );
	void SetPickingCropCoords( bool b );
	void SetColorPicking( bool b );
	void BeginColorPicking( void );
	void EndColorPicking( void );

	void GetViewportRect(RECT* prect);
    void GetViewportPos(POINT* pptSize, POINT* pptOrigin);
    void GetImageRect(RECT* prect);
    void GetImagePos(POINT* pptSize, POINT* pptOrigin);

private:

	HWND m_hWnd;
	HWND m_hWndParent;
	CBaseRenderer* m_pDevice;

	bool m_bImageValid;
	int m_iImageWidth;
	int m_iImageHeight;
	int m_nTilesHorz;
	int m_nTilesVert;

	RECT m_rectImage;
	RECT m_rectViewport;

	bool m_bScrolling;
	POINT m_ptScrollOrigin;

	bool m_bColorPicking;
	POINT m_ptPickStart;

	bool m_bClipImage;
	int m_iClipImageExtent;

	void SetViewportRect(RECT* prect);
    void SetImageRect(RECT* prect);
	void SetImagePos(POINT* pptSize, POINT* pptOrigin);

	void UpdateScroll(void);

    bool IsWithinImageBorders(POINT* ppt);

};

LRESULT CALLBACK ViewportWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

