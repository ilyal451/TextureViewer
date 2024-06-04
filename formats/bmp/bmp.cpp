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

SystemFuncs_t* g_sys;
PLibClientFuncs_t* g_plibclient;

#define DWORD_ALIGNED(v) ((((unsigned int)(v)) + 3) & (-4))


typedef struct PixelFormatInfo_s
{
	int eCompression;
	int iBitCount;
	DWORD aiChannelMasks[NUM_CHANNELS];
	const char* pszFormatStr;
	int eInPixelFormat;
	PixelFormat_t pf;
	char aiBitDepths[NUM_CHANNELS];
	int flags;
	int eGamma;
} PixelFormatInfo_t;


PixelFormatInfo_t g_apfi[] =
{
	{ BI_RGB,			 1, {}, "BMP 1-bit indexed color",		IN_P1M, { -1, CL_P, CS_8BIT, CT_UINT }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ BI_RGB,			 2, {}, "BMP 2-bit indexed color",		IN_P2M, { -1, CL_P, CS_8BIT, CT_UINT }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ BI_RGB,			 4, {}, "BMP 4-bit indexed color",		IN_P4M, { -1, CL_P, CS_8BIT, CT_UINT }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ BI_RGB,			 8, {}, "BMP 8-bit indexed color",		IN_P8, { -1, CL_P, CS_8BIT, CT_UINT }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ BI_RGB,			16, {}, "BMP 16-bit RGB",				IN_B5G5R5X1_UNORM, { PF_B8G8R8X8_UNORM }, { 5, 5, 5, 0 }, 0, GM_SRGB },
	{ BI_RGB,			24, {}, "BMP 24-bit RGB",				IN_B8G8R8_UNORM, { PF_B8G8R8X8_UNORM }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ BI_RGB,			32, {}, "BMP 32-bit ARGB",				IN_B8G8R8A8_UNORM, { PF_B8G8R8A8_UNORM }, { 8, 8, 8, 8 }, 0, GM_SRGB },

	{ BI_BITFIELDS,		16, { 0x00000F00, 0x000000F0, 0x0000000F, 0x00000000 }, "BMP B4G4R4X4",		IN_B4G4R4X4_UNORM, { PF_B8G8R8X8_UNORM }, { 4, 4, 4, 0 }, 0, GM_SRGB },
	{ BI_BITFIELDS,		16, { 0x00000F00, 0x000000F0, 0x0000000F, 0x0000F000 }, "BMP B4G4R4A4",		IN_B4G4R4A4_UNORM, { PF_B8G8R8A8_UNORM }, { 4, 4, 4, 4 }, 0, GM_SRGB },
	{ BI_BITFIELDS,		16, { 0x00007C00, 0x000003E0, 0x0000001F, 0x00000000 }, "BMP B5G5R5X1",		IN_B5G5R5X1_UNORM, { PF_B8G8R8X8_UNORM }, { 5, 5, 5, 0 }, 0, GM_SRGB },
	{ BI_BITFIELDS,		16, { 0x00007C00, 0x000003E0, 0x0000001F, 0x00008000 }, "BMP B5G5R5A1",		IN_B5G5R5A1_UNORM, { PF_B8G8R8A8_UNORM }, { 5, 5, 5, 1 }, 0, GM_SRGB },
	{ BI_BITFIELDS,		16, { 0x0000F800, 0x000007E0, 0x0000001F, 0x00000000 }, "BMP B5G6R5",		IN_B5G6R5_UNORM, { PF_B8G8R8X8_UNORM }, { 5, 6, 5, 0 }, 0, GM_SRGB },
	{ BI_BITFIELDS,		24, { 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000 }, "BMP B8G8R8",		IN_B8G8R8_UNORM, { PF_B8G8R8X8_UNORM }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ BI_BITFIELDS,		24, { 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000 }, "BMP R8G8B8",		IN_R8G8B8_UNORM, { PF_B8G8R8X8_UNORM }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ BI_BITFIELDS,		32, { 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000 }, "BMP B8G8R8X8",		IN_B8G8R8X8_UNORM, { PF_B8G8R8X8_UNORM }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ BI_BITFIELDS,		32, { 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000 }, "BMP B8G8R8A8",		IN_B8G8R8A8_UNORM, { PF_B8G8R8A8_UNORM }, { 8, 8, 8, 8 }, 0, GM_SRGB },
	{ BI_BITFIELDS,		32, { 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 }, "BMP R8G8B8A8",		IN_R8G8B8A8_UNORM, { PF_B8G8R8A8_UNORM }, { 8, 8, 8, 8 }, 0, GM_SRGB },
	{ BI_BITFIELDS,		32, { 0x3FF00000, 0x000FFC00, 0x000003FF, 0xC0000000 }, "BMP B10G10R10A2",	IN_B10G10R10A2_UNORM, { -1, CL_RGBA, CS_16BIT, CT_UNORM }, { 10, 10, 10, 2 }, 0, GM_LINEAR },
	{ BI_BITFIELDS,		32, { 0x000003FF, 0x000FFC00, 0x3FF00000, 0xC0000000 }, "BMP R10G10B10A2",	IN_R10G10B10A2_UNORM, { -1, CL_RGBA, CS_16BIT, CT_UNORM }, { 10, 10, 10, 2 }, 0, GM_LINEAR },

};
int g_nFormats = sizeof( g_apfi ) / sizeof( PixelFormatInfo_t );



