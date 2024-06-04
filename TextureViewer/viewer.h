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

#include <shlobj.h>

#include "viewport.h"

enum
{
	PROJ_XY = 0,
	PROJ_ZY,
	PROJ_XZ,
};

enum Commands
{
    CMD_NULL = 0,
	CMD_CHAN_RGB,
	CMD_CHAN_RGBA,
	CMD_CHAN_RED,
	CMD_CHAN_GREEN,
	CMD_CHAN_BLUE,
	CMD_CHAN_ALPHA,
	CMD_SINGLE_TILE,
	CMD_TILE_HORZ,
	CMD_TILE_VERT,
    CMD_TILE_HORZVERT,
    CMD_RESET_SIZE,
    CMD_RESET_ORIGIN, // designed to work on unclipped?
    //CMD_TOGGLE_ZOOM_MODE,
    CMD_ZOOM_OUT,
    CMD_ZOOM_IN,
	CMD_PROJ_XY,
	CMD_PROJ_ZY,
	CMD_PROJ_XZ,
    CMD_PREV_SLICE,
    CMD_NEXT_SLICE,
    CMD_PREV_MIPMAP,
    CMD_NEXT_MIPMAP,
    CMD_PREV_TEXTURE,
    CMD_NEXT_TEXTURE,
    CMD_PREV_FILE,
    CMD_NEXT_FILE,
	CMD_BEGIN_SCROLL,
	CMD_END_SCROLL,
	CMD_SHELL,
	CMD_DRAGDROP,
	CMD_MARK_FILE,
	CMD_SHOW_GUIDELINES,
	CMD_APPLY_FILE_SETTINGS,
	CMD_OPEN,
	CMD_SAVE_TGA,
	CMD_SAVE_BMP,
	CMD_LOAD_PROJECT,
	CMD_SAVE_PROJECT,
	CMD_EXIT,
	CMD_ASSOC,
	CMD_SETTINGS,
	CMD_FILE_LIST,
	CMD_FILE_SETTINGS,
	CMD_ABOUT,
	NUM_COMMANDS
};

// additional virtual keys
enum AddVKs
{
	TK_LBUTTON = 256,
	TK_RBUTTON,
	TK_MBUTTON,
	TK_MWHEELUP,
	TK_MWHEELDOWN,
	NUM_TK
};

enum ZoomOrigins
{
	ZOOM_VIEWPORT_CENTER = 0,
	ZOOM_MOUSE_CURSOR,
	NUM_ZOOM_ORIGINS
};

class CTextureViewer
{
public:

	CTextureViewer();
	~CTextureViewer();

