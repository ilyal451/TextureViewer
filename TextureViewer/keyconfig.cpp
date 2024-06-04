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

#include "viewer.h"

#include "controls.h"
#include "resource.h"


BYTE g_aiKeyEventCmd[NUM_TK];

char* g_apszKeyMapping[NUM_TK] =
{
	"", // 0x00
	"", // 0x01
	"", // 0x02
	"", // 0x03
	"", // 0x04
	"", // 0x05
	"", // 0x06
	"", // 0x07
	"Backspace", // 0x08
	"Tab", // 0x09
	"", // 0x0A
	"", // 0x0B
	"Clear", // 0x0C
	"Enter", // 0x0D
	"", // 0x0E
	"", // 0x0F
	"Shift", // 0x10
	"Ctrl", // 0x11
	"Menu", // 0x12
	"Pause", // 0x13
	"CapsLock", // 0x14
	"", // 0x15
	"", // 0x16
	"", // 0x17
	"", // 0x18
	"", // 0x19
	"", // 0x1A
	"Escape", // 0x1B
	"", // 0x1C
	"", // 0x1D
	"", // 0x1E
	"", // 0x1F
	"Space", // 0x20
	"PageUp", // 0x21
	"PageDown", // 0x22
	"End", // 0x23
	"Home", // 0x24
	"Left", // 0x25
	"Up", // 0x26
	"Right", // 0x27
	"Down", // 0x28
	"", // 0x29
	"", // 0x2A
	"", // 0x2B
	"", // 0x2C
	"Insert", // 0x2D
	"Delete", // 0x2E
	"", // 0x2F
	"0", // 0x30
	"1", // 0x31
	"2", // 0x32
	"3", // 0x33
	"4", // 0x34
	"5", // 0x35
	"6", // 0x36
	"7", // 0x37
	"8", // 0x38
	"9", // 0x39
	"", // 0x3A
	"", // 0x3B
	"", // 0x3C
	"", // 0x3D
	"", // 0x3E
	"", // 0x3F
	"", // 0x40
	"A", // 0x41
	"B", // 0x42
	"C", // 0x43
	"D", // 0x44
	"E", // 0x45
	"F", // 0x46
	"G", // 0x47
	"H", // 0x48
	"I", // 0x49
	"J", // 0x4A
	"K", // 0x4B
	"L", // 0x4C
	"M", // 0x4D
	"N", // 0x4E
	"O", // 0x4F
	"P", // 0x50
	"Q", // 0x51
	"R", // 0x52
	"S", // 0x53
	"T", // 0x54
	"U", // 0x55
	"V", // 0x56
	"W", // 0x57
	"X", // 0x58
	"Y", // 0x59
	"Z", // 0x5A
	"", // 0x5B
	"", // 0x5C
	"", // 0x5D
	"", // 0x5E
	"", // 0x5F
	"Num0", // 0x60
	"Num1", // 0x61
	"Num2", // 0x62
	"Num3", // 0x63
	"Num4", // 0x64
	"Num5", // 0x65
	"Num6", // 0x66
	"Num7", // 0x67
	"Num8", // 0x68
	"Num9", // 0x69
	"*", // 0x6A
	"+", // 0x6B
	",", // 0x6C
	"-", // 0x6D
	".", // 0x6E
	"/", // 0x6F
	"F1", // 0x70
	"F2", // 0x71
	"F3", // 0x72
	"F4", // 0x73
	"F5", // 0x74
	"F6", // 0x75
	"F7", // 0x76
	"F8", // 0x77
	"F9", // 0x78
	"F10", // 0x79
	"F11", // 0x7A
	"F12", // 0x7B
	"", // 0x7C
	"", // 0x7D
	"", // 0x7E
	"", // 0x7F
	"", // 0x80
	"", // 0x81
	"", // 0x82
	"", // 0x83
	"", // 0x84
	"", // 0x85
	"", // 0x86
	"", // 0x87
	"", // 0x88
	"", // 0x89
	"", // 0x8A
	"", // 0x8B
	"", // 0x8C
	"", // 0x8D
	"", // 0x8E
	"", // 0x8F
	"NumLock", // 0x90
	"Scroll", // 0x91
	"", // 0x92
	"", // 0x93
	"", // 0x94
	"", // 0x95
	"", // 0x96
	"", // 0x97
	"", // 0x98
	"", // 0x99
	"", // 0x9A
	"", // 0x9B
	"", // 0x9C
	"", // 0x9D
	"", // 0x9E
	"", // 0x9F
	"", // 0xA0
	"", // 0xA1
	"", // 0xA2
	"", // 0xA3
	"", // 0xA4
	"", // 0xA5
	"", // 0xA6
	"", // 0xA7
	"", // 0xA8
	"", // 0xA9
	"", // 0xAA
	"", // 0xAB
	"", // 0xAC
	"", // 0xAD
	"", // 0xAE
	"", // 0xAF
	"", // 0xB0
	"", // 0xB1
	"", // 0xB2
	"", // 0xB3
	"", // 0xB4
	"", // 0xB5
	"", // 0xB6
	"", // 0xB7
	"", // 0xB8
	"", // 0xB9
	";", // 0xBA
	"=", // 0xBB
	"<", // 0xBC
	"_", // 0xBD
	">", // 0xBE
	"?", // 0xBF
	"~", // 0xC0
	"", // 0xC1
	"", // 0xC2
	"", // 0xC3
	"", // 0xC4
	"", // 0xC5
	"", // 0xC6
	"", // 0xC7
	"", // 0xC8
	"", // 0xC9
	"", // 0xCA
	"", // 0xCB
	"", // 0xCC
	"", // 0xCD
	"", // 0xCE
	"", // 0xCF
	"", // 0xD0
	"", // 0xD1
	"", // 0xD2
	"", // 0xD3
	"", // 0xD4
	"", // 0xD5
	"", // 0xD6
	"", // 0xD7
	"", // 0xD8
	"", // 0xD9
	"", // 0xDA
	"[", // 0xDB
	"\\", // 0xDC
	"]", // 0xDD
	"\'", // 0xDE
	"", // 0xDF
	"", // 0xE0
	"", // 0xE1
	"", // 0xE2
	"", // 0xE3
	"", // 0xE4
	"", // 0xE5
	"", // 0xE6
	"", // 0xE7
	"", // 0xE8
	"", // 0xE9
	"", // 0xEA
	"", // 0xEB
	"", // 0xEC
	"", // 0xED
	"", // 0xEE
	"", // 0xEF
	"", // 0xF0
	"", // 0xF1
	"", // 0xF2
	"", // 0xF3
	"", // 0xF4
	"", // 0xF5
	"", // 0xF6
	"", // 0xF7
	"", // 0xF8
	"", // 0xF9
	"", // 0xFA
	"", // 0xFB
	"", // 0xFC
	"", // 0xFD
	"", // 0xFE
	"", // 0xFF
	// TK mouse codes
	"LButton",
	"RButton",
	"MButton",
	"MWheelUp",
	"MWheelDown",
};

