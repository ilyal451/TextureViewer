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
#include <stdio.h>

#include "../shared/utils.h"
#include "../shared/rect.h"
#include "../shared/parser.h"

#include "context.h"

#include "filelist.h"
#include "processing.h"

#include "viewer.h"

#include "settings.h"
#include "keyconfig.h"

#include "resource.h"


extern HWND g_hAssocDlg;

#define MIN_ZOOM_LEVEL -5
#define MAX_ZOOM_LEVEL 5



CTextureViewer::CTextureViewer(void)
{
	m_hWnd = NULL;
	m_hMainMenu = NULL;
	m_hToolbar = NULL;
	m_hStatus = NULL;
	m_hViewport = NULL;
	m_pcm2 = NULL;

	m_nTextures = 0;
	m_iTexture = -1;
	m_nMIPMaps = 0;
	m_iMIPMap = -1;
	m_iImageWidth = 0;
	m_iImageHeight = 0;
	m_nSlices = 0;
	m_iSlice = -1;

	m_iZoomLevel = 0;
	m_bValidImage = false;
	m_bValidMIPMap = false;
	m_bValidTexture = false;
	m_bValidFile = false;
	m_pszImageFormatString = NULL;

	m_bShowMenu = true;
	m_bShowToolbar = true;
	m_bShowStatus = true;
	m_bShowFileList = true;
	m_bShowFileSettings = true;
	m_bShowFileInfo = true;
	m_bShowProcessor = true;

	m_clrBackground = 0x00000000;
	m_eChannelMode = CM_RGBA;
	m_clrAlpha = 0x000000FF;
	m_iAlphaOpacity = 127;
	m_bShowBorder = true;
	m_clrBorder = 0x00FFFFFF;
	m_eTileMode = TM_SINGLE_TILE;
	m_eLastTileMode = TM_SINGLE_TILE;
	m_nTileSteps = 1;
	m_eProjection = PROJ_XY;
	m_eMinFilter = 1;
	m_eMagFilter = 0;
	m_bClipImage = true;
	m_iClipImageExtent = 0; // rem
	m_eZoomOrigin = 0;

	m_bCoords = false;
	m_xCoords = 0;
	m_yCoords = 0;

	m_bUpdateChannelMode = false;
	m_bUpdateTileMode = false;
	m_bUpdateProjection = false;
	m_bUpdateFile = false;
	m_bUpdateTexture = false;
	m_bUpdateMIPMap = false;
	m_bUpdateSlice = false;
	m_bUpdateZoomLevel = false;
	m_bUpdateImageSize = false;
	m_bUpdateFormat = false;
	m_bUpdateCoords = false;
	m_bUpdateWindowCaption = false;
}


CTextureViewer::~CTextureViewer()
{
	//
}


wchar_t* ConstructMenuItem( wchar_t* pszItem, int eCmd, wchar_t* buffer )
{
	int n = wsprintf( buffer, L"%s", pszItem );
	GetKeyBindings( eCmd, &buffer[n] );
	return buffer;
}


void CTextureViewer::CreateMainMenu(void)
{
	wchar_t buffer[128];

	// Help
	HMENU hHelp = CreateMenu();
	AppendMenu(hHelp, MF_STRING, CMD_ABOUT, ConstructMenuItem( L"About...", CMD_ABOUT, buffer ));

	// Tools
	HMENU hTools = CreateMenu();
	AppendMenu(hTools, MF_STRING, CMD_ASSOC, ConstructMenuItem( L"File Associations...", CMD_ASSOC, buffer ));
	AppendMenu(hTools, MF_STRING, CMD_SETTINGS, ConstructMenuItem( L"Settings...", CMD_SETTINGS, buffer ));

	// View.Channels
	HMENU hChannels = CreateMenu();
	AppendMenu(hChannels, MF_STRING, CMD_CHAN_RGB, ConstructMenuItem( L"RGB", CMD_CHAN_RGB, buffer ));
	AppendMenu(hChannels, MF_STRING, CMD_CHAN_RGBA, ConstructMenuItem( L"RGB + Alpha", CMD_CHAN_RGBA, buffer ));
	// RG?
	AppendMenu(hChannels, MF_STRING, CMD_CHAN_RED, ConstructMenuItem( L"Red", CMD_CHAN_RED, buffer ));
	AppendMenu(hChannels, MF_STRING, CMD_CHAN_GREEN, ConstructMenuItem( L"Green", CMD_CHAN_GREEN, buffer ));
	AppendMenu(hChannels, MF_STRING, CMD_CHAN_BLUE, ConstructMenuItem( L"Blue", CMD_CHAN_BLUE, buffer ));
	AppendMenu(hChannels, MF_STRING, CMD_CHAN_ALPHA, ConstructMenuItem( L"Alpha", CMD_CHAN_ALPHA, buffer ));

	// View.Tiling
	HMENU hTiling = CreateMenu();
	AppendMenu(hTiling , MF_STRING, CMD_SINGLE_TILE, ConstructMenuItem( L"Single Tile", CMD_SINGLE_TILE, buffer ));
	AppendMenu(hTiling , MF_STRING, CMD_TILE_HORZVERT, ConstructMenuItem( L"Tile Horizontal and Vertical", CMD_TILE_HORZVERT, buffer ));
	AppendMenu(hTiling , MF_STRING, CMD_TILE_HORZ, ConstructMenuItem( L"Tile Horizontal", CMD_TILE_HORZ, buffer ));
	AppendMenu(hTiling , MF_STRING, CMD_TILE_VERT, ConstructMenuItem( L"Tile Vertical", CMD_TILE_VERT, buffer ));

	// View.Reset
	HMENU hReset = CreateMenu();
	AppendMenu(hReset, MF_STRING, CMD_RESET_SIZE, ConstructMenuItem( L"Zoom", CMD_RESET_SIZE, buffer ));
	AppendMenu(hReset, MF_STRING, CMD_RESET_ORIGIN, ConstructMenuItem( L"Position", CMD_RESET_ORIGIN, buffer ));

	// Navigation
	HMENU hNavigation = CreateMenu();
	AppendMenu(hNavigation, MF_STRING, CMD_NEXT_FILE, ConstructMenuItem( L"Next File", CMD_NEXT_FILE, buffer ));
	AppendMenu(hNavigation, MF_STRING, CMD_PREV_FILE, ConstructMenuItem( L"Previous File", CMD_PREV_FILE, buffer ));
	AppendMenu(hNavigation, MF_SEPARATOR, 0, NULL);
	AppendMenu(hNavigation, MF_STRING, CMD_NEXT_TEXTURE, ConstructMenuItem( L"Next Image", CMD_NEXT_TEXTURE, buffer ));
	AppendMenu(hNavigation, MF_STRING, CMD_PREV_TEXTURE, ConstructMenuItem( L"Previous Image", CMD_PREV_TEXTURE, buffer ));
	AppendMenu(hNavigation, MF_SEPARATOR, 0, NULL);
	AppendMenu(hNavigation, MF_STRING, CMD_NEXT_MIPMAP, ConstructMenuItem( L"Next MIP Map", CMD_NEXT_MIPMAP, buffer ));
	AppendMenu(hNavigation, MF_STRING, CMD_PREV_MIPMAP, ConstructMenuItem( L"Previous MIP Map", CMD_PREV_MIPMAP, buffer ));
	AppendMenu(hNavigation, MF_SEPARATOR, 0, NULL);
	AppendMenu(hNavigation, MF_STRING, CMD_NEXT_SLICE, ConstructMenuItem( L"Next Slice", CMD_NEXT_SLICE, buffer ));
	AppendMenu(hNavigation, MF_STRING, CMD_PREV_SLICE, ConstructMenuItem( L"Previous Slice", CMD_PREV_SLICE, buffer ));

	HMENU hWindows = CreateMenu();
	AppendMenu(hWindows, MF_STRING, CMD_FILE_LIST, ConstructMenuItem( L"File List", CMD_FILE_LIST, buffer ));
	AppendMenu(hWindows, MF_STRING, CMD_FILE_SETTINGS, ConstructMenuItem( L"Processing", CMD_FILE_SETTINGS, buffer ));

	// View
	HMENU hView = CreateMenu();
	AppendMenu(hView, MF_POPUP | MF_STRING, (UINT_PTR)hChannels, L"Channel Mode");
	AppendMenu(hView, MF_SEPARATOR, 0, NULL);
	AppendMenu(hView, MF_POPUP | MF_STRING, (UINT_PTR)hTiling, L"Tile Mode");
	AppendMenu(hView, MF_SEPARATOR, 0, NULL);
	AppendMenu(hView, MF_STRING, CMD_ZOOM_IN, ConstructMenuItem( L"Zoom +", CMD_ZOOM_IN, buffer ));
	AppendMenu(hView, MF_STRING, CMD_ZOOM_OUT, ConstructMenuItem( L"Zoom -", CMD_ZOOM_OUT, buffer ));
	AppendMenu(hView, MF_SEPARATOR, 0, NULL);
	AppendMenu(hView, MF_POPUP | MF_STRING, (UINT_PTR)hNavigation, L"Navigation");
	AppendMenu(hView, MF_SEPARATOR, 0, NULL);
	AppendMenu(hView, MF_POPUP | MF_STRING, (UINT_PTR)hWindows, L"Windows");

	// File
	HMENU hFile = CreateMenu();
	AppendMenu(hFile, MF_STRING, CMD_OPEN, ConstructMenuItem( L"Open...", CMD_OPEN, buffer ));
	AppendMenu(hFile, MF_SEPARATOR, 0, NULL);

	//AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
	//AppendMenu(hFile, MF_STRING, CMD_SAVE_TGA, ConstructMenuItem( L"Save TGA...", CMD_SAVE_TGA, buffer ));
	AppendMenu(hFile, MF_STRING, CMD_SAVE_BMP, ConstructMenuItem( L"Save Bitmap...", CMD_SAVE_BMP, buffer ));
	AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
	AppendMenu(hFile, MF_STRING, CMD_EXIT, ConstructMenuItem( L"Exit", CMD_EXIT, buffer ));

	// Main
	HMENU hMenu = CreateMenu();
	AppendMenu(hMenu, MF_POPUP | MF_STRING, (UINT_PTR)hFile, L"File");
	AppendMenu(hMenu, MF_POPUP | MF_STRING, (UINT_PTR)hView, L"View");
	AppendMenu(hMenu, MF_POPUP | MF_STRING, (UINT_PTR)hTools, L"Tools");
	AppendMenu(hMenu, MF_POPUP | MF_STRING, (UINT_PTR)hHelp, L"Help");

	m_hMainMenu = hMenu;

	SetMenu(m_hWnd, m_hMainMenu);
}


