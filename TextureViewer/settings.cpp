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
#include "../shared/parser.h"

#include "controls.h"

#include "viewer.h"

#include "settings.h"

#include "resource.h"



#define MAX_ALPHA_OPACITY 255
#define MIN_ALPHA_OPACITY 1
#define MAX_TILE_STEPS 8
#define MIN_TILE_STEPS 1
//#define MIN_CLIPIMAGEEXTENT 0

int g_eDevice;
COLORREF g_clrBackground;
int g_eChannelMode;
COLORREF g_clrAlpha;
int g_iAlphaOpacity;
bool g_bShowBorder;
COLORREF g_clrBorder;
int g_eMinFilter;
int g_eMagFilter;
int g_eTileMode;
int g_nTileSteps;
int g_eProjection;
bool g_bClipImage;
//int g_iClipImageExtent; // XXX: not so required
int g_eZoomOrigin; // XXX
bool g_bSoftZoom; // use soft zoom values to zoom the image (UNDONE?)
bool g_bWrapAround; // 
bool g_bAutoZoom; // zoom image to fit the viewport on new file load (UNDONE)
bool g_bResetZoom; // reset zoom on the new file
bool g_bResetSubItems; // reset subitems (textures,mipmaps,slices) on the new file
int g_eDefChannelMode;
int g_eDefTileMode;
bool g_bRememberLast;
int g_eDefGamma;

bool g_bPrecacheFiles;

bool g_bShowMenu;
bool g_bShowToolbar;
bool g_bShowStatus;
bool g_bShowFileList;
bool g_bShowFileSettings;


typedef struct var_s
{
	char* pszName;
	int eType;
	void* p;
	int iMaxSize;
	char** apszEnum;
} var_t;

enum VarTypes
{
	VAR_INT = 0,
	VAR_FLOAT,
	VAR_BOOL,
	VAR_STRING,
	VAR_COLOR,
	VAR_ENUM,
	NUM_VAR_TYPES
};


// the following vars get saved/restored from the config
var_t g_avarMap[] =
{
	{ "WindowMaximized", VAR_BOOL, &g_bWindowMaximized, 0 },
	{ "WindowLeft", VAR_INT, &g_iWindowLeft, 0 },
	{ "WindowTop", VAR_INT, &g_iWindowTop, 0 },
	{ "WindowWidth", VAR_INT, &g_iWindowWidth, 0 },
	{ "WindowHeight", VAR_INT, &g_iWindowHeight, 0 },
	{ "ShowToolbar", VAR_BOOL, &g_bShowToolbar, 0 },
	{ "ShowStatus", VAR_BOOL, &g_bShowStatus, 0 },
	{ "Device", VAR_INT, &g_eDevice, 0 },
	{ "ChannelMode", VAR_INT, &g_eChannelMode, 0 },
	{ "TileMode", VAR_INT, &g_eTileMode, 0 },
	{ "BackgroundColor", VAR_COLOR, &g_clrBackground, 0 },
	{ "AlphaColor", VAR_COLOR, &g_clrAlpha, 0 },
	{ "AlphaOpacity", VAR_INT, &g_iAlphaOpacity, 0 },
	{ "ShowBorder", VAR_BOOL, &g_bShowBorder, 0 },
	{ "BorderColor", VAR_COLOR, &g_clrBorder, 0 },
	{ "TileSteps", VAR_INT, &g_nTileSteps, 0 },
	{ "MagFilter", VAR_INT, &g_eMagFilter, 0 },
	{ "MinFilter", VAR_INT, &g_eMinFilter, 0 },
	{ "ClipImage", VAR_BOOL, &g_bClipImage, 0 },
	{ "DefChannelMode", VAR_INT, &g_eDefChannelMode, 0 },
	{ "DefTileMode", VAR_INT, &g_eDefTileMode, 0 },
	{ "PrecacheFiles", VAR_BOOL, &g_bPrecacheFiles, 0 },
	{ "ShowFileList", VAR_BOOL, &g_bShowFileList, 0 },
	{ "FileListLeft", VAR_INT, &g_iFileListLeft, 0 },
	{ "FileListTop", VAR_INT, &g_iFileListTop, 0 },
	{ "FileListWidth", VAR_INT, &g_iFileListWidth, 0 },
	{ "FileListHeight", VAR_INT, &g_iFileListHeight, 0 },
	{ "ShowFileSettings", VAR_BOOL, &g_bShowFileSettings, 0 },
	{ "FileSettingsLeft", VAR_INT, &g_iProcessingLeft, 0 },
	{ "FileSettingsTop", VAR_INT, &g_iProcessingTop, 0 },
	{ "FileSettingsWidth", VAR_INT, &g_iProcessingWidth, 0 },
	{ "FileSettingsHeight", VAR_INT, &g_iProcessingHeight, 0 },
};

