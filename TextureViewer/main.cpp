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

#include "defs.h" // also includes <windows.h>
#include "globals.h"

#include <stdio.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

#include "../shared/utils.h"
#include "../shared/plib.h"
#include "../shared/plibclient.h"
#include "../shared/plibnative.h"

#include "controls.h"
#include "system.h"

#include "format.h"
#include "context.h"

#include "viewer.h"

#include "settings.h"
#include "keyconfig.h"


PLibFuncs_t* g_plibfuncs;
PLibClientFuncs_t* g_plibclientfuncs;
PLibNativeFuncs_t* g_plibnativefuncs;

HINSTANCE g_hInst;
HWND g_hWnd;
//int g_eDevice;
bool g_bWindowMaximized;
int g_iWindowLeft;
int g_iWindowTop;
int g_iWindowWidth;
int g_iWindowHeight;

//int g_eCacheMode = FCT_INTERNAL;
//unsigned int g_iCacheSize = 0x20000000; // 512MB
//unsigned int g_iCacheSize = 0x00100000; // 1MB
//unsigned int g_iCacheFileSizeMax = 0x01000000; // 16MB

wchar_t g_szDisplayName[] = L"Texture Viewer";

wchar_t g_szAppExePath[MAX_PATH];
wchar_t g_szAppDir[MAX_PATH];
wchar_t g_szFormatsDir[MAX_PATH];


// shared buffers (a bad idea actually...)
char abuffer[BUFFER_MAX];
wchar_t wbuffer[BUFFER_MAX];


void ErrorMessage(wchar_t* pszModule, wchar_t* pszMessage)
{
	MessageBox(((g_hWnd != NULL)? g_hWnd: NULL), pszMessage, pszModule, MB_OK | MB_ICONHAND);
}


void FatalError(const wchar_t* fmt, ...)
{
	va_list args;
	
	va_start(args, fmt);
	_vsnwprintf(wbuffer, BUFFER_MAX, fmt, args);
	va_end(args);

	ErrorMessage(g_szDisplayName, wbuffer);

#ifdef _DEBUG
		__asm int 3;
#else
		exit(1);
#endif
}


void NonFatalError(const wchar_t* fmt, ...)
{
	va_list args;
	
	va_start(args, fmt);
	_vsnwprintf(wbuffer, BUFFER_MAX, fmt, args);
	va_end(args);

	ErrorMessage(g_szDisplayName, wbuffer);
}



const wchar_t* g_pszParserSource;
int g_iParserLine;

void ParserFatalError(const char* fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	_vsnprintf(abuffer, BUFFER_MAX, fmt, args);
	va_end(args);

	swprintf(wbuffer, L"%S\nfile: %s, line: %d", abuffer, g_pszParserSource, g_iParserLine);

	ErrorMessage(g_szDisplayName, wbuffer);

#ifdef _DEBUG
		__asm int 3;
#else
		exit(1);
#endif
}


void RuntimeCheckFailed( void )
{
	ErrorMessage( L"Internal error", L"Runtime check failed." );
}


void InitMainWindow(void)
{
    const wchar_t* pszWindowClass = L"VCtrl_TextureViewer";
	WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize           = sizeof(WNDCLASSEX);
    wc.style            = CS_DBLCLKS;
	wc.lpfnWndProc      = CTextureViewer::WndProc;
    wc.cbClsExtra       = NULL;
    wc.cbWndExtra       = sizeof( void* );
    wc.hInstance        = g_hInst;
    wc.hbrBackground    = NULL;
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = pszWindowClass;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hIconSm          = 0;

	if (!RegisterClassEx(&wc))
	{
		FATAL_ERROR(L"RegisterClassEx() error");
	}

	g_hWnd = CreateWindowEx(
		NULL,
		pszWindowClass,
		NULL,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		g_iWindowLeft,
		g_iWindowTop,
		g_iWindowWidth, 
		g_iWindowHeight, 
		NULL,
		NULL,
		g_hInst,
		NULL
		);

	if (g_hWnd == NULL)
	{
		FATAL_ERROR(L"failed to create the main window (CreateWindowEx() == %d)", GetLastError());
	}
}


const wchar_t* GetAppExePath(void)
{
	return g_szAppExePath;
}


const wchar_t* GetAppDir(void)
{
	return g_szAppDir;
}


const wchar_t* GetFormatsDir(void)
{
	return g_szFormatsDir;
}


void InitDirs(void)
{
	wchar_t* pszFileName;

	GetModuleFileName(NULL, g_szAppExePath, MAX_PATH);
	GeneralizePath(g_szAppExePath);

	wcscpy(g_szAppDir, g_szAppExePath);
	pszFileName = GetFileName(g_szAppDir);
	*pszFileName = '\0';

	wcscpy(g_szFormatsDir, g_szAppDir);
	wcscat(g_szFormatsDir, L"formats/");
}


#define GETFUNCS "GetFuncs"
#define GETCLIENTFUNCS "GetClientFuncs"
#define GETNATIVEFUNCS "GetNativeFuncs"
#define INITLIBRARY "InitLibrary"

void InitPLIB( void )
{
	const wchar_t* pszDLLName = L"plib.dll";

	HMODULE hDLL = LoadLibrary( pszDLLName );
	if (hDLL == NULL)
	{
		FATAL_ERROR(L"cannot load library %s", pszDLLName);
	}

	PFNPLIBINITLIBRARY pfnInitLibrary = (PFNPLIBINITLIBRARY)GetProcAddress(hDLL, INITLIBRARY);
	pfnInitLibrary();

	PFNPLIBGETFUNCS pfnGetFuncs = (PFNPLIBGETFUNCS)GetProcAddress(hDLL, GETFUNCS);
	g_plibfuncs = pfnGetFuncs();

	PFNPLIBGETCLIENTFUNCS pfnGetClientFuncs = (PFNPLIBGETCLIENTFUNCS)GetProcAddress(hDLL, GETCLIENTFUNCS);
	g_plibclientfuncs = pfnGetClientFuncs();

	PFNPLIBGETNATIVEFUNCS pfnGetNativeFuncs = (PFNPLIBGETNATIVEFUNCS)GetProcAddress(hDLL, GETNATIVEFUNCS);
	g_plibnativefuncs = pfnGetNativeFuncs();
}


// tabstop fixes
extern HWND g_hFileOpts;

void InitAssocDlg(void);

int APIENTRY WinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR     CmdLine,
					int       nCmdShow)
{
	MSG msg;

	g_hInst = hInstance;

	OleInitialize(NULL);

	InitDirs();

	InitControls(hInstance);
	InitCommonControls();

	LoadSettings();
	LoadKeyConfig();

	SYS_InitThreads();

	// XXX: better init at context create..
	InitImageLoader();

	InitMainWindow();

	InitPLIB();

	InitFormats();
	InitAssocDlg();

	InitContext();

	g_pTextureViewer->ProcessCommandLine();

	ShowWindow(g_hWnd, (g_bWindowMaximized)? SW_SHOWMAXIMIZED: SW_SHOWNORMAL);

	g_pTextureViewer->ShowWindow1();

	while (GetMessage(&msg, NULL, 0, 0))
	{
		if ( !IsDialogMessage( g_hFileOpts, &msg ) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	SaveSettings();
	SaveKeyConfig();

	OleUninitialize();

	return (int)msg.wParam;
}