PixelFormatInfo_t* _BMP_PixelFormat( BITMAPV5HEADER* pbmiV5 )
{
	for ( int i = 0; i < g_nFormats; i++ )
	{
		if ( g_apfi[i].eCompression == pbmiV5->bV5Compression )
		{
			if ( g_apfi[i].iBitCount == pbmiV5->bV5BitCount )
			{
				if ( pbmiV5->bV5Compression == BI_RGB )
				{
					return &g_apfi[i];
				}
				else if ( pbmiV5->bV5Compression == BI_BITFIELDS )
				{
					if ( g_apfi[i].aiChannelMasks[0] == pbmiV5->bV5RedMask &&
						g_apfi[i].aiChannelMasks[1] == pbmiV5->bV5GreenMask &&
						g_apfi[i].aiChannelMasks[2] == pbmiV5->bV5BlueMask &&
						g_apfi[i].aiChannelMasks[3] == pbmiV5->bV5AlphaMask )
					{
						return &g_apfi[i];
					}
				}
			}
		}
	}

	return NULL;
}


int _BMP_ReadHeader( HF stream, BITMAPV5HEADER* pbmi )
{
	memset( pbmi, 0, sizeof( BITMAPV5HEADER ) );

	if ( SYS_fread( &pbmi->bV5Size, sizeof( DWORD ), 1, stream ) )
	{
		if ( SYS_fread( &pbmi->bV5Width, pbmi->bV5Size - sizeof( DWORD ), 1, stream ) )
		{
			if ( pbmi->bV5Size == sizeof( BITMAPINFOHEADER ) )
			{
				if ( pbmi->bV5Compression == BI_BITFIELDS )
				{
					if ( !SYS_fread( &pbmi->bV5RedMask, sizeof(unsigned int) * 3, 1, stream ) )
					{
						goto err1;
					}
				}
			}

			return 1;
err1:
			;
		}
	}

	return 0;
}


typedef struct File_s
{
	PixelFormatInfo_t* ppfi;
	int nColors;
	HMEM hPalette;
	int iWidth;
	int iHeight;
	HMEM hImage;
} File_t;