int g_nKeys = sizeof(g_apszKeyMapping) / sizeof(char*);

typedef struct cmd_s
{
	char* pszName;
	int eCommand;
} cmd_t;

cmd_t g_acmdMap[] =
{
	{ "", 0},
	{ "chan_rgb",		CMD_CHAN_RGB },
	{ "chan_rgba",		CMD_CHAN_RGBA },
	{ "chan_red",		CMD_CHAN_RED },
	{ "chan_green",		CMD_CHAN_GREEN },
	{ "chan_blue",		CMD_CHAN_BLUE },
	{ "chan_alpha",		CMD_CHAN_ALPHA },
	{ "tile_single",	CMD_SINGLE_TILE },
	{ "tile_horz",		CMD_TILE_HORZ },
	{ "tile_vert",		CMD_TILE_VERT },
	{ "tile_horzvert",	CMD_TILE_HORZVERT },
	{ "reset_size",		CMD_RESET_SIZE },
	{ "reset_origin",	CMD_RESET_ORIGIN },
	{ "zoom_in",		CMD_ZOOM_IN },
	{ "zoom_out",		CMD_ZOOM_OUT },
	{ "proj_xy",		CMD_PROJ_XY },
	{ "proj_zy",		CMD_PROJ_ZY },
	{ "proj_xz",		CMD_PROJ_XZ },
	{ "slice_next",		CMD_NEXT_SLICE },
	{ "slice_prev",		CMD_PREV_SLICE },
	{ "mm_next",		CMD_NEXT_MIPMAP },
	{ "mm_prev",		CMD_PREV_MIPMAP },
	{ "img_next",		CMD_NEXT_TEXTURE },
	{ "img_prev",		CMD_PREV_TEXTURE },
	{ "file_next",		CMD_NEXT_FILE },
	{ "file_prev",		CMD_PREV_FILE },
	{ "shell",			CMD_SHELL },
	{ "dragdrop",		CMD_DRAGDROP },
	{ "mark_file",		CMD_MARK_FILE },
	{ "show_guidelines",		CMD_SHOW_GUIDELINES },
	{ "apply_file_settings",	CMD_APPLY_FILE_SETTINGS },
	{ "open",			CMD_OPEN },
	{ "save_tga",		CMD_SAVE_TGA },
	{ "save_bmp",		CMD_SAVE_BMP },
	{ "load_project",	CMD_LOAD_PROJECT },
	{ "save_project",	CMD_SAVE_PROJECT },
	{ "exit",			CMD_EXIT },
	{ "assoc",			CMD_ASSOC },
	{ "settings",		CMD_SETTINGS },
	{ "file_list",		CMD_FILE_LIST },
	{ "file_settings",	CMD_FILE_SETTINGS },
	{ "about",			CMD_ABOUT },
};