int g_nVars = sizeof(g_avarMap) / sizeof(var_t);


int FindVar(char* pszName)
{
	int i;

	for (i = 0; i < g_nVars; i++)
	{
		if (FStrEq(g_avarMap[i].pszName, pszName))
		{
			return i;
		}
	}

	return -1;
}


void InitVar(var_t* pvar, char* pszValue)
{
	switch(pvar->eType)
	{
	case VAR_INT:
		*(int*)pvar->p = ParseInt(pszValue);
		break;
	case VAR_FLOAT:
		*(float*)pvar->p = ParseFloat(pszValue);
		break;
	case VAR_BOOL:
		*(bool*)pvar->p = ParseBool(pszValue);
		break;
	case VAR_STRING:
		strncpy((char*)pvar->p, pszValue, pvar->iMaxSize-1);
		break;
	case VAR_COLOR:
		*(int*)pvar->p = ParseHex(pszValue);
		break;
	case VAR_ENUM:
		*(int*)pvar->p = ParseEnum(pvar->apszEnum, pszValue);
		break;
	}
}


bool SettingsLineProc(char* pszLine, void* param)
{
	if ( *pszLine == '\0' )
	{
		return true;
	}

	int argc;
	char* argv[2];
	int i;

	argc = ParseLine(2, argv, pszLine, "=", NULL);
	
	if (argc != 2)
	{
		PARSER_ERROR( "incomplete line" );
	}

	i = FindVar(argv[0]);
	
	if (i == -1)
	{
		PARSER_ERROR( "unknown var %s", argv[0] );
	}

	InitVar(&g_avarMap[i], argv[1]);

	return true;
}


void ResetSettings(void)
{
	g_bWindowMaximized = false;
	g_iWindowWidth = 700;
	g_iWindowHeight = 510;
	g_iWindowLeft = (GetSystemMetrics(SM_CXSCREEN) - g_iWindowWidth) / 2;
	g_iWindowTop = (GetSystemMetrics(SM_CYSCREEN) - g_iWindowHeight) / 2;

	g_bShowMenu = true;
	g_bShowToolbar = true;
	g_bShowStatus = true;

	g_eDevice = DEVICE_GDI;
	g_clrBackground = 0x00EEEEEE; //0x00C8D0D4;
	g_eChannelMode = CM_RGBA;
	g_clrAlpha = 0x00FF00FF;
	g_iAlphaOpacity = MAX_ALPHA_OPACITY; // full solid
	g_bShowBorder = true;
	g_clrBorder = 0x00000000;
	g_eTileMode = TM_SINGLE_TILE;
	g_nTileSteps = 5;
	g_eProjection = PROJ_XY;
	g_eMinFilter = SM_LINEAR_SHARPEN;
	g_eMagFilter = SM_POINT;
	g_bClipImage = true;
	//g_iClipImageExtent = 0;
	g_eZoomOrigin = ZOOM_VIEWPORT_CENTER;
	g_bSoftZoom = false;
	g_bWrapAround = true;
	g_bAutoZoom = false;
	g_bResetZoom = false;
	g_bResetSubItems = true;
	g_eDefChannelMode = -1;
	g_eDefTileMode = TM_SINGLE_TILE;
	g_bRememberLast = true;
	g_eDefGamma = GM_SRGB;
	g_bPrecacheFiles = true;

	g_bShowFileList = true;
	g_iFileListWidth = 218;
	g_iFileListHeight = 420;
	g_iFileListLeft = GetSystemMetrics(SM_CXSCREEN) - g_iFileListWidth - 25;
	g_iFileListTop = GetSystemMetrics(SM_CYSCREEN) - g_iFileListHeight - 75;

	g_bShowFileSettings = false;
	g_iProcessingWidth = 218;
	g_iProcessingHeight = 456;
	g_iProcessingLeft = 25;
	g_iProcessingTop = GetSystemMetrics(SM_CYSCREEN) - g_iProcessingHeight - 75;
}


void GetSettingsFileName(wchar_t* buffer)
{
	wcscpy(buffer, GetAppDir());
	wcscat(buffer, L"Settings.ini");
}