// do not break the order
enum ToolbarButtons
{
    TBB_NONE = 0,
    TBB_SEPARATOR,
	TBB_CHANNEL_RGB,
	TBB_CHANNEL_RGBA,
    TBB_CHANNEL_R,
    TBB_CHANNEL_G,
    TBB_CHANNEL_B,
    TBB_CHANNEL_A,
	TBB_SINGLE_TILE,
    TBB_TILE_HORZVERT,
	TBB_TILE_HORZ,
	TBB_TILE_VERT,
    TBB_ZOOM_OUT,
    TBB_ZOOM_IN,
    TBB_PREV_SLICE,
    TBB_NEXT_SLICE,
    TBB_PREV_MIPMAP,
    TBB_NEXT_MIPMAP,
    TBB_PREV_TEXTURE,
    TBB_NEXT_TEXTURE,
    TBB_PREV_FILE,
    TBB_NEXT_FILE,
	TBB_FILE_LIST,
	TBB_FILE_SETTINGS,
    TBB_PROJ_XY,
	TBB_PROJ_ZY,
	TBB_PROJ_XZ,
    NUM_TB_BUTTONS
};

// ! should match the above definition !
char* g_apszToolbarButtonNames[NUM_TB_BUTTONS] =
{
	"NONE",
	"SEPARATOR",
	"CHANNEL_RGB",
	"CHANNEL_RGBA",
	"CHANNEL_RED",
	"CHANNEL_GREEN",
	"CHANNEL_BLUE",
	"CHANNEL_ALPHA",
	"TILE_SINGLE",
	"TILE_HORZVERT",
	"TILE_HORZ",
	"TILE_VERT",
	"ZOOM_OUT",
	"ZOOM_IN",
	"SLICE_PREV",
	"SLICE_NEXT",
	"MIPMAP_PREV",
	"MIPMAP_NEXT",
	"TEXTURE_PREV",
	"TEXTURE_NEXT",
	"FILE_PREV",
	"FILE_NEXT",
	"FILE_LIST",
	"FILE_SETTINGS",
	"PROJ_XY",
	"PROJ_ZY",
	"PROJ_XZ",
};