int g_nCommands = sizeof(g_acmdMap) / sizeof(cmd_t);


char* FindCmd(int e)
{
	int i;

	for (i = 0; i < g_nCommands; i++)
	{
		if (g_acmdMap[i].eCommand == e)
		{
			return g_acmdMap[i].pszName;
		}
	}

	return NULL;
}


int LookupKeyName(char* pszName)
{
	int i;

	for (i = 0; i < NUM_TK; i++)
	{
		if (g_apszKeyMapping[i] != NULL)
		{
			if (FStrEq(g_apszKeyMapping[i], pszName))
			{
				return i;
			}
		}
	}

	return -1;
}


int LookupCmdName(char* pszName)
{
	int i;

	for (i = 0; i < g_nCommands; i++)
	{
		if (FStrEq(g_acmdMap[i].pszName, pszName))
		{
			return g_acmdMap[i].eCommand;
		}
	}

	return -1;
}


void BindKey(char* pszKey, char* pszCmd)
{
	int eKey = LookupKeyName(pszKey);
	if (eKey == -1)
	{
		PARSER_ERROR( "unknown key %s", pszKey );
	}

	int eCmd = LookupCmdName(pszCmd);
	if (eCmd == -1)
	{
		PARSER_ERROR( "unknown cmd %s", pszCmd );
	}

	g_aiKeyEventCmd[eKey] = eCmd;
}


void UnbindAll(void)
{
	int i;

	for (i = 0; i < NUM_TK; i++)
	{
		g_aiKeyEventCmd[i] = 0;
	}
}


bool KeyConfigLineProc(char* pszLine, void* param)
{
	if ( *pszLine == '\0' )
	{
		return true;
	}

	int argc;
	char* argv[2];

	argc = ParseLine(2, argv, pszLine, "=", NULL);
	
	if (argc != 2)
	{
		PARSER_ERROR( "incomplete line" );
	}

	BindKey(argv[0], argv[1]);

	return true;
}