void LoadSettings(void)
{
	wchar_t szFileName[MAX_PATH];

	ResetSettings();

	GetSettingsFileName(szFileName);
	ParseConfig(szFileName, SettingsLineProc);
}


void SaveVar(var_t* pvar, FILE* stream)
{
	switch(pvar->eType)
	{
	case VAR_INT:
		fprintf(stream, "%s=%d\n", pvar->pszName, *(int*)pvar->p);
		break;
	case VAR_FLOAT:
		fprintf(stream, "%s=%g\n", pvar->pszName, *(float*)pvar->p);
		break;
	case VAR_BOOL:
		fprintf(stream, "%s=%s\n", pvar->pszName, (*(bool*)pvar->p)? "true": "false");
		break;
	case VAR_STRING:
		fprintf(stream, "%s=%s\n", pvar->pszName, pvar->p);
		break;
	case VAR_COLOR:
		fprintf(stream, "%s=%.8X\n", pvar->pszName, *(int*)pvar->p);
		break;
	case VAR_ENUM:
		fprintf(stream, "%s=%s\n", pvar->pszName, pvar->apszEnum[*(int*)pvar->p]);
		break;
	}
}


void SaveSettings(void)
{
	wchar_t szFileName[MAX_PATH];
	FILE* stream;
	int i;

	GetSettingsFileName(szFileName);
	stream = _wfopen(szFileName, L"wt");

	if (stream != NULL)
	{
		for (i = 0; i < g_nVars; i++)
		{
			SaveVar(&g_avarMap[i], stream);
		}

		fclose(stream);
	}
}




//
// the dialog
//



typedef struct enum_s
{
	int e;
	wchar_t* psz;
} enum_t;


typedef struct setting_s
{
	int iCtrlId;
	int eType;
	void* p;
	enum_t* paenum;
	union
	{
		int nStrings;
		int iRangeMin;
	};
	int iRangeMax;
} setting_t;

enum SettingTypes
{
	ST_INT = 0,
	ST_RANGE,
	ST_BOOL,
	ST_COLOR,
	ST_ENUM,
	NUM_SETTING_TYPES
};


enum_t g_aenDevice[] =
{
	{ DEVICE_GDI, L"GDI" },
	{ DEVICE_OPENGL, L"OpenGL" },
};
int g_nDevices = sizeof(g_aenDevice) / sizeof(enum_t);

// XXX: the CM_ defs are useless here, since it obtains it straight by the index anyways..
enum_t g_aenumChannelMode[] =
{
	{ -1, L"[Last used]" },
	{ CM_RGB, L"RGB" },
	{ CM_RGBA, L"RGB + alpha" },
	{ CM_RED, L"Red" },
	{ CM_GREEN, L"Green" },
	{ CM_BLUE, L"Blue" },
	{ CM_ALPHA, L"Alpha" },
};
int g_nChannelModes = sizeof(g_aenumChannelMode) / sizeof(enum_t);

enum_t g_aenumTileMode[] =
{
	{ -1, L"[Last used]" },
	{ TM_SINGLE_TILE, L"Single tile" },
	{ TM_TILE_HORZVERT, L"Tile horz and vert" },
	{ TM_TILE_HORZ, L"Tile horz" },
	{ TM_TILE_VERT, L"Tile vert" },
};
int g_nTileModes = sizeof(g_aenumTileMode) / sizeof(enum_t);

enum_t g_aenumGamma[] =
{
	{ GM_LINEAR, L"Linear" },
	{ GM_SRGB, L"sRGB" },
};
int g_nGammas = sizeof(g_aenumGamma) / sizeof(enum_t);

enum_t g_aenumMagMode[] =
{
	{ SM_POINT, L"Point" },
};
int g_nMagModes = sizeof(g_aenumMagMode) / sizeof(enum_t);

enum_t g_aenumMinMode[] =
{
	{ SM_POINT, L"Point" },
	{ SM_LINEAR, L"Linear" },
	{ SM_LINEAR_SHARPEN, L"Linear (sharpen)" },
	//{ SM_LINEAR_SHARPEN_X8, L"Linear (sharpen more)" },
};
int g_nMinModes = sizeof(g_aenumMinMode) / sizeof(enum_t);

enum_t g_aenumZoomOrigin[] =
{
	{ ZOOM_VIEWPORT_CENTER, L"Viewport center" },
	{ ZOOM_MOUSE_CURSOR, L"Mouse cursor" },
};
int g_nZoomOrigins = sizeof(g_aenumZoomOrigin) / sizeof(enum_t);