    int WM_Create(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_Destroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_Close(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_Size(HWND hWnd, WPARAM wParam, LPARAM lParam);
    int WM_Command(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_KeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_KeyUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_MouseWheel(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_LButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_LButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_MButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_MButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_RButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
    void WM_RButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);

	bool HandleShellMenuMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void DragDrop(void);

	void SetShowMenu(bool bShow, bool bInit = false);
	void SetShowToolbar(bool bShow, bool bInit = false);
	void SetShowStatus(bool bShow, bool bInit = false);
	void SetShowFileList(bool bShow, bool bInit = false);
	void SetShowFileSettings(bool bShow, bool bInit = false);
	void SetBackgroundColor(COLORREF clr, bool bRedraw, bool bInit = false);
	void SetChannelMode(int eMode, bool bRedraw, bool bInit = false);
	void SetAlphaColor(COLORREF clr, bool bRedraw, bool bInit = false);
	void SetAlphaOpacity(int iOpacity, bool bRedraw, bool bInit = false);
	void SetShowBorder(bool bShow, bool bRedraw, bool bInit = false);
	void SetBorderColor(COLORREF clr, bool bRedraw, bool bInit = false);
	void SetTileMode(int eMode, bool bRedraw, bool bInit = false);
	void SetNumTiles(int nTiles, bool bRedraw, bool bInit = false);
	void SetProjection(int eProjection, bool bRedraw, bool bInit = false);
	void SetMinFilter(int eFilter, bool bRedraw, bool bInit = false);
	void SetMagFilter(int eFilter, bool bRedraw, bool bInit = false);
	void SetClipImage(bool bClip, bool bInit = false);
	//void SetClipImageExtent(int iExtent, bool bInit = false);
	void SetZoomOrigin(int eOrigin, bool bInit = false);
	void SetSoftZoom(bool b, bool bInit = false);
	void SetWrapAround(bool b, bool bInit = false);
	void SetAutoZoom(bool b, bool bInit = false);
	void SetResetZoom(bool b, bool bInit = false);
	void SetResetSubItems(bool b, bool bInit = false);
	void SetDefChannelMode(int eMode, bool bInit = false);
	void SetDefTileMode(int eMode, bool bInit = false);
	void SetDefGamma(int eGamma, bool bInit = false);
	void SetPrecacheFiles(bool b, bool bInit = false);

	void ResetSize(void);
	void ResetOrigin(void);

	void SetPickingColor( bool bPicking );
	void PickColor( bool bValid, Rect_t* prect );
	void FinishColorPicking( void );
	void SetCoords(bool bValid, int x, int y);

	int GetNumFiles(void);
	int GetFile(void);
	const wchar_t* GetFileName(void);
	int GetNumTextures(void);
	int GetTexture(void);
	int GetNumMIPMaps(void);
	int GetMIPMap(void);
	int GetNumSlices(void);
	int GetSlice(void);
	int GetImageWidth(void);
	int GetImageHeight(void);
	const char* GetImageFormatStr(void);
	int GetImageGamma(void);
	int GetImageFlags(void);

	void UpdateInterface(void);
	void UpdateZoomLevel(void);
	void SetZoomLevel( int iZoomLevel );
	void ChangeZoomLevel( int iChange );
	void UpdatePixels( void );
	void UpdateImage( void );
	bool IsValidImage( void );
	void UpdateSlice( void );
	void SetSlice( int iSlice );
	void ChangeSlice( int iChange );
	bool IsValidSlice( void );
	void UpdateMIPMap( void );
	void SetMIPMap( int iMIPMap );
	void ChangeMIPMap( int iChange );
	bool IsValidMIPMap( void );
	void UpdateTexture( void );
	void SetTexture( int iTexture );
	void ChangeTexture( int iChange );
	bool IsValidTexture( void );
	void UpdateFile( void );
	void SetFile( int iFile );
	void ChangeFile( int iChange );
	bool IsValidFile( void );

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void ProcessCommandLine(void);
	void ShowWindow1( void );

private:

	void ApplySettings(bool bInit = false);
	void DumpSettings(void);

	void CreateMainMenu(void);
	void CreateToolbar(void);
	void CreateStatus(void);
	void CreateViewport(void);

	void CheckToolbarButtons(int eCmd, BOOL bChecked);
	void EnableToolbarButtons(int eCmd, BOOL bEnabled);
	void SetStatusText(int ePart, wchar_t* psz);

	void ToggleFileList(void);
	void ToggleFileSettings(void);

	void UpdateTiling(void);

	static void __cdecl ReadPixelsStatic(void* buffer, void* param);
	void ReadPixels(void* buffer);

	bool SetSource(wchar_t* pszPath, wchar_t* pszFilter);
	bool ChooseSource(void);
	void LoadProject(void);
	void SaveProject(void);
	void SaveTGA(void);
	void SaveBMP(void);

	void ShowAssocDialog(void);
	void ShowSettingsDialog(void);
	void ShowBrowserSettingsDialog(void);
	void ShowAboutDialog(void);
	void ShowEULADialog(void);
	void ShowRegisterDialog(void);
	void Shell(void);
	//void DragDrop(void);
	void MarkFile(void);


	HWND m_hWnd;
	HMENU m_hMainMenu;
	HWND m_hToolbar;
	HWND m_hStatus;
	HWND m_hViewport;
	CViewport* m_pViewport;
	IContextMenu2 *m_pcm2; // TODO: remove this

	//int m_nFiles;
	//int m_iFile;
	int m_nTextures;
	int m_iTexture;
	int m_nMIPMaps;
	int m_iMIPMap;
	int m_nSlices;
	int m_iSlice;
	int m_iImageWidth;
	int m_iImageHeight;
	const char* m_pszImageFormatString;
	int m_iZoomLevel;

	bool m_bValidImage;
	bool m_bValidMIPMap;
	bool m_bValidTexture;
	bool m_bValidFile;

	bool m_bShowMenu;
	bool m_bShowToolbar;
	bool m_bShowStatus;
	bool m_bShowFileList;
	bool m_bShowFileSettings;
	bool m_bShowFileInfo;
	bool m_bShowProcessor;

	COLORREF m_clrBackground;
	int m_eChannelMode;
	COLORREF m_clrAlpha;
	int m_iAlphaOpacity;
	bool m_bShowBorder;
	COLORREF m_clrBorder;
	int m_eTileMode;
	int m_eLastTileMode;
	int m_nTileSteps;
	int m_eProjection;
	int m_eMinFilter;
	int m_eMagFilter;
	bool m_bClipImage;
	int m_iClipImageExtent;
	int m_eZoomOrigin;
	bool m_bSoftZoom;
	bool m_bWrapAround;
	bool m_bAutoZoom;
	//bool m_bResetState;
	bool m_bResetZoom;
	bool m_bResetSubItems;
	int m_eDefChannelMode;
	int m_eDefTileMode;
	int m_eDefGamma;
	bool m_bPrecacheFiles;

	bool m_bCoords;
	int m_xCoords;
	int m_yCoords;

	bool m_bUpdateChannelMode;
	bool m_bUpdateTileMode;
	bool m_bUpdateProjection;
	bool m_bUpdateFile;
	bool m_bUpdateTexture;
	bool m_bUpdateMIPMap;
	bool m_bUpdateSlice;
	bool m_bUpdateZoomLevel;
	bool m_bUpdateImageSize;
	bool m_bUpdateFormat;
	bool m_bUpdateCoords;
	bool m_bUpdateWindowCaption;

	wchar_t m_szFileNum[32];		// "1024/1025"
	wchar_t m_szTextureNum[32];		// "1/1"
	wchar_t m_szMIPMapNum[32];		// "1/8"
	wchar_t m_szSliceNum[32];		// "1/1"
	wchar_t m_szImageSize[32];		// "512x512"
	wchar_t m_szZoomPercent[32];	// "100%"
	wchar_t m_szCoords[32];			// "x:10,y:50"
};

extern CTextureViewer* g_pTextureViewer;