TBBUTTON g_atbButtons[NUM_TB_BUTTONS] =
{
    { TBB_NONE,             0,                          0, 0, {0}, 0, 0 },
    { 5,                    0,                          TBSTATE_ENABLED, TBSTYLE_SEP, {0}, 0, 0 },
	{ TBB_CHANNEL_RGB,      CMD_CHAN_RGB,               TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"RGB" },
	{ TBB_CHANNEL_RGBA,     CMD_CHAN_RGBA,              TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"RGB + alpha" },
	{ TBB_CHANNEL_R,        CMD_CHAN_RED,               TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Red" },
    { TBB_CHANNEL_G,        CMD_CHAN_GREEN,             TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Green" },
    { TBB_CHANNEL_B,        CMD_CHAN_BLUE,              TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Blue" },
    { TBB_CHANNEL_A,        CMD_CHAN_ALPHA,             TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Alpha" },
    { TBB_SINGLE_TILE,      CMD_SINGLE_TILE,            TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Single tile" },
    { TBB_TILE_HORZVERT,    CMD_TILE_HORZVERT,          TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Tile horizontal and vertical" },
	{ TBB_TILE_HORZ,        CMD_TILE_HORZ,              TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Tile horizontal" },
    { TBB_TILE_VERT,        CMD_TILE_VERT,              TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Tile vertical" },
	{ TBB_ZOOM_OUT,         CMD_ZOOM_OUT,               TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Zoom -" },
    { TBB_ZOOM_IN,          CMD_ZOOM_IN,                TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Zoom +" },
	{ TBB_PREV_SLICE,       CMD_PREV_SLICE,             TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Previous slice" },
    { TBB_NEXT_SLICE,       CMD_NEXT_SLICE,             TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Next slice" },
    { TBB_PREV_MIPMAP,      CMD_PREV_MIPMAP,            TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Previous MIP map" },
    { TBB_NEXT_MIPMAP,      CMD_NEXT_MIPMAP,            TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Next MIP map" },
    { TBB_PREV_TEXTURE,     CMD_PREV_TEXTURE,           TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Previous image" },
    { TBB_NEXT_TEXTURE,     CMD_NEXT_TEXTURE,           TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Next image" },
	{ TBB_PREV_FILE,        CMD_PREV_FILE,              TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Previous file" },
    { TBB_NEXT_FILE,        CMD_NEXT_FILE,              TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Next file" },
    { TBB_FILE_LIST,        CMD_FILE_LIST,              TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"File list" },
    { TBB_FILE_SETTINGS,    CMD_FILE_SETTINGS,          TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"Processing" },
    { TBB_PROJ_XY,          CMD_PROJ_XY,                TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"XY projection (front)" },
    { TBB_PROJ_ZY,          CMD_PROJ_ZY,                TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"ZY projection (right)" },
    { TBB_PROJ_XZ,          CMD_PROJ_XZ,                TBSTATE_ENABLED, 0, {0}, 0, (INT_PTR)L"XZ projection (bottom)" },

};

#define MAX_TOOLBAR_BUTTONS 48
int g_aiToolbarButtons[MAX_TOOLBAR_BUTTONS] =
{
    TBB_SEPARATOR,
	TBB_CHANNEL_RGB,
	TBB_CHANNEL_RGBA,
    TBB_CHANNEL_R,
    TBB_CHANNEL_G,
    TBB_CHANNEL_B,
    TBB_CHANNEL_A,
	TBB_SEPARATOR,
	TBB_SINGLE_TILE,
	TBB_TILE_HORZVERT,
	TBB_TILE_HORZ,
	TBB_TILE_VERT,
	TBB_SEPARATOR,
	TBB_ZOOM_IN,
	TBB_ZOOM_OUT,
    TBB_SEPARATOR,
    TBB_PREV_SLICE, 
    TBB_NEXT_SLICE, 
    TBB_SEPARATOR,
    TBB_PREV_MIPMAP, 
    TBB_NEXT_MIPMAP, 
    TBB_SEPARATOR,
    TBB_PREV_TEXTURE, 
    TBB_NEXT_TEXTURE, 
    TBB_SEPARATOR,
    TBB_PREV_FILE, 
    TBB_NEXT_FILE, 
    TBB_SEPARATOR,
	TBB_FILE_LIST, 
	TBB_FILE_SETTINGS, 
	//TBB_SEPARATOR,
	//TBB_PROJ_XY,
	//TBB_PROJ_ZY,
	//TBB_PROJ_XZ,
};

int g_nToolbarButtons;

bool ToolbarLineProc(char* pszLine, void* param)
{
	if ( *pszLine == '\0' )
	{
		return true;
	}

	int argc;
	char* argv[2];
	argc = ParseLine(1, argv, pszLine, ",", NULL);

	// the 1 is important (it skips "NONE")
	for ( int i = 1; i < NUM_TB_BUTTONS; i++ )
	{
		if ( FStrEq( argv[0], g_apszToolbarButtonNames[i] ) )
		{
			if ( g_nToolbarButtons >= MAX_TOOLBAR_BUTTONS )
			{
				PARSER_ERROR( "too many toolbar buttons (%d allowed)", MAX_TOOLBAR_BUTTONS );
			}

			if ( i != TBB_SEPARATOR )
			{
				for ( int j = 0; j < g_nToolbarButtons; j++ )
				{
					if ( g_aiToolbarButtons[j] == i )
					{
						PARSER_ERROR( "%s is already added", g_apszToolbarButtonNames[i] );
					}
				}
			}

			g_aiToolbarButtons[g_nToolbarButtons++] = i;

			return true;
		}
	}

	PARSER_ERROR( "unknown toolbar button %s", argv[0] );

	return false;
}


void LoadToolbarButtons( void )
{
	wchar_t szFileName[MAX_PATH];

	wcscpy(szFileName, GetAppDir());
	wcscat(szFileName, L"Toolbar.ini" );

	g_nToolbarButtons = 0;
	if ( !ParseConfig( szFileName, ToolbarLineProc ) )
	{
		int i;
		for ( i = 0; i < MAX_TOOLBAR_BUTTONS; i++ )
		{
			if ( g_aiToolbarButtons[i] == 0 )
			{
				break;
			}
		}
		g_nToolbarButtons = i;

		// init default
		FILE* stream = _wfopen( szFileName, L"wt" );
		if ( stream )
		{
			for ( i = 0; i < g_nToolbarButtons; i++ )
			{
				fprintf( stream, "%s\n", g_apszToolbarButtonNames[g_aiToolbarButtons[i]] );
			}
			fclose( stream );
		}
	}
}


void CTextureViewer::CreateToolbar(void)
{
    HIMAGELIST hImageList;
    HBITMAP hBmp;
    int i;

    m_hToolbar = CreateWindowEx(
        0,
        TOOLBARCLASSNAME,
        NULL,
        WS_CHILD | TBSTYLE_TOOLTIPS,
        0,
        0,
        4096,
        25,
        m_hWnd,
        NULL,
        g_hInst,
        NULL
        );

    hImageList = ImageList_Create(16, 16, ILC_MASK, 32, 32);

    hBmp = LoadBitmap(g_hInst, (LPCTSTR)IDB_BITMAP1);
    i = ImageList_AddMasked(hImageList, hBmp, 0x00FFFF00);
    DeleteObject(hBmp);

    SendMessage(m_hToolbar, TB_SETIMAGELIST, 0, (LPARAM)hImageList);
    SendMessage(m_hToolbar, TB_SETMAXTEXTROWS, 0, 0);
    SendMessage(m_hToolbar, TB_SETPADDING, 0, MAKELONG(10,10));
    SendMessage(m_hToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

	LoadToolbarButtons();
    for (i = 0; (g_aiToolbarButtons[i] != 0) && (i < MAX_TOOLBAR_BUTTONS); i++)
	{
        SendMessage(m_hToolbar, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&g_atbButtons[g_aiToolbarButtons[i]]);
    }
}


enum StatusBarParts
{
    SBP_FILE,
	SBP_TEXTURE,
    SBP_MIPMAP,
    SBP_SLICE,
    SBP_IMAGE_SIZE,
    SBP_ZOOM_LEVEL,
    SBP_FORMAT,
	NUM_SB_PARTS
};

char* g_apszStatusBarPartNames[NUM_SB_PARTS] =
{
	"FILE",
	"IMAGE",
	"MIP_MAP",
	"SLICE",
	"IMAGE_SIZE",
	"ZOOM_LEVEL",
	"FORMAT_STR",
};

int g_aiStatusBarPartLookup[NUM_SB_PARTS];

int g_aiStatusBarPartWidths[NUM_SB_PARTS] =
{
    100,
	90,
    90,
    80,
    100,
    80,
    -1,
};

int g_nStatusBarButtons;


bool StatusBarLineProc(char* pszLine, void* param)
{
	if ( *pszLine == '\0' )
	{
		return true;
	}

	int argc;
	char* argv[2];
	argc = ParseLine(2, argv, pszLine, ",", NULL);
	
	if (argc != 2)
	{
		PARSER_ERROR( "incomplete line" );
	}

	for ( int i = 0; i < NUM_SB_PARTS; i++ )
	{
		if ( FStrEq( argv[0], g_apszStatusBarPartNames[i] ) )
		{
			if ( g_aiStatusBarPartLookup[i] != -1 )
			{
				PARSER_ERROR( "%s is already assigned", g_apszStatusBarPartNames[i] );
			}

			if ( g_nStatusBarButtons >= NUM_SB_PARTS )
			{
				PARSER_ERROR( "too many SB parts (%d allowed)", NUM_SB_PARTS );
			}

			g_aiStatusBarPartLookup[i] = g_nStatusBarButtons;
			g_aiStatusBarPartWidths[g_nStatusBarButtons] = atoi( argv[1] );

			g_nStatusBarButtons++;

			return true;
		}
	}

	PARSER_ERROR( "unknown SB part %s", argv[0] );

	return false;
}


void LoadStatusBarButtons( void )
{
	wchar_t szFileName[MAX_PATH];

	wcscpy(szFileName, GetAppDir());
	wcscat(szFileName, L"StatusBar.ini" );

	g_nStatusBarButtons = 0;
	if ( !ParseConfig( szFileName, StatusBarLineProc ) )
	{
		// init default
		FILE* stream = _wfopen( szFileName, L"wt" );
		if ( stream )
		{
			for ( int i = 0; i < NUM_SB_PARTS; i++ )
			{
				fprintf( stream, "%s,%d\n", g_apszStatusBarPartNames[i], g_aiStatusBarPartWidths[i] );
			}
			fclose( stream );
		}
		for ( int i = 0; i < NUM_SB_PARTS; i++ )
		{
			g_aiStatusBarPartLookup[i] = i;
			g_nStatusBarButtons++;
		}
	}
}


void CTextureViewer::CreateStatus(void)
{
    int aiParts[NUM_SB_PARTS];
    int i, j;

    m_hStatus = CreateWindowEx(
        0,
        STATUSCLASSNAME,
        NULL,
        WS_CHILD | SBARS_SIZEGRIP,
        0,
        0,
        1,
        1,
        m_hWnd,
        NULL,
        g_hInst,
        NULL
        );

	for ( i = 0; i < NUM_SB_PARTS; i++ )
	{
		g_aiStatusBarPartLookup[i] = -1;
	}

	LoadStatusBarButtons();

    for (i = 0, j = 0; i < g_nStatusBarButtons; i++)
	{
        j += g_aiStatusBarPartWidths[i];
        aiParts[i] = j;
    }
	if ( i > 0 )
	{
		aiParts[i-1] = -1;
		SendMessage(m_hStatus, SB_SETPARTS, (WPARAM)i, (LPARAM)aiParts);
	}
}


void CTextureViewer::CreateViewport(void)
{
	const wchar_t* pszWindowClass = L"VCtrl_Viewport";
	WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_OWNDC; //NULL; //CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = ViewportWndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = sizeof( void* );
    wc.hInstance = g_hInst;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = pszWindowClass;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIconSm = 0;

    if (RegisterClassEx(&wc))
	{
		m_hViewport = CreateWindowEx(
			NULL,
			pszWindowClass,
			NULL,
			WS_VISIBLE | WS_CHILD | WS_HSCROLL | WS_VSCROLL,
			0,
			0,
			0, 
			0, 
			m_hWnd,
			NULL,
			g_hInst,
			NULL
			);
	}
}


void CTextureViewer::ProcessCommandLine(void)
{
	int argc;
	wchar_t* argv[2];

	wcscpy(wbuffer, GetCommandLineW());

	ContractCharsW(wbuffer, L" \t", ' ');
	argc = ParseLineW(2, argv, wbuffer, L" ", NULL);

	if (argc > 1)
	{
		SetSource(argv[1], NULL);

		return;
	}

	if ( !ChooseSource() )
	{
		//NONFATAL_ERROR( "No source, exiting"
		exit(0);
	}

	m_pViewport->Update();

	UpdateInterface();
}


// the window is just shown
void CTextureViewer::ShowWindow1( void )
{
	SetShowFileList(g_bShowFileList, true);
	SetShowFileSettings(g_bShowFileSettings, true);
}


int CTextureViewer::WM_Create(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_hWnd = hWnd;

	CreateMainMenu();
    CreateToolbar();
    CreateStatus();
	CreateViewport();
	if (m_hViewport == NULL)
	{
		goto quit;
	}
	m_pViewport = (CViewport*)GetWindowLong(m_hViewport, 0);

	m_pViewport->SetPixelsCallbackProc(ReadPixelsStatic, this);

	InitFileList(hWnd);

	InitFileSettings(hWnd);

	ApplySettings(true);


    return 0;

quit:

	FATAL_ERROR( L"Create main window error" );

	delete this; // ???

	return 1;
}


void CTextureViewer::WM_Destroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	//
}


void CTextureViewer::CheckToolbarButtons(int eCmd, BOOL bChecked)
{
    PostMessage(m_hToolbar, TB_CHECKBUTTON, (WPARAM)eCmd, (LPARAM)bChecked);
	CheckMenuItem(m_hMainMenu, eCmd, (bChecked)? MF_CHECKED: MF_UNCHECKED);
}


void CTextureViewer::EnableToolbarButtons(int eCmd, BOOL bEnabled)
{
    PostMessage(m_hToolbar, TB_ENABLEBUTTON, (WPARAM)eCmd, (LPARAM)bEnabled);
	//CheckMenuItem(m_hMainMenu, eCmd, (bChecked)? MF_CHECKED: MF_UNCHECKED);
}


void CTextureViewer::SetStatusText(int ePart, wchar_t* psz)
{
	int iPartId = g_aiStatusBarPartLookup[ePart];
	if ( iPartId != -1 )
	{
		SendMessage(m_hStatus, SB_SETTEXT, (WPARAM)iPartId, (LPARAM)psz);
	}
}


float GetZoomFloat(int iLevel)
{
    float flZoom;

    if (iLevel > 0)
	{
        flZoom = (float)(1 << iLevel);
    }
    else if (iLevel < 0)
	{
        flZoom = (float)(0x80000000 >> (-iLevel));
        flZoom /= 0x80000000;
    }
    else
	{
        flZoom = 1.0;
    }

    return flZoom;
}


void CTextureViewer::UpdateInterface(void)
{
	if (m_bUpdateChannelMode)
	{
		CheckToolbarButtons(CMD_CHAN_RGB, (m_eChannelMode == CM_RGB));
		CheckToolbarButtons(CMD_CHAN_RGBA, (m_eChannelMode == CM_RGBA));
		CheckToolbarButtons(CMD_CHAN_RED, (m_eChannelMode == CM_RED));
		CheckToolbarButtons(CMD_CHAN_GREEN, (m_eChannelMode == CM_GREEN));
		CheckToolbarButtons(CMD_CHAN_BLUE, (m_eChannelMode == CM_BLUE));
		CheckToolbarButtons(CMD_CHAN_ALPHA, (m_eChannelMode == CM_ALPHA));
		m_bUpdateChannelMode = false;
	}

	if (m_bUpdateTileMode)
	{
		m_bUpdateTileMode = false;
		CheckToolbarButtons(CMD_SINGLE_TILE, (m_eTileMode == TM_SINGLE_TILE));
		CheckToolbarButtons(CMD_TILE_HORZ, (m_eTileMode == TM_TILE_HORZ));
		CheckToolbarButtons(CMD_TILE_VERT, (m_eTileMode == TM_TILE_VERT));
		CheckToolbarButtons(CMD_TILE_HORZVERT, (m_eTileMode == TM_TILE_HORZVERT));
	}

	if (m_bUpdateProjection)
	{
		m_bUpdateProjection = false;
		CheckToolbarButtons(CMD_PROJ_XY, (m_eProjection == PROJ_XY));
		CheckToolbarButtons(CMD_PROJ_ZY, (m_eProjection == PROJ_ZY));
		CheckToolbarButtons(CMD_PROJ_XZ, (m_eProjection == PROJ_XZ));
	}

	if (m_bUpdateFile)
	{
		m_bUpdateFile = false;

		if ( GetFile() != -1 )
		{
			EnableToolbarButtons(CMD_NEXT_FILE, m_bWrapAround || ((GetFile() + 1) < GetNumFiles()));
			EnableToolbarButtons(CMD_PREV_FILE, m_bWrapAround || (GetFile() > 0));
			swprintf(
				m_szFileNum,
				L"%d/%d",
				GetFile() + 1,
				GetNumFiles()
				);
			swprintf(wbuffer, L" File: %s", m_szFileNum);
			SetStatusText(SBP_FILE, wbuffer);
		}
		else
		{
			EnableToolbarButtons(CMD_NEXT_FILE, FALSE);
			EnableToolbarButtons(CMD_PREV_FILE, FALSE);
			SetStatusText(SBP_FILE, NULL);
		}

		m_bUpdateTexture = true;
		m_bUpdateMIPMap = true;
		m_bUpdateSlice = true;
		m_bUpdateImageSize = true;
		m_bUpdateZoomLevel = true;
		m_bUpdateFormat = true;
		m_bUpdateCoords = true;
		m_bUpdateWindowCaption = true;
	}

	if (m_bUpdateTexture)
	{
		m_bUpdateTexture = false;

		if (IsValidFile())
		{
			EnableToolbarButtons(CMD_NEXT_TEXTURE, (GetTexture() + 1) < GetNumTextures());
			EnableToolbarButtons(CMD_PREV_TEXTURE, GetTexture() > 0);
			swprintf(
				m_szTextureNum,
				L"%d/%d",
				GetTexture() + 1,
				GetNumTextures()
				);
			swprintf(wbuffer, L" Image: %s", m_szTextureNum);
			SetStatusText(SBP_TEXTURE, wbuffer);
		}
		else
		{
			EnableToolbarButtons(CMD_NEXT_TEXTURE, FALSE);
			EnableToolbarButtons(CMD_PREV_TEXTURE, FALSE);
			SetStatusText(SBP_TEXTURE, NULL);
		}

		m_bUpdateMIPMap = true;
		m_bUpdateSlice = true;
		m_bUpdateImageSize = true;
		m_bUpdateZoomLevel = true;
		m_bUpdateFormat = true;
		m_bUpdateCoords = true;
		m_bUpdateWindowCaption = true;
	}

	if (m_bUpdateMIPMap)
	{
		m_bUpdateMIPMap = false;

		if (IsValidTexture())
		{
			EnableToolbarButtons(CMD_NEXT_MIPMAP, (GetMIPMap() + 1) < GetNumMIPMaps());
			EnableToolbarButtons(CMD_PREV_MIPMAP, GetMIPMap() > 0);
			swprintf(
				m_szMIPMapNum,
				L"%d/%d",
				GetMIPMap() + 1,
				GetNumMIPMaps()
				);
			swprintf(wbuffer, L" MIP map: %s", m_szMIPMapNum);
			SetStatusText(SBP_MIPMAP, wbuffer);
		}
		else
		{
			EnableToolbarButtons(CMD_NEXT_MIPMAP, FALSE);
			EnableToolbarButtons(CMD_PREV_MIPMAP, FALSE);
			SetStatusText(SBP_MIPMAP, NULL);
		}

		m_bUpdateSlice = true;
		m_bUpdateImageSize = true;
		m_bUpdateZoomLevel = true;
		m_bUpdateFormat = true;
		m_bUpdateCoords = true;
		m_bUpdateWindowCaption = true;
	}

	if (m_bUpdateSlice)
	{
		m_bUpdateSlice = false;

		if (IsValidMIPMap())
		{
			EnableToolbarButtons(CMD_NEXT_SLICE, (GetSlice() + 1) < m_nSlices);
			EnableToolbarButtons(CMD_PREV_SLICE, GetSlice() > 0);
			swprintf(
				m_szSliceNum,
				L"%d/%d",
				GetSlice() + 1,
				m_nSlices
				);
			swprintf(wbuffer, L" Slice: %s", m_szSliceNum);
			SetStatusText(SBP_SLICE, wbuffer);
		}
		else
		{
			EnableToolbarButtons(CMD_NEXT_SLICE, FALSE);
			EnableToolbarButtons(CMD_PREV_SLICE, FALSE);
			SetStatusText(SBP_SLICE, NULL);
		}

		m_bUpdateImageSize = true;
		m_bUpdateZoomLevel = true;
		m_bUpdateWindowCaption = true;
	}

	if (m_bUpdateImageSize)
	{
		m_bUpdateImageSize = false;

		if (IsValidImage())
		{
			swprintf(
				m_szImageSize, 
				L"%dx%d", 
				m_iImageWidth, 
				m_iImageHeight
				);
			swprintf(wbuffer, L" Size: %s", m_szImageSize);
			SetStatusText(SBP_IMAGE_SIZE, wbuffer);
		}
		else
		{
			SetStatusText(SBP_IMAGE_SIZE, NULL);
		}

		m_bUpdateZoomLevel = true;
		m_bUpdateCoords = true;
		m_bUpdateWindowCaption = true;
	}

	if (m_bUpdateZoomLevel)
	{
		m_bUpdateZoomLevel = false;
		//sprintf(buffer, " Zoom: %.lf%%", GetZoomFloat(g_iZoomLevel) * 100);
		swprintf(
			m_szZoomPercent, 
			L"%g%%", 
			GetZoomFloat(m_iZoomLevel) * 100
			);
		swprintf(wbuffer, L" Zoom: %s", m_szZoomPercent);
		SetStatusText(SBP_ZOOM_LEVEL, wbuffer);

		m_bUpdateCoords = true;
		m_bUpdateWindowCaption = true;
	}

	if (m_bUpdateFormat)
	{
		m_bUpdateFormat = false;

		if ( IsValidFile() && ( GetImageFormatStr() != NULL ) )
		{
			swprintf(wbuffer, L" Format: %S", GetImageFormatStr());
			SetStatusText(SBP_FORMAT, wbuffer);
		}
		else
		{
			SetStatusText(SBP_FORMAT, NULL);
		}
	}

	if (m_bUpdateCoords)
	{
		m_bUpdateCoords = false;

		if (m_bCoords)
		{
			swprintf(
				m_szCoords, 
				L" [x:%d y:%d]", 
				m_xCoords,
				m_yCoords
				);
		}
		else
		{
			m_szCoords[0] = '\0';
		}

		m_bUpdateWindowCaption = true;
	}

	if (m_bUpdateWindowCaption)
	{
		m_bUpdateWindowCaption = false;
		int n = swprintf(
			wbuffer,
			L"%s%s - Texture Viewer [%s] %s",
			(IsValidFile())? L"": L"[ERROR] ",
			(GetFile() == -1)? L"": GetFileName(),
			m_szZoomPercent,
			m_szCoords
			);

		SetWindowText(m_hWnd, wbuffer);
	}
}


void CTextureViewer::ToggleFileList(void)
{
	SetShowFileList(!m_bShowFileList, false);
}


void CTextureViewer::ToggleFileSettings(void)
{
	SetShowFileSettings(!m_bShowFileSettings, false);
}


void CTextureViewer::ApplySettings(bool bInit)
{
	SetBackgroundColor(g_clrBackground, false, bInit);
	if ( bInit )
		SetChannelMode(g_eDefChannelMode == -1 ? g_eChannelMode : g_eDefChannelMode, false, bInit);
	SetAlphaColor(g_clrAlpha, false, bInit);
	SetAlphaOpacity(g_iAlphaOpacity, false, bInit);
	SetShowBorder(g_bShowBorder, false, bInit);
	SetBorderColor(g_clrBorder, false, bInit);
	if ( bInit )
		SetTileMode(g_eDefTileMode == -1 ? g_eTileMode : g_eDefTileMode, false, bInit);
	SetNumTiles(g_nTileSteps, false, bInit);
	SetMinFilter(g_eMinFilter, false, bInit);
	SetMagFilter(g_eMagFilter, false, bInit);
	SetProjection(g_eProjection, false, bInit);
	SetClipImage(g_bClipImage, bInit);
	SetSoftZoom(g_bSoftZoom, bInit);
	SetWrapAround(g_bWrapAround, bInit);
	SetAutoZoom(g_bAutoZoom, bInit);
	SetResetZoom(g_bResetZoom, bInit);
	SetResetSubItems(g_bResetSubItems, bInit);
	SetDefChannelMode(g_eDefChannelMode, bInit);
	SetDefTileMode(g_eDefTileMode, bInit);
	SetDefGamma(g_eDefGamma, bInit);
	SetPrecacheFiles(g_bPrecacheFiles, bInit);

	SetShowMenu(g_bShowMenu, bInit);
	SetShowToolbar(g_bShowToolbar, bInit);
	SetShowStatus(g_bShowStatus, bInit);
}


void CTextureViewer::DumpSettings(void)
{
	g_clrBackground = m_clrBackground;
	g_eChannelMode = m_eChannelMode;
	g_clrAlpha = m_clrAlpha;
	g_iAlphaOpacity = m_iAlphaOpacity;
	g_bShowBorder = m_bShowBorder;
	g_clrBorder = m_clrBorder;
	g_eTileMode = m_eTileMode;
	g_nTileSteps = m_nTileSteps;
	g_eProjection = m_eProjection;
	g_eMinFilter = m_eMinFilter;
	g_eMagFilter = m_eMagFilter;
	g_bClipImage = m_bClipImage;
	g_bSoftZoom = m_bSoftZoom;
	g_bWrapAround = m_bWrapAround;
	g_bAutoZoom = m_bAutoZoom;
	g_bResetZoom = m_bResetZoom;
	g_bResetSubItems = m_bResetSubItems;
	g_eDefChannelMode = m_eDefChannelMode;
	g_eDefTileMode = m_eDefTileMode;
	g_eDefGamma = m_eDefGamma;
	g_bPrecacheFiles = m_bPrecacheFiles;

	g_bShowMenu = m_bShowMenu;
	g_bShowToolbar = m_bShowToolbar;
	g_bShowStatus = m_bShowStatus;
	g_bShowFileList = m_bShowFileList;
	g_bShowFileSettings = m_bShowFileSettings;
}


void CTextureViewer::SetShowMenu(bool bShow, bool bInit)
{
	if ((m_bShowMenu != bShow) || bInit)
	{
		m_bShowMenu = bShow;
		//SetMenu(m_hWnd, (m_bShowMenu)? m_hMainMenu: NULL);
	}
}


void CTextureViewer::SetShowToolbar(bool bShow, bool bInit)
{
	if ((m_bShowToolbar != bShow) || bInit)
	{
		m_bShowToolbar = bShow;
		ShowWindow(m_hToolbar, (m_bShowToolbar)? SW_SHOWNORMAL: SW_HIDE);
	}
}


void CTextureViewer::SetShowStatus(bool bShow, bool bInit)
{
	if ((m_bShowStatus != bShow) || bInit)
	{
		m_bShowStatus = bShow;
		ShowWindow(m_hStatus, (m_bShowStatus)? SW_SHOWNORMAL: SW_HIDE);
	}
}


void CTextureViewer::SetShowFileList(bool bShow, bool bInit)
{
	if ((m_bShowFileList != bShow) || bInit)
	{
		if (!bShow && !bInit)
		{
			// save the pos
			RECT rect;

			if (GetWindowRect(g_pFileList->GetWnd(), &rect))
			{
				g_iFileListLeft = rect.left;
				g_iFileListTop = rect.top;
				g_iFileListWidth = rect.right - rect.left;
				g_iFileListHeight = rect.bottom - rect.top;
			}
		}

		m_bShowFileList = bShow;
		ShowWindow(g_pFileList->GetWnd(), (m_bShowFileList)? SW_SHOWNOACTIVATE: SW_HIDE);
		CheckToolbarButtons(CMD_FILE_LIST, m_bShowFileList);
	}
}


void CTextureViewer::SetShowFileSettings(bool bShow, bool bInit)
{
	if ((m_bShowFileSettings != bShow) || bInit)
	{
		if (!bShow && !bInit)
		{
			// save the pos
			RECT rect;

			if (GetWindowRect(g_pFileSettings->GetWnd(), &rect))
			{
				g_iProcessingLeft = rect.left;
				g_iProcessingTop = rect.top;
				g_iProcessingWidth = rect.right - rect.left;
				g_iProcessingHeight = rect.bottom - rect.top;
			}
		}

		m_bShowFileSettings = bShow;
		ShowWindow(g_pFileSettings->GetWnd(), (m_bShowFileSettings)? SW_SHOWNOACTIVATE: SW_HIDE);
		CheckToolbarButtons(CMD_FILE_SETTINGS, m_bShowFileSettings);
	}
}


void CTextureViewer::SetBackgroundColor(COLORREF clr, bool bRedraw, bool bInit)
{
	if ((m_clrBackground != clr) || bInit)
	{
		m_clrBackground = clr;
		m_pViewport->SetBackgroundColor(m_clrBackground);
		if (bRedraw)
		{
			m_pViewport->Update();
		}
	}
}


void CTextureViewer::SetChannelMode(int eMode, bool bRedraw, bool bInit)
{
	if ((m_eChannelMode != eMode) || bInit)
	{
		m_eChannelMode = eMode;
		m_pViewport->SetChannelMode(m_eChannelMode);
		if (bRedraw)
		{
			m_pViewport->Update();
		}
		m_bUpdateChannelMode = true;
		UpdateInterface();
	}
}


void CTextureViewer::SetAlphaColor(COLORREF clr, bool bRedraw, bool bInit)
{
	if ((m_clrAlpha != clr) || bInit)
	{
		m_clrAlpha = clr;
		m_pViewport->SetAlphaColor(m_clrAlpha);
		if (bRedraw)
		{
			m_pViewport->Update();
		}
	}
}


void CTextureViewer::SetAlphaOpacity(int iOpacity, bool bRedraw, bool bInit)
{
	if ((m_iAlphaOpacity != iOpacity) || bInit)
	{
		m_iAlphaOpacity = iOpacity;
		m_pViewport->SetAlphaOpacity(m_iAlphaOpacity);
		if (bRedraw)
		{
			m_pViewport->Update();
		}
	}
}


void CTextureViewer::SetShowBorder(bool bShow, bool bRedraw, bool bInit)
{
	if ((m_bShowBorder != bShow) || bInit)
	{
		m_bShowBorder = bShow;
		m_pViewport->SetShowBorder(m_bShowBorder);
		if (bRedraw)
		{
			m_pViewport->Update();
		}
	}
}


void CTextureViewer::SetBorderColor(COLORREF clr, bool bRedraw, bool bInit)
{
	if ((m_clrBorder != clr) || bInit)
	{
		m_clrBorder = clr;
		m_pViewport->SetBorderColor(m_clrBorder);
		if (bRedraw)
		{
			m_pViewport->Update();
		}
	}
}


void CTextureViewer::UpdateTiling(void)
{
	switch (m_eTileMode)
	{
	case TM_SINGLE_TILE:
	default:
		m_pViewport->SetNumTilesHorz(1);
		m_pViewport->SetNumTilesVert(1);
		break;
	case TM_TILE_HORZVERT:
		m_pViewport->SetNumTilesHorz(1+m_nTileSteps*2);
		m_pViewport->SetNumTilesVert(1+m_nTileSteps*2);
		break;
	case TM_TILE_HORZ:
		m_pViewport->SetNumTilesHorz(1+m_nTileSteps*2);
		m_pViewport->SetNumTilesVert(1);
		break;
	case TM_TILE_VERT:
		m_pViewport->SetNumTilesHorz(1);
		m_pViewport->SetNumTilesVert(1+m_nTileSteps*2);
		break;
	}

	UpdateZoomLevel();
}


void CTextureViewer::SetTileMode(int eMode, bool bRedraw, bool bInit)
{
	if ((m_eTileMode != eMode) || bInit)
	{
		m_eLastTileMode = m_eTileMode;
		m_eTileMode = eMode;
		m_pViewport->SetTileMode(m_eTileMode);
		UpdateTiling();
		if (bRedraw)
		{
			m_pViewport->Update();
		}
		m_bUpdateTileMode = true;
		UpdateInterface();
	}
}


void CTextureViewer::SetNumTiles(int nTiles, bool bRedraw, bool bInit)
{
	if ((m_nTileSteps != nTiles) || bInit)
	{
		m_nTileSteps = nTiles;
		UpdateTiling();
		if (bRedraw)
		{
			m_pViewport->Update();
		}
	}
}


void CTextureViewer::SetProjection(int eProjection, bool bRedraw, bool bInit)
{
	if ((m_eProjection != eProjection) || bInit)
	{
		m_eProjection = eProjection;
		// TODO:
		//m_pViewport->SetProjection(m_eProjection);
		if (bRedraw)
		{
			m_pViewport->Update();
		}
		m_bUpdateProjection = true;
		UpdateInterface();
	}
}


void CTextureViewer::SetMinFilter(int eFilter, bool bRedraw, bool bInit)
{
	if ((m_eMinFilter != eFilter) || bInit)
	{
		m_eMinFilter = eFilter;
		m_pViewport->SetMinFilter(m_eMinFilter);
		if (bRedraw)
		{
			m_pViewport->Update();
		}
	}
}


void CTextureViewer::SetMagFilter(int eFilter, bool bRedraw, bool bInit)
{
	if ((m_eMagFilter != eFilter) || bInit)
	{
		m_eMagFilter = eFilter;
		m_pViewport->SetMagFilter(m_eMagFilter);
		if (bRedraw)
		{
			m_pViewport->Update();
		}
	}
}


void CTextureViewer::SetClipImage(bool bClip, bool bInit)
{
	if ((m_bClipImage != bClip) || bInit)
	{
		m_bClipImage = bClip;
		m_pViewport->SetClipImage(m_bClipImage);
	}
}

/*
void CTextureViewer::SetClipImageExtent(int iExtent, bool bInit)
{
	if ((m_iClipImageExtent != iExtent) || bInit)
	{
		m_iClipImageExtent = iExtent;
		m_pViewport->SetClipImageExtent(m_iClipImageExtent);
	}
}
*/

void CTextureViewer::SetSoftZoom(bool bSoftZoom, bool bInit)
{
	if ((m_bSoftZoom != bSoftZoom) || bInit)
	{
		m_bSoftZoom = bSoftZoom;
		UpdateZoomLevel();
	}
}


void CTextureViewer::SetWrapAround(bool b, bool bInit)
{
	if ((m_bWrapAround != b) || bInit)
	{
		m_bWrapAround = b;
	}
}


void CTextureViewer::SetAutoZoom(bool b, bool bInit)
{
	if ((m_bAutoZoom != b) || bInit)
	{
		m_bAutoZoom = b;
	}
}


void CTextureViewer::SetResetZoom(bool b, bool bInit)
{
	if ((m_bResetZoom != b) || bInit)
	{
		m_bResetZoom = b;
	}
}


void CTextureViewer::SetResetSubItems(bool b, bool bInit)
{
	if ((m_bResetSubItems != b) || bInit)
	{
		m_bResetSubItems = b;
	}
}


void CTextureViewer::SetDefChannelMode(int eMode, bool bInit)
{
	if ((m_eDefChannelMode != eMode) || bInit)
	{
		m_eDefChannelMode = eMode;
	}
}


void CTextureViewer::SetDefTileMode(int eMode, bool bInit)
{
	if ((m_eDefTileMode != eMode) || bInit)
	{
		m_eDefTileMode = eMode;
	}
}


void CTextureViewer::SetDefGamma(int eGamma, bool bInit)
{
	if ((m_eDefGamma != eGamma) || bInit)
	{
		m_eDefGamma = eGamma;
		if (!bInit)
		{
			//XXX: should reload the file?
			//UpdateImage();
		}
	}
}


void CTextureViewer::SetPrecacheFiles(bool b, bool bInit)
{
	if ((m_bPrecacheFiles != b) || bInit)
	{
		m_bPrecacheFiles = b;
	}
}

//
//
//


void CTextureViewer::ResetSize(void)
{
    SetZoomLevel(0);
}


void CTextureViewer::ResetOrigin(void)
{
	m_pViewport->ResetOrigin();
}


// XXX:
float g_aflSoftZoom[] =
{
	0.25f, 0.33f, 0.5f, 0.66f, 0.75f, 1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 6.0f, 10.0f
};


int ApplySoftZoom(int iValue, int iLevel)
{
	float fl;

	fl = iValue * g_aflSoftZoom[MAX_ZOOM_LEVEL+iLevel];

	return (int)fl;
}


void CTextureViewer::UpdateZoomLevel(void)
{
    // XXX:
	if (IsValidImage())
	{
		POINT ptSizeNew;
		int iWidth = GetImageWidth();
		int iHeight = GetImageHeight();

		if (!m_bSoftZoom)
		{
			ptSizeNew.x = RaiseToLevel(iWidth, m_iZoomLevel);
			ptSizeNew.y = RaiseToLevel(iHeight, m_iZoomLevel);
		}
		else
		{
			ptSizeNew.x = ApplySoftZoom(iWidth, m_iZoomLevel);
			ptSizeNew.y = ApplySoftZoom(iHeight, m_iZoomLevel);
		}

		if (m_eTileMode == TM_TILE_HORZVERT)
		{
			ptSizeNew.x *= 1+m_nTileSteps*2;
			ptSizeNew.y *= 1+m_nTileSteps*2;
		}
		else if (m_eTileMode == TM_TILE_HORZ)
		{
			ptSizeNew.x *= 1+m_nTileSteps*2;
		}
		else if (m_eTileMode == TM_TILE_VERT)
		{
			ptSizeNew.y *= 1+m_nTileSteps*2;
		}

		m_pViewport->Zoom(&ptSizeNew);
	}

	//m_bUpdateZoomLevel = true;
}



void CTextureViewer::SetPickingColor( bool bPicking )
{
	m_pViewport->SetColorPicking( bPicking );
}


void CTextureViewer::PickColor( bool bValid, Rect_t* prect )
{
	if ( bValid )
	{
		// do some useful stuff
		g_pFileSettings->DoAnalysis( prect );
	}
}


void CTextureViewer::FinishColorPicking( void )
{
	m_pViewport->SetColorPicking( false );
	g_pFileSettings->SetPickingColor( false );
}


void CTextureViewer::SetCoords(bool bValid, int x, int y)
{
	if (bValid)
	{
		if (!m_bCoords || ((m_xCoords != x) || (m_yCoords != y)))
		{
			m_xCoords = x;
			m_yCoords = y;
			m_bCoords = bValid;

			m_bUpdateCoords = true;

			UpdateInterface();
		}
	}
	else
	{
		if (m_bCoords)
		{
			m_bCoords = bValid;
			
			m_bUpdateCoords = true;

			UpdateInterface();
		}
	}
}



void CTextureViewer::WM_Close(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	RECT rect;

	// here we dump the settings to globals

	// main window
	if (!IsIconic(m_hWnd))
	{
		g_bWindowMaximized = (IsZoomed(m_hWnd) == TRUE);

		if (!g_bWindowMaximized)
		{
			if (GetWindowRect(m_hWnd, &rect))
			{
				g_iWindowLeft = rect.left;
				g_iWindowTop = rect.top;
				g_iWindowWidth = rect.right - rect.left;
				g_iWindowHeight = rect.bottom - rect.top;
			}
		}
	}

	// floaters
	if (GetWindowRect(g_pFileList->GetWnd(), &rect))
	{
		g_iFileListLeft = rect.left;
		g_iFileListTop = rect.top;
		g_iFileListWidth = rect.right - rect.left;
		g_iFileListHeight = rect.bottom - rect.top;
	}
	if (GetWindowRect(g_pFileSettings->GetWnd(), &rect))
	{
		g_iProcessingLeft = rect.left;
		g_iProcessingTop = rect.top;
		g_iProcessingWidth = rect.right - rect.left;
		g_iProcessingHeight = rect.bottom - rect.top;
	}

	// process settings that are stored in *this
	DumpSettings();
}


void CTextureViewer::WM_Size(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	RECT rectBar;

	SendMessage(m_hStatus, WM_SIZE, 0, 0);
    SendMessage(m_hToolbar, TB_AUTOSIZE, 0, 0);

	GetClientRect(hWnd, &rect);

	if (m_bShowToolbar)
	{
		GetWindowRect(m_hToolbar, &rectBar);
		rect.top += (rectBar.bottom - rectBar.top);
	}
	
	if (m_bShowStatus)
	{
		GetWindowRect(m_hStatus, &rectBar);
		rect.bottom -= (rectBar.bottom - rectBar.top);
	}
	
	SetWindowPos(m_hViewport, NULL, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, NULL);
}


void CTextureViewer::WM_MouseWheel(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	static int iPassed;
	int iDelta;
	int eCmd;
	int n;
	int i;

	iPassed += (short)HIWORD(wParam);
	iDelta = iPassed / 120;
	if (iDelta != 0)
	{
		iPassed -= iDelta * 120;
		eCmd = (iDelta > 0)? g_aiKeyEventCmd[TK_MWHEELDOWN]: g_aiKeyEventCmd[TK_MWHEELUP];
		if (eCmd != 0)
		{
			n = abs(iDelta);
			for (i = 0; i < n; i++)
			{
				SendMessage(hWnd, WM_COMMAND, eCmd, 0);
			}
		}
	}
}


void CTextureViewer::WM_LButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (GetKeyState(VK_SHIFT) < 0)
	{
		DragDrop();
	}
	else
	{
		m_pViewport->BeginScroll();
	}
}


void CTextureViewer::WM_LButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_pViewport->EndScroll();
}


void CTextureViewer::WM_MButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_pViewport->BeginScroll();
}


void CTextureViewer::WM_MButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_pViewport->EndScroll();
}


void CTextureViewer::WM_RButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	//
}


void CTextureViewer::WM_RButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	Shell();
}


void CTextureViewer::WM_KeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (g_aiKeyEventCmd[(DWORD)wParam] != 0)
	{
		PostMessage(hWnd, WM_COMMAND, (WPARAM)g_aiKeyEventCmd[(DWORD)wParam], 0);
	}
}


void CTextureViewer::WM_KeyUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
 //   if (g_aiKeyUpCmd[(DWORD)wParam] != 0)
	//{
	//	PostMessage(hWnd, WM_COMMAND, (WPARAM)g_aiKeyUpCmd[(DWORD)wParam], 0);
	//}
}


void CTextureViewer::SetZoomLevel(int iZoomLevel)
{
	m_iZoomLevel = max(MIN_ZOOM_LEVEL, min(MAX_ZOOM_LEVEL, iZoomLevel));

	UpdateZoomLevel();
	m_bUpdateZoomLevel = true;

	UpdateInterface();
}


void CTextureViewer::ChangeZoomLevel(int iChange)
{
	SetZoomLevel(m_iZoomLevel + iChange);
}


int CTextureViewer::GetNumFiles(void)
{
	return g_pContext->GetNumFiles();
}


int CTextureViewer::GetFile(void)
{
	return g_pContext->GetFile();
}


const wchar_t* CTextureViewer::GetFileName(void)
{
	if (GetFile() == -1)
	{
		return L"Logo";
	}
	
	return g_pContext->GetFileName(GetFile());
}


const char* CTextureViewer::GetImageFormatStr(void)
{
	return m_pszImageFormatString;
}


int CTextureViewer::GetNumTextures(void)
{
	return m_nTextures;
}


int CTextureViewer::GetTexture(void)
{
	return m_iTexture;
}


int CTextureViewer::GetNumMIPMaps(void)
{
	return m_nMIPMaps;
}


int CTextureViewer::GetMIPMap(void)
{
	return m_iMIPMap;
}


int CTextureViewer::GetNumSlices(void)
{
	return m_nSlices;
}


int CTextureViewer::GetSlice(void)
{
	return m_iSlice;
}


int CTextureViewer::GetImageWidth(void)
{
	return m_iImageWidth;
}


int CTextureViewer::GetImageHeight(void)
{
	return m_iImageHeight;
}


void __cdecl CTextureViewer::ReadPixelsStatic(void* buffer, void* param)
{
	((CTextureViewer*)param)->ReadPixels(buffer);
}


void CTextureViewer::ReadPixels(void* buffer)
{
	/*
	if (m_pimg != NULL)
	{
		//int iPitch = PIXEL_SIZE * m_iImageWidth;
		//int iSliceSize = iPitch * m_iImageHeight;
		//unsigned int iOffset = (iSliceSize * m_iDepthLevel) + (iPitch * iStartLine);
		//unsigned int iSize = iPitch * nLines;

		//SYS_ReadMemory(m_hMIPMap, iOffset, iSize, buffer);
	}
	*/
	g_pContext->GetFile(GetFile())->GetImageData(buffer);
}


void GetDefaultGamma( Gamma_t* pcs )
{
	// TODO:
	pcs->eGamma = GM_SRGB;
}


void CTextureViewer::UpdatePixels(void)
{
	m_pViewport->UpdatePixels( 0 );
	m_pViewport->Update();
}


void CTextureViewer::UpdateImage(void)
{
	if (IsValidImage())
	{
		ImageParams_t* params = ( ImageParams_t* )g_pContext->GetOutputParams();
		m_iImageWidth = params->iImageWidth;
		m_iImageHeight = params->iImageHeight;
		m_pViewport->SetImage(m_iImageWidth, m_iImageHeight, params->flags, NULL, true);

		UpdateZoomLevel();
		m_bUpdateImageSize = true;
	}
	// XXX: should be already reset?
	else
	{
		m_iImageWidth = 0;
		m_iImageHeight = 0;
		m_pViewport->SetImage(0, 0, 0, NULL, false);
	}

	//SetCursor(LoadCursor(NULL, IDC_WAIT));
	m_pViewport->Update();
	//SetCursor(LoadCursor(NULL, IDC_ARROW));
}


bool CTextureViewer::IsValidImage(void)
{
	return m_bValidImage;
}


void CTextureViewer::UpdateSlice( void )
{
	if ( IsValidMIPMap() )
	{
		m_bValidImage = g_pContext->GetFile( GetFile() )->IsValidImage();
	}
	else
	{
		m_bValidImage = false;
	}

	UpdateImage();
	m_bUpdateSlice = true;
}


void CTextureViewer::SetSlice(int iSlice)
{
	if (IsValidMIPMap())
	{
		iSlice = min(m_nSlices - 1, max(0, iSlice));

		g_pContext->GetFile(GetFile())->SetSlice( iSlice );
		m_iSlice = g_pContext->GetFile( GetFile() )->GetSlice();

		UpdateSlice();

		UpdateInterface();
	}
}


void CTextureViewer::ChangeSlice(int iChange)
{
    int i;

    i = GetSlice() + iChange;
	/*
    if (i < 0)
	{
        i = m_nSlices - 1;
    }

    if (i > m_nSlices - 1)
	{
        i = 0;
    }
	*/
    SetSlice(i);
}


void CTextureViewer::UpdateMIPMap( void )
{
	if ( IsValidTexture() )
	{
		m_bValidMIPMap = g_pContext->GetFile( GetFile() )->IsValidImage();
	}
	else
	{
		m_bValidMIPMap = false;
	}

	if ( IsValidMIPMap() )
	{
		CFile* pFile = g_pContext->GetFile( GetFile() );
		m_pszImageFormatString = pFile->GetImageFormatStr();
		m_nSlices = pFile->GetNumSlices();
		m_iSlice = pFile->GetSlice();
	}
	else
	{
		m_pszImageFormatString = NULL;
		m_nSlices = 0;
		m_iSlice = -1;
	}

	UpdateSlice();
	UpdateZoomLevel(); // must be here since the dimensions below this level never change
	m_bUpdateMIPMap = true;
}


void CTextureViewer::SetMIPMap( int iMIPMap )
{
	if ( IsValidTexture() )
	{
		iMIPMap = max(0, min(GetNumMIPMaps() - 1, iMIPMap));

		g_pContext->GetFile( GetFile() )->SetMIPMap( iMIPMap );
		m_iMIPMap = g_pContext->GetFile( GetFile() )->GetMIPMap();

		// the input size has probably changed...
		g_pContext->UpdateInputParams( false, false );

		UpdateMIPMap();

		UpdateInterface();
	}
}


void CTextureViewer::ChangeMIPMap( int iChange )
{
	SetMIPMap( GetMIPMap() + iChange );
}


bool CTextureViewer::IsValidMIPMap(void)
{
	return m_bValidMIPMap;
}


void CTextureViewer::UpdateTexture( void )
{
	if ( IsValidFile() )
	{
		m_bValidTexture = g_pContext->GetFile( GetFile() )->IsValidImage();
	}
	else
	{
		m_bValidTexture = false;
	}

	if ( IsValidTexture() )
	{
		CFile* pFile = g_pContext->GetFile( GetFile() );
		m_nMIPMaps = pFile->GetNumMIPMaps();
		m_iMIPMap = pFile->GetMIPMap();
	}
	else
	{
		m_nMIPMaps = 0;
		m_iMIPMap = -1;
	}

	UpdateMIPMap();
	m_bUpdateTexture = true;
}


void CTextureViewer::SetTexture( int iTexture )
{
	if ( IsValidFile() )
	{
		iTexture = max(0, min(GetNumTextures() - 1, iTexture));

		g_pContext->GetFile( GetFile() )->SetTexture( iTexture );
		m_iTexture = g_pContext->GetFile( GetFile() )->GetTexture();

		// the input size has probably changed, so call this
		g_pContext->UpdateInputParams( false, false );

		UpdateTexture();

		UpdateInterface();
	}
}


void CTextureViewer::ChangeTexture( int iChange )
{
	SetTexture( GetTexture() + iChange );
}


bool CTextureViewer::IsValidTexture(void)
{
	return m_bValidTexture;
}


void CTextureViewer::UpdateFile( void )
{
	if ( GetFile() != -1 )
	{
		m_bValidFile = g_pContext->GetFile( GetFile() )->IsValidFile();
	}
	else
	{
		m_bValidFile = false;
	}

	if ( IsValidFile() )
	{
		CFile* pFile = g_pContext->GetFile( GetFile() );
	
		m_nTextures = pFile->GetNumTextures();
		m_iTexture = pFile->GetTexture();
	}
	else
	{
		m_nTextures = 0;
		m_iTexture = -1;
	}

	UpdateTexture();

	m_bUpdateFile = true;
}


void CTextureViewer::SetFile( int iFile )
{
	//SetCursor(LoadCursor(NULL, IDC_WAIT));

	// XXX
	m_pViewport->ResetImage(); // we reset it here to avoid the fragmentation of the address space

    iFile = min(GetNumFiles() - 1, max(0, iFile));
    if (iFile == -1)
	{
	//	__DEBUG_BREAK;
	}

	// XXX:
	g_pContext->SetFile(iFile);

	UpdateFile();

	if (m_bAutoZoom)
	{
		if (iFile != -1)
		{
			if (IsValidImage())
			{
				// pick an appropriate zoom level
				RECT rect;

				m_pViewport->GetViewportRect(&rect);

				int iViewportWidth;
				int iViewportHeight;

				RectWidthHeight(&rect, &iViewportWidth, &iViewportHeight);

				int i;

				for (i = 0; i >= MIN_ZOOM_LEVEL; i--)
				{
					int iWidth = RaiseToLevel(m_iImageWidth, i);
					int iHeight = RaiseToLevel(m_iImageHeight, i);
					
					if ((iWidth <= iViewportWidth) && (iHeight <= iViewportHeight))
					{
						break;
					}
				}

				SetZoomLevel(i);
			}
		}
	}

	UpdateInterface();
	//SetCursor(LoadCursor(NULL, IDC_ARROW));
}


void CTextureViewer::ChangeFile(int iChange)
{
	int i;

	i = GetFile() + iChange;

	if (i < 0)
	{
		if (m_bWrapAround)
		{
			i = GetNumFiles() - 1;
		}
		else
		{
			i = 0;
		}
	}

	if (i > GetNumFiles() - 1)
	{
		if (m_bWrapAround)
		{
			i = 0;
		}
		else
		{
			i = GetNumFiles() - 1;
		}
	}

	SetFile( i );
}


bool CTextureViewer::IsValidFile(void)
{
	return m_bValidFile;
}


bool CTextureViewer::SetSource(wchar_t* pszPath, wchar_t* pszFilter)
{
	wchar_t szDir[MAX_PATH];
	wchar_t* pszFileName;

	wcscpy(szDir, pszPath);
	pszFileName = ::GetFileName(szDir);
	if (pszFileName == szDir)
		__DEBUG_BREAK;
	pszFileName[-1] = '\0';

	if ( g_pContext->SetSourcePath(szDir) )
	{
		g_pContext->PopulateSourceList(pszFilter);

		int nFiles = GetNumFiles();

		if (nFiles != 0)
		{
			int iFile = 0;
			for (int i = 0; i < nFiles; i++)
			{
				if (FWStrEq(pszFileName, g_pContext->GetFileName(i)))
				{
					iFile = i;
					break;
				}
			}

			SetFile(iFile);

			return true;
		}
	}

	return false;
}


bool ShowOpenDialog(HWND hWnd, wchar_t* pszFileNameBuffer);

bool CTextureViewer::ChooseSource(void)
{
	wchar_t szFileName[MAX_PATH];
	szFileName[0] = '\0';
	wchar_t* pszFilter; // XXX: filters won't work as we have multiple extensions per format
	if (ShowOpenDialog(m_hWnd, szFileName))
	{
		return SetSource(szFileName, NULL);
	}

	return false;
}


#include "targa.h"
void CTextureViewer::SaveTGA(void)
{
	static wchar_t szFileName[MAX_PATH];
	OPENFILENAME ofn;

	if ( !IsValidImage() )
	{
		return;
	}

	const wchar_t* pszFileName = GetFileName();
	if (!pszFileName)
	{
		return;
	}

	wcscpy(szFileName, pszFileName);
	SetExtension(szFileName, L"tga");

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.hInstance = g_hInst;
	ofn.lpstrFilter = L"Targa (*.tga)\0*.tga\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn))
	{
		/*
		wchar_t* pszExt = GetExtension(szFileName);
		if (pszExt == NULL)
		{
			wcscat(szFileName, L".tga");
		}
		*/
		FILE* stream = _wfopen(szFileName, L"wb");
		if (stream != NULL)
		{
			int iWidth = m_iImageWidth;
			int iHeight = m_iImageHeight;
			int iScanSize = iWidth*4;
			int iImageSize = iScanSize*iHeight;
			BYTE* data = (BYTE*)malloc(iImageSize);
			if (data != NULL)
			{
				tga_file_header_t fh;
				memset(&fh, 0, sizeof(tga_file_header_t));
				fh.iIDLen = 0;
				fh.eColorMapType = 0;
				fh.eImageType = TGA_RGB;
				fh.cm.iStart = 0;
				fh.cm.iLength = 0;
				fh.cm.iEntrySize = 0;
				fh.img.xOrigin = 0;
				fh.img.yOrigin = 0;
				fh.img.iWidth = iWidth;
				fh.img.iHeight = iHeight;
				fh.img.iPixelSize = 32;
				int nAlphaBits = 8;
				fh.img.flags = nAlphaBits | TGA_INVERSETOPBOTTOM; // the lower 3 bits are the alpha bit depth
				fwrite(&fh, sizeof(tga_file_header_t), 1, stream);

				ReadPixels(data);
				fwrite(data, iImageSize, 1, stream);

				free(data);
			}

			fclose(stream);
		}
	}
}


void CTextureViewer::SaveBMP(void)
{
	static wchar_t szFileName[MAX_PATH];
	OPENFILENAME ofn;

	if ( !IsValidImage() )
	{
		return;
	}

	const wchar_t* pszFileName = GetFileName();
	if (!pszFileName)
	{
		return;
	}

	wcscpy(szFileName, pszFileName);
	SetExtension(szFileName, L"bmp");

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.hInstance = g_hInst;
	ofn.lpstrFilter = L"Windows Bitmap (*.bmp)\0*.bmp\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn))
	{
		/*
		wchar_t* pszExt = GetExtension(szFileName);
		if (pszExt == NULL)
		{
			wcscat(szFileName, L".bmp");
		}
		*/

		int iWidth = m_iImageWidth;
		int iHeight = m_iImageHeight;
		int iScanSize = iWidth*4;
		int iImageSize = iScanSize*iHeight;
		BYTE* data = (BYTE*)malloc(iImageSize);
		if (data != NULL)
		{
			ReadPixels(data);

			SaveBitmapW( szFileName, iWidth, iHeight, 4, iScanSize, data, 0, NULL );

			free(data);
		}
	}
}


wchar_t g_szSaveFileName[MAX_PATH];

void CTextureViewer::LoadProject(void)
{
	OPENFILENAME ofn;

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.hInstance = g_hInst;
	ofn.lpstrFilter = L"Project Files (*.TextureViewerProject)\0*.TextureViewerProject\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = g_szSaveFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

	if (GetOpenFileName(&ofn))
	{
		g_pContext->Load(g_szSaveFileName);
		UpdateFile();
	}
}


void CTextureViewer::SaveProject(void)
{
	OPENFILENAME ofn;

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.hInstance = g_hInst;
	ofn.lpstrFilter = L"Project Files (*.TextureViewerProject)\0*.TextureViewerProject\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = g_szSaveFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;

	if (GetSaveFileName(&ofn))
	{
		wchar_t* pszExt = GetExtension(g_szSaveFileName);
		if (pszExt == NULL)
		{
			wcscat(g_szSaveFileName, L".TextureViewerProject");
		}

		g_pContext->Save(g_szSaveFileName);
	}
}


void CTextureViewer::ShowAssocDialog(void)
{
	ShowWindow(g_hAssocDlg, SW_SHOWNORMAL);
}



void CTextureViewer::ShowSettingsDialog(void)
{
	DumpSettings();

	if (DialogBox(NULL, MAKEINTRESOURCE(IDD_SETTINGS), m_hWnd, SettingsDlgProc) == IDOK)
	{
		ApplySettings(false);
		m_pViewport->Update();
	}
}



BOOL CALLBACK AboutDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ( uMsg == WM_COMMAND)
	{
		if (HIWORD(wParam) == BN_CLICKED)
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				EndDialog( hWnd, 0 );
				break;
			}
			return 1;
		}
	}
	else if ( uMsg == WM_INITDIALOG )
	{
		static wchar_t* szAbout = 
			L"The Texture Viewer Project\n"
			L"Copyright (c) 2011-2024 Ilya Lyutin, http://imagetools.itch.io\n";

		SendDlgItemMessage( hWnd, IDC_ABOUT, WM_SETTEXT, NULL, (LPARAM)szAbout );
		static wchar_t* szVersion = L"Version 0.9.1 (June 2024)";
		SendDlgItemMessage( hWnd, IDC_ABOUT_VERSION, WM_SETTEXT, NULL, (LPARAM)szVersion );
		return 1;
	}

	return 0;
}

void CTextureViewer::ShowAboutDialog(void)
{
	DialogBox(NULL, MAKEINTRESOURCE(IDD_ABOUT), m_hWnd, AboutDlgProc);
}


// *******************************************
// this code was taken from Raymond Chen's tutorial on IContextMenu
// https://blogs.msdn.microsoft.com/oldnewthing/20040920-00/?p=37823/

HRESULT GetUIObjectOfFile(HWND hwnd, LPCWSTR pszPath, REFIID riid, void **ppv)
{
  *ppv = NULL;
  HRESULT hr;
  LPITEMIDLIST pidl;
  SFGAOF sfgao;
  if (SUCCEEDED(hr = SHParseDisplayName(pszPath, NULL, &pidl, 0, &sfgao))) {
    IShellFolder *psf;
    LPCITEMIDLIST pidlChild;
    if (SUCCEEDED(hr = SHBindToParent(pidl, IID_IShellFolder,
                                      (void**)&psf, &pidlChild))) {
      hr = psf->GetUIObjectOf(hwnd, 1, &pidlChild, riid, NULL, ppv);
      psf->Release();
    }
    CoTaskMemFree(pidl);
  }
  return hr;
}


#define SCRATCH_QCM_FIRST 1
#define SCRATCH_QCM_LAST  0x6FFF
void CTextureViewer::Shell(void)
{
	IContextMenu *pcm;
	wchar_t szPath[MAX_PATH];

	if (IsValidFile())
	{
		wsprintf(szPath, L"%s\\%s", g_pContext->GetSourcePath(), GetFileName());
	}
	else
	{
		return;
	}


	if (SUCCEEDED(GetUIObjectOfFile(m_hWnd, szPath, IID_IContextMenu, (void**)&pcm)))
	{	
		HMENU hmenu = CreatePopupMenu();
		
		if (hmenu)
		{
			if (SUCCEEDED(pcm->QueryContextMenu(hmenu, 0, SCRATCH_QCM_FIRST, SCRATCH_QCM_LAST, CMF_NORMAL)))
			{
				POINT pt;
				GetCursorPos(&pt);
				//ScreenToClient(m_hWnd, &pt);

				// initializes m_pcm2, so that our window procedure can process messages for it
				// see HandleShellMenuMsg()
				pcm->QueryInterface(IID_IContextMenu2, (void**)&m_pcm2);

				int iCmd = TrackPopupMenuEx(hmenu, TPM_RETURNCMD, pt.x, pt.y, m_hWnd, NULL);

				// uninit it, we don't want the menu messages outside the menu
				if (m_pcm2)
				{
					m_pcm2->Release();
					m_pcm2 = NULL;
				}

				if (iCmd > 0)
				{
					CMINVOKECOMMANDINFOEX info = { 0 };
					info.cbSize = sizeof(info);
					info.fMask = CMIC_MASK_UNICODE | CMIC_MASK_PTINVOKE;
					info.hwnd = m_hWnd;
					info.lpVerb  = MAKEINTRESOURCEA(iCmd - SCRATCH_QCM_FIRST);
					info.lpVerbW = MAKEINTRESOURCEW(iCmd - SCRATCH_QCM_FIRST);
					info.nShow = SW_SHOWNORMAL;
					info.ptInvoke = pt;
					pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)&info);
				}
			}

			DestroyMenu(hmenu);
		}

		pcm->Release();
	}
}


class CDropSource : public IDropSource
{
public:
  // *** IUnknown ***
  STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();

  // *** IDropSource ***
  STDMETHODIMP QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
  STDMETHODIMP GiveFeedback(DWORD dwEffect);

  CDropSource() : m_cRef(1) { }
private:
  ULONG m_cRef;
};

HRESULT CDropSource::QueryInterface(REFIID riid, void **ppv)
{
  IUnknown *punk = NULL;
  if (riid == IID_IUnknown) {
    punk = static_cast<IUnknown*>(this);
  } else if (riid == IID_IDropSource) {
    punk = static_cast<IDropSource*>(this);
  }

  *ppv = punk;
  if (punk) {
    punk->AddRef();
    return S_OK;
  } else {
    return E_NOINTERFACE;
  }
}


ULONG CDropSource::AddRef()
{
  return ++m_cRef;
}


ULONG CDropSource::Release()
{
  ULONG cRef = --m_cRef;
  if (cRef == 0) delete this;
  return cRef;
}


HRESULT CDropSource::QueryContinueDrag(
          BOOL fEscapePressed, DWORD grfKeyState)
{
  if (fEscapePressed) return DRAGDROP_S_CANCEL;

  // [Update: missing paren repaired, 7 Dec]
  if (!(grfKeyState & (MK_LBUTTON | MK_RBUTTON)))
    return DRAGDROP_S_DROP;

  return S_OK;
}

HRESULT CDropSource::GiveFeedback(DWORD dwEffect)
{
  return DRAGDROP_S_USEDEFAULTCURSORS;
}


void CTextureViewer::DragDrop(void)
{
	wchar_t szPath[MAX_PATH];

	if (IsValidFile())
	{
		wsprintf(szPath, L"%s\\%s", g_pContext->GetSourcePath(), GetFileName());
	}
	else
	{
		return;
	}
	
	IDataObject *pdto;
  // In a real program of course
  // you wouldn't use a hard-coded path.
  // [comment added 11am because apparently some
  // people thought this wasn't self-evident.]
  if (SUCCEEDED(GetUIObjectOfFile(m_hWnd,
                    szPath,
		    IID_IDataObject, (void**)&pdto))) {
    IDropSource *pds = new CDropSource();
    if (pds) {
      DWORD dwEffect;
      DoDragDrop(pdto, pds, DROPEFFECT_COPY | DROPEFFECT_LINK,
                 &dwEffect);
      pds->Release();
    }
    pdto->Release();
  }
}


// *******************************************



void CTextureViewer::MarkFile(void)
{
	/*
	if (GetFile() != -1)
	{
		g_pFileList->SetMarked(GetFile(), !g_pFileList->IsMarked(GetFile()));
	}
	*/
	g_pFileList->ToggleSelectionMarked();
}


BOOL CTextureViewer::WM_Command(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
        case CMD_NULL:
			// nothing
            break;

		case CMD_CHAN_RGB:
			SetChannelMode(CM_RGB, true, false);
			break;
		case CMD_CHAN_RGBA:
			SetChannelMode(CM_RGBA, true, false);
			break;
		case CMD_CHAN_RED:
			SetChannelMode(CM_RED, true, false);
			break;
		case CMD_CHAN_GREEN:
			SetChannelMode(CM_GREEN, true, false);
			break;
		case CMD_CHAN_BLUE:
			SetChannelMode(CM_BLUE, true, false);
			break;
		case CMD_CHAN_ALPHA:
			SetChannelMode(CM_ALPHA, true, false);
			break;

        case CMD_SINGLE_TILE:
			SetTileMode((m_eTileMode == TM_SINGLE_TILE)? m_eLastTileMode: TM_SINGLE_TILE, true, false);
			//SetTileMode(TM_SINGLE_TILE, true, false);
            break;
        case CMD_TILE_HORZ:
			SetTileMode((m_eTileMode == TM_TILE_HORZ)? TM_SINGLE_TILE : TM_TILE_HORZ, true, false);
           // SetTileMode(TM_TILE_HORZ, true, false);
			break;
        case CMD_TILE_VERT:
			SetTileMode((m_eTileMode == TM_TILE_VERT)? TM_SINGLE_TILE : TM_TILE_VERT, true, false);
            //SetTileMode(TM_TILE_VERT, true, false);
			break;
        case CMD_TILE_HORZVERT:
			SetTileMode((m_eTileMode == TM_TILE_HORZVERT)? TM_SINGLE_TILE : TM_TILE_HORZVERT, true, false);
            //SetTileMode(TM_TILE_HORZVERT, true, false);
			break;

		// obsolete?
        case CMD_RESET_SIZE:
			ResetSize();
            break;
        case CMD_RESET_ORIGIN:
            ResetOrigin();
            break;

        case CMD_ZOOM_OUT:
            ChangeZoomLevel(-1);
            break;
        case CMD_ZOOM_IN:
            ChangeZoomLevel(1);
            break;

        case CMD_PROJ_XY:
            SetProjection(PROJ_XY, true, false);
            break;
        case CMD_PROJ_ZY:
            SetProjection(PROJ_ZY, true, false);
            break;
        case CMD_PROJ_XZ:
            SetProjection(PROJ_XZ, true, false);
            break;

        case CMD_PREV_SLICE:
			ChangeSlice(-1);
            break;
        case CMD_NEXT_SLICE:
            ChangeSlice(1);
            break;
        case CMD_PREV_MIPMAP:
            ChangeMIPMap(-1);
            break;
        case CMD_NEXT_MIPMAP:
            ChangeMIPMap(1);
            break;
        case CMD_PREV_TEXTURE:
            ChangeTexture(-1);
            break;
        case CMD_NEXT_TEXTURE:
            ChangeTexture(1);
            break;
        case CMD_PREV_FILE:
            ChangeFile(-1);
            break;
        case CMD_NEXT_FILE:
            ChangeFile(1);
            break;

		case CMD_BEGIN_SCROLL:
			m_pViewport->BeginScroll();
			break;
		case CMD_END_SCROLL:
			m_pViewport->EndScroll();
			break;

        case CMD_SHELL:
			Shell();
            break;

		case CMD_DRAGDROP:
			DragDrop();
			break;

		case CMD_MARK_FILE:
			MarkFile();
			break;

		case CMD_APPLY_FILE_SETTINGS:
			g_pFileSettings->ApplyFileSettings();
			break;

        case CMD_OPEN:
			ChooseSource();
            break;
        
		case CMD_SAVE_TGA:
			SaveTGA();
            break;
		case CMD_SAVE_BMP:
			SaveBMP();
            break;

        case CMD_EXIT:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
            break;

        case CMD_ASSOC:
			ShowAssocDialog();
            break;
        case CMD_SETTINGS:
			ShowSettingsDialog();
            break;

        case CMD_ABOUT:
			ShowAboutDialog();
            break;

		case CMD_FILE_LIST:
            ToggleFileList();
            break;
		case CMD_FILE_SETTINGS:
            ToggleFileSettings();
            break;

        default:

            return FALSE;
    }

    return TRUE;
}


bool CTextureViewer::HandleShellMenuMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// for the shell context menu
	if (m_pcm2 != NULL)
	{
		if (SUCCEEDED(m_pcm2->HandleMenuMsg(uMsg, wParam, lParam)))
		{
			return true;
		}
	}

	return false;
}



CTextureViewer* g_pTextureViewer;


LRESULT CALLBACK CTextureViewer::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CTextureViewer* pViewer = (CTextureViewer*)GetWindowLong(hWnd, 0);
    LRESULT lResult = NULL;

	// complicated but works ok
	if ((pViewer != NULL) && (uMsg != WM_CREATE))
	{
		if (pViewer->HandleShellMenuMsg(hWnd, uMsg, wParam, lParam))
		{
			return 0;
		}
	}

    switch (uMsg)
	{
        case WM_SIZE:
            pViewer->WM_Size(hWnd, wParam, lParam);
            break;

		case 0x020A: // WM_MOUSEWHEEL
			pViewer->WM_MouseWheel(hWnd, wParam, lParam);
			break;

		case WM_LBUTTONDOWN:
			pViewer->WM_LButtonDown(hWnd, wParam, lParam);
			break;

		case WM_LBUTTONUP:
			pViewer->WM_LButtonUp(hWnd, wParam, lParam);
			break;

		case WM_MBUTTONDOWN:
			pViewer->WM_MButtonDown(hWnd, wParam, lParam);
			break;

		case WM_MBUTTONUP:
			pViewer->WM_MButtonUp(hWnd, wParam, lParam);
			break;

		case WM_RBUTTONDOWN:
			pViewer->WM_RButtonDown(hWnd, wParam, lParam);
			break;

		case WM_RBUTTONUP:
			pViewer->WM_RButtonUp(hWnd, wParam, lParam);
			break;

        case WM_KEYDOWN:
            pViewer->WM_KeyDown(hWnd, wParam, lParam);
            break;

        case WM_KEYUP:
            pViewer->WM_KeyUp(hWnd, wParam, lParam);
            break;

        case WM_COMMAND:
            pViewer->WM_Command(hWnd, wParam, lParam);
            break;

		case WM_CLOSE:
			 pViewer->WM_Close(hWnd, wParam, lParam);
            PostQuitMessage(0);
			break;

        case WM_CREATE:
			pViewer = new CTextureViewer();
			g_pTextureViewer = pViewer;
			SetWindowLong(hWnd, 0, (LONG)pViewer);
			lResult = pViewer->WM_Create(hWnd, wParam, lParam);
            break;

        case WM_DESTROY:
			pViewer->WM_Destroy(hWnd, wParam, lParam);
			delete pViewer;
            break;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return lResult;
}