setting_t g_aSettingsMap[] =
{
	{ IDC_DEVICE,			ST_ENUM,	&g_eDevice, g_aenDevice, g_nDevices },

	{ IDC_BG_COLOR,			ST_COLOR,	&g_clrBackground },
	{ IDC_SHOW_FRAME,		ST_BOOL,	&g_bShowBorder },
	{ IDC_FRAME_COLOR,		ST_COLOR,	&g_clrBorder },

	{ IDC_DEF_CHANNEL_MODE,	ST_ENUM,	&g_eDefChannelMode, g_aenumChannelMode, g_nChannelModes },
	{ IDC_DEF_TILE_MODE,	ST_ENUM,	&g_eDefTileMode, g_aenumTileMode, g_nTileModes },
	{ IDC_REMEMBER_LAST,	ST_BOOL,	&g_bRememberLast },

	{ IDC_FILTER_MIN,		ST_ENUM,	&g_eMinFilter, g_aenumMinMode, g_nMinModes },
	{ IDC_FILTER_MAG,		ST_ENUM,	&g_eMagFilter, g_aenumMagMode, g_nMagModes },

	{ IDC_ALPHA_COLOR,		ST_COLOR,	&g_clrAlpha },
	{ IDC_ALPHA_OPACITY,	ST_RANGE,	&g_iAlphaOpacity, NULL, MIN_ALPHA_OPACITY, MAX_ALPHA_OPACITY },

	{ IDC_TILE_STEPS,		ST_RANGE,	&g_nTileSteps, NULL, MIN_TILE_STEPS, MAX_TILE_STEPS },

	{ IDC_CLIP_IMAGE,		ST_BOOL,	&g_bClipImage },
	{ IDC_ZOOM_ORIGIN,		ST_ENUM,	&g_eZoomOrigin, g_aenumZoomOrigin, g_nZoomOrigins },
	{ IDC_SOFT_ZOOM,		ST_BOOL,	&g_bSoftZoom },

	{ IDC_WRAP_AROUND,		ST_BOOL,	&g_bWrapAround },
	{ IDC_AUTO_ZOOM,		ST_BOOL,	&g_bAutoZoom },

	{ IDC_SHOW_MENU,		ST_BOOL,	&g_bShowMenu },
	{ IDC_SHOW_TOOLBAR,		ST_BOOL,	&g_bShowToolbar },
	{ IDC_SHOW_STATUS,		ST_BOOL,	&g_bShowStatus },

	{ IDC_PRECACHE_FILES,	ST_BOOL,	&g_bPrecacheFiles },
};

int g_nSettings = sizeof(g_aSettingsMap) / sizeof(setting_t);


