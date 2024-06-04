/*
PLib (Pixel Library), part of the Texture Viewer project
http://imagetools.itch.io/texview
Copyright (c) 2013-2024 Ilya Lyutin

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

#include <windows.h>

#include "../shared/plib.h"
#include "../shared/plibclient.h"
#include "../shared/plibnative.h"
 

extern PLibFuncs_t g_funcs;
extern "C" __declspec(dllexport) PLibFuncs_t* __cdecl GetFuncs( void )
{
	return &g_funcs;
}
 

extern PLibClientFuncs_t g_clientFuncs;
extern "C" __declspec(dllexport) PLibClientFuncs_t* __cdecl GetClientFuncs( void )
{
	return &g_clientFuncs;
}


extern PLibNativeFuncs_t g_nativeFuncs;
extern "C" __declspec(dllexport) PLibNativeFuncs_t* __cdecl GetNativeFuncs( void )
{
	return &g_nativeFuncs;
}


void _PLIB_InitProcessor( void );
void _PLIB_InitDecoder( void );


extern "C" __declspec(dllexport) void __cdecl InitLibrary( void )
{
	_PLIB_InitProcessor();
	_PLIB_InitDecoder();
}


int WINAPI DllEntryPoint(       HINSTANCE hinst,
                                DWORD reason,
                                void* lpReserved)
{
	/*
	if ( reason == DLL_PROCESS_ATTACH )
	{
		_PLIB_InitProcessor();
		_PLIB_InitDecoder();
	}
	*/
    return TRUE;
}

