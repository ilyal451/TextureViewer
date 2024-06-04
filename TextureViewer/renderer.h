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

typedef void (__cdecl *PIXELSCALLBACK)(void* buffer, void* param);

// renderer base class
// all renderers derive from this
// only the GDI renderer is currently implemented

enum DeviceTypes
{
	DEVICE_GDI = 0,
	DEVICE_OPENGL,
	NUM_DEVICES
};

enum ChannelModes
{
	CM_RGB = 0,
	CM_RGBA,
	CM_RED,
	CM_GREEN,
	CM_BLUE,
	CM_ALPHA,
	NUM_CHANNEL_MODES
};

enum TileModes
{
	TM_SINGLE_TILE,
	TM_TILE_HORZVERT,
	TM_TILE_HORZ,
	TM_TILE_VERT,
};

enum StretchModes
{
	SM_POINT = 0,
	SM_LINEAR,
	SM_LINEAR_SHARPEN,
	SM_LINEAR_SHARPEN_X8,
};

class CBaseRenderer
{
public:

	//CBaseRenderer();
	virtual ~CBaseRenderer() { };

	virtual bool Init(HWND hWnd) = 0;
	virtual void SetPixelsCallbackProc(PIXELSCALLBACK pfn, void* param) = 0;
	virtual void SetWindowSize(int x, int y) = 0;
	virtual void Paint(void) = 0;
	virtual void Update(void) = 0;
	virtual void SetImage(int iWidth, int iHeight, int flags, Gamma_t* pcs, bool bValid) = 0;
	virtual void ResetImage(void) = 0;
	virtual void UpdatePixels(int flags) = 0;
	virtual void SetImageRect(RECT* prect) = 0;
	virtual void SetBackgroundColor(COLORREF clr) = 0;
	virtual void SetChannelMode(int eMode) = 0;
	virtual void SetAlphaColor(COLORREF clr) = 0;
	virtual void SetAlphaOpacity(int iOpacity) = 0;
	virtual void SetShowBorder(bool bShow) = 0;
	virtual void SetBorderColor(COLORREF clr) = 0;
	virtual void SetTileMode(int eMode) = 0;
	virtual void SetNumTilesHorz(int nTiles) = 0;
	virtual void SetNumTilesVert(int nTiles) = 0;
	virtual void SetMinFilter(int eMode) = 0; // 0 - point; 1 - bilinear
	virtual void SetMagFilter(int eMode) = 0;
	//virtual void SetShowGidelines(bool bShow) = 0;
	//virtual void SetGuidelinePos(int eGuideline, int iPos) = 0;
	//virtual void SetShowOriginalRect( bool bShow ) = 0;
	//virtual void SetOriginalRect( RECT* prect ) = 0;
};