class CSettingsDlg
{
public:
	void Exit(bool bApply);
	void WM_InitDlg(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void WM_Command(HWND hWnd, WPARAM wParam, LPARAM lParam);
	//static BOOL CALLBACK SettingsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	void LoadSetting(setting_t* ps);
	void LoadSettings(void);
	void SaveSetting(setting_t* ps);
	void SaveSettings(void);
	void LoadDefaultSettings(void);
	HWND m_hWnd;
};


void CSettingsDlg::LoadSetting(setting_t* ps)
{
	switch (ps->eType)
	{
	case ST_RANGE:
		swprintf(wbuffer, L"%d", *(int*)ps->p);
		SendDlgItemMessage(m_hWnd, ps->iCtrlId, WM_SETTEXT, NULL, (LPARAM)wbuffer);
		break;
	case ST_BOOL:
		SendDlgItemMessage(m_hWnd, ps->iCtrlId, BM_SETCHECK, (WPARAM)*(bool*)ps->p, NULL);
		break;
	case ST_COLOR:
		SendDlgItemMessage(m_hWnd, ps->iCtrlId, CB_SETCOLOR, (WPARAM)*(COLORREF*)ps->p, NULL);
		break;
	case ST_ENUM:
		{
			int iSel = -1;
			for (int i = 0; i < ps->nStrings; i++)
			{
				if ( ps->paenum[i].e == *(int*)ps->p )
					iSel = i;
				SendDlgItemMessage(m_hWnd, ps->iCtrlId, CB_ADDSTRING, NULL, (LPARAM)ps->paenum[i].psz);
			}
			SendDlgItemMessage(m_hWnd, ps->iCtrlId, CB_SETCURSEL, (WPARAM)iSel, NULL);
		}
		break;
	}
}


void CSettingsDlg::LoadSettings(void)
{
	int i;

	for (i = 0; i < g_nSettings; i++)
	{
		LoadSetting(&g_aSettingsMap[i]);
	}
}


void CSettingsDlg::SaveSetting(setting_t* ps)
{
	switch (ps->eType)
	{
	case ST_RANGE:
		SendDlgItemMessage(m_hWnd, ps->iCtrlId, WM_GETTEXT, (WPARAM)sizeof(wbuffer), (LPARAM)wbuffer);
		*(int*)ps->p = min(ps->iRangeMax, max(ps->iRangeMin, wcstol(wbuffer, NULL, 10)));
		break;
	case ST_BOOL:
		*(bool*)ps->p = (BOOL)(SendDlgItemMessage(m_hWnd, ps->iCtrlId, BM_GETCHECK, NULL, NULL) != 0);
		break;
	case ST_COLOR:
		*(COLORREF*)ps->p = (COLORREF)SendDlgItemMessage(m_hWnd, ps->iCtrlId, CB_GETCOLOR, NULL, NULL);
		break;
	case ST_ENUM:
		int iSel = (int)SendDlgItemMessage(m_hWnd, ps->iCtrlId, CB_GETCURSEL, NULL, NULL);
		*(int*)ps->p = ps->paenum[iSel].e;
		break;
	}
}


void CSettingsDlg::SaveSettings(void)
{
	int i;

	for (i = 0; i < g_nSettings; i++)
	{
		SaveSetting(&g_aSettingsMap[i]);
	}
}


void CSettingsDlg::LoadDefaultSettings(void)
{
	if ( MessageBox( m_hWnd, L"Are you sure?", L"Load default settings", MB_YESNOCANCEL ) == IDYES )
	{
		ResetSettings();
	}
}


void CSettingsDlg::Exit(bool bApply)
{
	int eState;

	if (bApply)
	{
		SaveSettings();
		eState = IDOK;
	}
	else
	{
		eState = IDCANCEL;
	}
	
	EndDialog(m_hWnd, eState);
}


void CSettingsDlg::WM_InitDlg(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HWND hBuddy;

	m_hWnd = hWnd;

	hBuddy = GetDlgItem(m_hWnd, IDC_ALPHA_OPACITY);
	SendDlgItemMessage(m_hWnd, IDC_ALPHA_OPACITY_SPIN, UDM_SETBUDDY, (WPARAM)hBuddy, NULL);
	SendDlgItemMessage(m_hWnd, IDC_ALPHA_OPACITY_SPIN, UDM_SETRANGE, (WPARAM)NULL, (LPARAM)MAKELONG(MAX_ALPHA_OPACITY, MIN_ALPHA_OPACITY));

	hBuddy = GetDlgItem(m_hWnd, IDC_TILE_STEPS);
	SendDlgItemMessage(m_hWnd, IDC_TILE_STEPS_SPIN, UDM_SETBUDDY, (WPARAM)hBuddy, NULL);
	SendDlgItemMessage(m_hWnd, IDC_TILE_STEPS_SPIN, UDM_SETRANGE, (WPARAM)NULL, (LPARAM)MAKELONG(MAX_TILE_STEPS, MIN_TILE_STEPS));

	LoadSettings();
}


void CSettingsDlg::WM_Command(HWND hWnd, WPARAM wParam, LPARAM lParam)
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
		case IDC_ALPHA_SET_HALF:
			SendDlgItemMessage(m_hWnd, IDC_ALPHA_OPACITY, WM_SETTEXT, NULL, (WPARAM)L"127");
			break;
		case IDC_ALPHA_SET_MAX:
			SendDlgItemMessage(m_hWnd, IDC_ALPHA_OPACITY, WM_SETTEXT, NULL, (WPARAM)L"255");
			break;
		case IDC_RESET_TO_DEFAULTS:
			LoadDefaultSettings();
			break;
		}
	}
}


CSettingsDlg g_SettingsDlg;


BOOL CALLBACK SettingsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			g_SettingsDlg.WM_InitDlg(hWnd, wParam, lParam);
			break;

		case WM_COMMAND:
			g_SettingsDlg.WM_Command(hWnd, wParam, lParam);
			break;

		case WM_NOTIFY:
			break;

		case WM_CLOSE:
			g_SettingsDlg.Exit(false);
			break;

		default:
			return 0;
	}

	return 1;
}

