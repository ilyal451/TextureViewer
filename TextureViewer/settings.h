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

extern COLORREF g_clrBackground;
extern int g_eChannelMode;
extern COLORREF g_clrAlpha;
extern int g_iAlphaOpacity;
extern bool g_bShowBorder;
extern COLORREF g_clrBorder;
extern int g_eMinFilter;
extern int g_eMagFilter;
extern int g_eTileMode;
extern int g_nTileSteps;
extern int g_eProjection;
extern bool g_bClipImage;
extern int g_eZoomOrigin;
extern bool g_bSoftZoom;
extern bool g_bWrapAround;
extern bool g_bAutoZoom;
extern bool g_bResetZoom;
extern bool g_bResetSubItems;
extern int g_eDefChannelMode;
extern int g_eDefTileMode;
extern int g_eDefGamma;
extern bool g_bPrecacheFiles;

extern bool g_bShowMenu;
extern bool g_bShowToolbar;
extern bool g_bShowStatus;
extern bool g_bShowFileList;
extern bool g_bShowFileSettings;

extern bool g_bWindowMaximized;
extern int g_iWindowLeft;
extern int g_iWindowTop;
extern int g_iWindowWidth;
extern int g_iWindowHeight;

extern int g_iFileListLeft;
extern int g_iFileListTop;
extern int g_iFileListWidth;
extern int g_iFileListHeight;

extern int g_iProcessingLeft;
extern int g_iProcessingTop;
extern int g_iProcessingWidth;
extern int g_iProcessingHeight;

void ResetSettings(void);
void LoadSettings(void);
void SaveSettings(void);
BOOL CALLBACK SettingsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