void ResetKeyConfig(void)
{
	UnbindAll();
 
	// XXX: draft
    g_aiKeyEventCmd['1'] = CMD_CHAN_RGB;
    g_aiKeyEventCmd['2'] = CMD_CHAN_RGBA;
    g_aiKeyEventCmd['3'] = CMD_CHAN_RED;
    g_aiKeyEventCmd['4'] = CMD_CHAN_GREEN;
    g_aiKeyEventCmd['5'] = CMD_CHAN_BLUE;
    g_aiKeyEventCmd['6'] = CMD_CHAN_ALPHA;
    g_aiKeyEventCmd['Q'] = CMD_SINGLE_TILE;
    g_aiKeyEventCmd['W'] = CMD_TILE_HORZVERT;
	g_aiKeyEventCmd['E'] = CMD_TILE_HORZ;
	g_aiKeyEventCmd['R'] = CMD_TILE_VERT;
    g_aiKeyEventCmd['X'] = CMD_RESET_SIZE;
    g_aiKeyEventCmd['C'] = CMD_RESET_ORIGIN;
	g_aiKeyEventCmd['M'] = CMD_MARK_FILE;
	g_aiKeyEventCmd['G'] = CMD_SHOW_GUIDELINES;
	g_aiKeyEventCmd[VK_RETURN] = CMD_APPLY_FILE_SETTINGS;
    g_aiKeyEventCmd[VK_ADD] = CMD_ZOOM_IN;
	g_aiKeyEventCmd[VK_SUBTRACT] = CMD_ZOOM_OUT;
    g_aiKeyEventCmd[VK_UP] = CMD_ZOOM_IN;
	g_aiKeyEventCmd[VK_DOWN] = CMD_ZOOM_OUT;
    g_aiKeyEventCmd[VK_NEXT] = CMD_NEXT_TEXTURE;
    g_aiKeyEventCmd[VK_PRIOR] = CMD_PREV_TEXTURE;
    g_aiKeyEventCmd[VK_END] = CMD_NEXT_MIPMAP;
    g_aiKeyEventCmd[VK_HOME] = CMD_PREV_MIPMAP;
    g_aiKeyEventCmd[VK_MULTIPLY] = CMD_NEXT_SLICE;
    g_aiKeyEventCmd[VK_DIVIDE] = CMD_PREV_SLICE;
    g_aiKeyEventCmd[VK_RIGHT] = CMD_NEXT_FILE;
	g_aiKeyEventCmd[VK_LEFT] = CMD_PREV_FILE;
	g_aiKeyEventCmd[TK_RBUTTON] = CMD_SHELL;
	g_aiKeyEventCmd[TK_MWHEELUP] = CMD_ZOOM_IN;
	g_aiKeyEventCmd[TK_MWHEELDOWN] = CMD_ZOOM_OUT;
}


void GetKeyConfigFileName(wchar_t* buffer)
{
	wcscpy(buffer, GetAppDir());
	wcscat(buffer, L"KeyConfig.ini");
}


void LoadKeyConfig(void)
{
	wchar_t szFileName[MAX_PATH];

	UnbindAll();

	GetKeyConfigFileName(szFileName);
	if ( !ParseConfig(szFileName, KeyConfigLineProc) )
	{
		ResetKeyConfig();
	}
}


void SaveKeyConfig(void)
{
	wchar_t szFileName[MAX_PATH];
	GetKeyConfigFileName(szFileName);

	FILE* stream = _wfopen(szFileName, L"wt");
	if (stream != NULL)
	{
		for (int i = 0; i < NUM_TK; i++)
		{
			if (g_apszKeyMapping[i] != NULL)
			{
				if (g_aiKeyEventCmd[i] != 0)
				{
					char* pszCmd = FindCmd(g_aiKeyEventCmd[i]);
					if (pszCmd != NULL)
					{
						fprintf(stream, "\"%s\"=%s\n", g_apszKeyMapping[i], pszCmd);
					}
				}
			}
		}

		fclose(stream);
	}
}


void GetKeyBindings( int eCmd, wchar_t* buffer )
{
	int n = 0;
	for (int i = 0; i < NUM_TK; i++)
	{
		if (g_apszKeyMapping[i] != NULL)
		{
			if (g_aiKeyEventCmd[i] == eCmd)
			{
				if ( n == 0 )
				{
					buffer[n++] = '\t';
					buffer[n] = '\0';
				}
				else
				{
					buffer[n++] = ',';
					buffer[n] = '\0';
				}
				n += wsprintf( &buffer[n], L"%S", g_apszKeyMapping[i] );
			}
		}
	}
}