H_FILE LoadFile( HF stream, KEYVALUEBUFFER hkvbufSettings, KEYVALUEBUFFER hkvbufMetadata )
{
	BITMAPFILEHEADER bmf;
	if ( SYS_fread( &bmf, sizeof(BITMAPFILEHEADER), 1, stream ) )
	{
		if ( bmf.bfType == 'MB')
		{
			union
			{
				BITMAPINFOHEADER bmi;
				BITMAPV5HEADER bmiV5;
			};

			if ( _BMP_ReadHeader( stream, &bmiV5 ) )
			{
				int iWidth = bmi.biWidth;
				int iHeight = abs( bmi.biHeight );
				bool bSwapTopBottom = ( bmi.biHeight > 0 );

				if ( bmi.biCompression == BI_RGB || bmi.biCompression == BI_BITFIELDS )
				{
					PixelFormatInfo_t* ppfi = _BMP_PixelFormat( &bmiV5 );
					if ( ppfi != NULL )
					{
						File_t* pFile = (File_t*)SYS_malloc(sizeof(File_t));
						if (pFile != NULL)
						{
							pFile->ppfi = ppfi;
							pFile->nColors = 0;
							pFile->hPalette = NULL;
							pFile->iWidth = iWidth;
							pFile->iHeight = iHeight;
							pFile->hImage = NULL;

							if ( bmi.biBitCount <= 8 ) // has palette
							{
								bool bPalPassed = false;
								int nColors = ( bmi.biClrUsed == 0 ) ? 256 : bmi.biClrUsed;
								int iPaletteSize = nColors * 4;
								pFile->hPalette = SYS_AllocStreamMemory( iPaletteSize );
								if ( pFile->hPalette != NULL )
								{
									char* pal = ( char* )SYS_malloc( iPaletteSize );
									if ( pal != NULL )
									{
										if ( SYS_fread( pal, iPaletteSize, 1, stream ) )
										{
											SYS_WriteStreamMemory( pFile->hPalette, 0, iPaletteSize, pal );
											pFile->nColors = nColors;
											bPalPassed = true;
										}
									}
								}

								if ( !bPalPassed )
								{
									goto err1;
								}
							}

							SYS_fseek( stream, bmf.bfOffBits, SEEK_SET );

							int iPixelSize = g_plibclient->pfnGetPixelSize( &ppfi->pf );
							int iPitch = iPixelSize * iWidth;
							int iImageSize = iPitch * iHeight;

							pFile->hImage = SYS_AllocStreamMemory( iImageSize );
							if ( pFile->hImage != NULL )
							{
								//int iInPixelSize = (bmi.biBitCount + 7) / 8;
								int iInPitch = ( ( bmi.biBitCount * iWidth ) + 7 ) / 8;
								iInPitch = DWORD_ALIGNED( iInPitch );

								char* out = ( char* )SYS_malloc( iPitch + iInPitch ); // alloc two buffers at once
								if ( out != NULL )
								{
									char* in = &out[iPitch];

									for ( int y = 0; y < iHeight; y++ )
									{
										if (!SYS_fread(in, iInPitch, 1, stream))
										{
											goto finish;
										}
										g_plibclient->pfnDecodePixels( ppfi->eInPixelFormat, out, in, iWidth, 1, 0 );
										int iLine = bSwapTopBottom ? ( iHeight - 1 ) - y : y;
										int iOffset = iLine * iPitch;
										SYS_WriteStreamMemory( pFile->hImage, iOffset, iPitch, out );
									}
finish:
									SYS_free( out );

									return ( H_FILE )pFile;
								}

								SYS_FreeStreamMemory( pFile->hImage );
							}
err1:
							if ( pFile->hPalette != NULL )
							{
								SYS_FreeStreamMemory( pFile->hPalette );
							}

							SYS_free(pFile);
						}
					}
				}
			}
		}
	}

	return NULL;
}


void FreeFile( H_FILE hFile )
{
	File_t* pFile = (File_t*)hFile;

	if ( pFile->hPalette != NULL )
	{
		SYS_FreeStreamMemory( pFile->hPalette );
	}

	SYS_FreeStreamMemory( pFile->hImage );

	SYS_free( pFile );
}



int GetNumImageSets( H_FILE hFile )
{
	return 1;
}


H_SET LoadSet( H_FILE hFile, int iSet )
{
	// we do not support sets here
	return ( H_SET )hFile;
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
	File_t* pFile = ( File_t* )hSet;
	return pFile->ppfi->pszFormatStr;
}


void GetInputGamma( H_SET hSet, Gamma_t* pgm )
{
	File_t* pFile = ( File_t* )hSet;
	pgm->eGamma = pFile->ppfi->eGamma;
}


void GetOutputGamma( H_SET hSet, Gamma_t* pgm )
{
	File_t* pFile = ( File_t* )hSet;
	pgm->eGamma = pFile->ppfi->eGamma;
}


int GetImageFlags( H_SET hSet )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->ppfi->flags;
}


int GetOriginalBitDepth( H_SET hSet, int iChannel )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->ppfi->aiBitDepths[iChannel];
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
	File_t* pFile = ( File_t* )hSet;
	return pFile->nColors;
}


int GetNumStreams( H_SET hSet, int eStreamType )
{
	File_t* pFile = ( File_t* )hSet;

	if ( eStreamType == ST_PALETTE )
	{
		return pFile->hPalette ? 1 : 0;
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
		ppf->ePixelFormat = PF_B8G8R8X8_UNORM;
	}
	else
	{
		*ppf = pFile->ppfi->pf;
	}
}


HMEM GetStreamBits( H_SET hSet, int eStreamType, int iStream )
{
	File_t* pFile = ( File_t* )hSet;

	if ( eStreamType == ST_PALETTE )
	{
		return pFile->hPalette;
	}
	else
	{
		return pFile->hImage;
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

