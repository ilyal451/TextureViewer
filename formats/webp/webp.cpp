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

#include <windows.h>
#include <stdio.h>

#include "../../format.h"
#include "../../shared/system.h"
#include "../../shared/plibclient.h"

#include "libwebp/src/webp/decode.h"

SystemFuncs_t* g_sys;
PLibClientFuncs_t* g_plibclient;


struct File_t
{
	int iWidth;
	int iHeight;
	HMEM hImageBits;
};


H_FILE LoadFile( HF stream, KEYVALUEBUFFER hkvbufSettings, KEYVALUEBUFFER hkvbufMetadata )
{
	FileSize_t fs;
	SYS_GetFileSize(stream, &fs);
	if (fs.iSizeHigh == 0 && fs.iSizeLow != 0)
	{
		File_t* pFile = (File_t*)SYS_malloc(sizeof(File_t));
		if ( pFile != NULL )
		{
			void* pFileBuf = SYS_malloc( fs.iSizeLow );
			if ( pFileBuf )
			{
				SYS_ReadFile( stream, pFileBuf, fs.iSizeLow );

				if ( WebPGetInfo( ( uint8_t* )pFileBuf, fs.iSizeLow, &pFile->iWidth, &pFile->iHeight ) )
				{
					int iScanSize = pFile->iWidth * 4;
					int iDataSize = iScanSize * pFile->iHeight;

					pFile->hImageBits = SYS_AllocStreamMemory( iDataSize );
					if ( pFile->hImageBits != NULL )
					{
						void* pBits = SYS_malloc( iDataSize );
						if ( pBits )
						{
							if ( WebPDecodeRGBAInto( ( uint8_t* )pFileBuf, fs.iSizeLow, ( uint8_t* )pBits, iDataSize, iScanSize ) )
							{
								SYS_WriteStreamMemory( pFile->hImageBits, 0, iDataSize, pBits );

								SYS_free( pBits );
								SYS_free( pFileBuf );

								return ( H_FILE )pFile;
							}

							SYS_free( pBits );
						}

						SYS_FreeStreamMemory( pFile->hImageBits );
					}
				}

				SYS_free( pFileBuf );
			}
		}

		SYS_free( pFile );
	}

	return NULL;
}


void FreeFile( H_FILE hFile )
{
	File_t* pFile = ( File_t* )hFile;

	SYS_FreeStreamMemory( pFile->hImageBits );

	SYS_free( pFile );
}


int GetNumImageSets( H_FILE hFile )
{
	return 1;
}


H_SET LoadSet( H_FILE hFile, int iSet )
{
	return ( H_SET )hFile; // no sets here
}


void FreeSet( H_SET hSet )
{
	// do nothing
}


int GetArraySize( H_SET hSet )
{
	return 1;
}


int GetNumMIPMaps( H_SET hSet )
{
	return 1;
}


const char* GetFormatStr( H_SET hSet )
{
	return "WebP 32-bit RGBA";
}


void GetInputGamma( H_SET hSet, Gamma_t* pgm )
{
	File_t* pFile = ( File_t* )hSet;
	pgm->eGamma = GM_SRGB;
}


void GetOutputGamma( H_SET hSet, Gamma_t* pgm )
{
	File_t* pFile = ( File_t* )hSet;
	pgm->eGamma = GM_SRGB;
}


int GetImageFlags( H_SET hSet )
{
	File_t* pFile = ( File_t* )hSet;
	return 0;
}


int GetOriginalBitDepth( H_SET hSet, int iChannel )
{
	return 8;
}



int GetImageWidth( H_SET hSet, int iMIPMap )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->iWidth;
}


int GetImageHeight( H_SET hSet, int iMIPMap )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->iHeight;
}


int GetImageDepth( H_SET hSet, int iMIPMap )
{
	return 1;
}


int GetNumPaletteColors( H_SET hSet )
{
	return 0;
}


int GetNumStreams( H_SET hSet, int eStreamType )
{
	if ( eStreamType == ST_PALETTE )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


void GetStreamPixelFormat( H_SET hSet, int eStreamType, int iStream, PixelFormat_t* ppf )
{
	File_t* pFile = ( File_t* )hSet;

	if ( eStreamType == ST_PALETTE )
	{
		// none
	}
	else
	{
		ppf->ePixelFormat = -1;
		ppf->eChannelLayout = CL_RGBA;
		ppf->iComponentSize = 1;
		ppf->eDataFormat = CT_UNORM;
	}
}


HMEM GetStreamBits( H_SET hSet, int eStreamType, int iStream )
{
	File_t* pFile = ( File_t* )hSet;

	if ( eStreamType == ST_PALETTE )
	{
		return NULL;
	}
	else
	{
		return pFile->hImageBits;
	}
}


FormatFuncs_t g_fmt =
{
	LoadFile,
	FreeFile,
	GetNumImageSets,
	LoadSet,
	FreeSet,
	GetArraySize,
	GetNumMIPMaps,
	GetFormatStr,
	GetInputGamma,
	GetOutputGamma,
	GetImageFlags,
	GetOriginalBitDepth,
	GetImageWidth,
	GetImageHeight,
	GetImageDepth,
	GetNumPaletteColors,
	GetNumStreams,
	GetStreamPixelFormat,
	GetStreamBits,
};


extern "C" __declspec(dllexport) int __cdecl GetInterfaceVersion( int iCurrentInterfaceVersion )
{
	return FORMAT_INTERFACE_VERSION;
}


extern "C" __declspec(dllexport) FormatFuncs_t* __cdecl LoadDll( SystemFuncs_t* psystem, HWND hWndMain )
{
	g_sys = psystem;
	g_plibclient = ( PLibClientFuncs_t* )psystem->pfnGetPLibClientFuncs();

	return &g_fmt;
}


BOOL WINAPI DllMain(       HINSTANCE hinst,
                                DWORD reason,
                                LPVOID lpReserved)
{
    return TRUE;
}



