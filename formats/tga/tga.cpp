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

#include "targa.h"


SystemFuncs_t* g_sys;
PLibClientFuncs_t* g_plibclient;


typedef unsigned char byte_t;


typedef struct PixelFormatInfo_s
{
	int eImageType;
	int iPixelSize;
	const char* pszFormatStr;
	int eInPixelFormat;
	PixelFormat_t pf;
	char aiBitDepths[NUM_CHANNELS];
	int flags;
	int eGamma;
} PixelFormatInfo_t;


PixelFormatInfo_t g_apfi[] =
{
	{ TGA_PAL,					 8,	"8-bit indexed color",					IN_P8, { -1, CL_P, CS_8BIT, CT_UINT }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ TGA_PAL,					16,	"16-bit indexed color",					IN_P16, { -1, CL_P, CS_16BIT, CT_UINT }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ TGA_RGB,					15,	"15-bit RGB",							IN_B5G5R5X1_UNORM, { PF_B8G8R8X8_UNORM }, { 5, 5, 5, 0 }, 0, GM_SRGB },
	{ TGA_RGB,					16,	"16-bit ARGB",							IN_B5G5R5A1_UNORM, { PF_B8G8R8A8_UNORM }, { 5, 5, 5, 1 }, 0, GM_SRGB },
	{ TGA_RGB,					24,	"24-bit RGB",							IN_B8G8R8_UNORM, { PF_B8G8R8X8_UNORM }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ TGA_RGB,					32,	"32-bit ARGB",							IN_B8G8R8A8_UNORM, { PF_B8G8R8A8_UNORM }, { 8, 8, 8, 8 }, 0, GM_SRGB },
	{ TGA_RGB,					48,	"48-bit RGB",							IN_R16G16B16, { -1, CL_RGB, CS_16BIT, CT_UNORM }, { 16, 16, 16, 0 }, 0, GM_LINEAR },
	{ TGA_RGB,					64,	"64-bit ARGB",							IN_R16G16B16A16, { -1, CL_RGBA, CS_16BIT, CT_UNORM }, { 16, 16, 16, 16 }, 0, GM_LINEAR },
	{ TGA_BW,					 8,	"8-bit grayscale",						IN_L8, { -1, CL_L, CS_8BIT, CT_UNORM }, { 8, 8, 8, 0 }, IMAGE_GRAYSCALE, GM_SRGB },
	{ TGA_BW,					16,	"16-bit grayscale",						IN_L16, { -1, CL_L, CS_16BIT, CT_UNORM }, { 8, 8, 8, 0 }, IMAGE_GRAYSCALE, GM_LINEAR },

};
int g_nFormats = sizeof( g_apfi ) / sizeof( PixelFormatInfo_t );


PixelFormatInfo_t* _TGA_PixelFormat( int eImageType, int iPixelSize, bool *pbRLE )
{
	if ( eImageType >= 8 )
	{
		eImageType -= 8;
		*pbRLE = true;
	}
	else
	{
		*pbRLE = false;
	}

	for ( int i = 0; i < g_nFormats; i++ )
	{
		if ( g_apfi[i].eImageType == eImageType )
		{
			if ( g_apfi[i].iPixelSize == iPixelSize )
			{
				return &g_apfi[i];
			}
		}
	}

	return NULL;
}


// this is slow as it reads the stream by one byte
bool _TGA_ReadRLEPixels(HF stream, byte_t* out, int iCount, int iPixelSize)
{
	byte_t c;
	byte_t v[4];
	int nPixels;
	int i;
	int j;
	int n;

	for (j = 0; j < iCount; )
	{
		if (!SYS_fread(&c, 1, 1, stream))
		{
			goto quit;
		}

		nPixels = (c & 0x7F) + 1;

		if (c & 0x80)
		{
			// encoded pixels

			if (!SYS_fread(v, iPixelSize, 1, stream))
			{
				goto quit;
			}

			for (n = 0; n < nPixels; n++)
			{
				if (j >= iCount)
				{
					goto quit;
				}

				for (i = 0; i < iPixelSize; i++)
				{
					out[j*iPixelSize+i] = v[i];
				}

				j++;
			}
		}
		else
		{
			// raw pixels
			for (n = 0; n < nPixels; n++)
			{
				if (!SYS_fread(v, iPixelSize, 1, stream))
				{
					goto quit;
				}

				if (j >= iCount)
				{
					goto quit;
				}

				for (i = 0; i < iPixelSize; i++)
				{
					out[j*iPixelSize+i] = v[i];
				}

				j++;
			}
		}
	}

	return true;

quit:

	return false;
}



typedef struct File_s
{
	PixelFormatInfo_t* ppfi;
	PixelFormatInfo_t* ppfiPal;
	int nColors;
	HMEM hPalette;
	int iWidth;
	int iHeight;
	HMEM hImage;
} File_t;


H_FILE LoadFile( HF stream, KEYVALUEBUFFER hkvbufSettings, KEYVALUEBUFFER hkvbufMetadata )
{
	tga_file_header_t fh;
	if (SYS_fread(&fh, sizeof(tga_file_header_t), 1, stream))
	{
		int iWidth = fh.img.iWidth;
		int iHeight = fh.img.iHeight;

		int eImageType = fh.eImageType;
		bool bRLE = false;
		/* just don't support rle may be
		if ( eImageType == TGA_PAL_RLE )
		{
			eImageType = TGA_PAL;
			bRLE = true;
		}
		else if ( eImageType == TGA_RGB_RLE )
		{
			eImageType = TGA_RGB;
			bRLE = true;
		}
		else if ( eImageType == TGA_BW_RLE )
		{
			eImageType = TGA_BW;
			bRLE = true;
		}
		*/

		PixelFormatInfo_t* ppfi = _TGA_PixelFormat( eImageType, fh.img.iPixelSize, &bRLE );
		if ( ppfi )
		{
			if (fh.iIDLen != 0)// skip the ID
			{
				SYS_fseek(stream, (unsigned long)fh.iIDLen, SEEK_CUR);
			}

			File_t* pFile = (File_t*)SYS_malloc(sizeof(File_t));
			if (pFile != NULL)
			{
				pFile->ppfi = ppfi;
				pFile->ppfiPal = NULL;
				pFile->nColors = 0;
				pFile->hPalette = NULL;
				pFile->iWidth = iWidth;
				pFile->iHeight = iHeight;
				pFile->hImage = NULL;

				// grab the palette
				if (fh.eColorMapType == 1)
				{
					bool bPalPassed = false;
					bool b;
					PixelFormatInfo_t* ppfi = _TGA_PixelFormat( TGA_RGB, fh.cm.iEntrySize, &b );
					if ( ppfi != NULL )
					{
						int nColors = fh.cm.iStart + fh.cm.iLength;
						int iPalettePixelSize = g_plibclient->pfnGetPixelSize( &ppfi->pf );
						int iPaletteSize = nColors * iPalettePixelSize;
						pFile->hPalette = SYS_AllocStreamMemory( iPaletteSize );
						if ( pFile->hPalette != NULL )
						{
							int iEntrySizeBytes = (fh.cm.iEntrySize + 7) / 8;
							int iPaletteInputSize = iEntrySizeBytes * nColors;
							char* out = ( char* )SYS_malloc( iPaletteSize + iPaletteInputSize );
							if ( out != NULL )
							{
								char* in = &out[iPaletteSize];
								if (SYS_fread(in, iPaletteInputSize, 1, stream))
								{
									g_plibclient->pfnDecodePixels( ppfi->eInPixelFormat, out, in, nColors, 1, 0 );
									SYS_WriteStreamMemory( pFile->hPalette, 0, iPaletteSize, out );

									pFile->ppfiPal = ppfi;
									pFile->nColors = nColors;

									bPalPassed = true;
								}
								SYS_free( out );
							}
						}
					}

					if ( !bPalPassed )
					{
						goto err1;
					}
				}

				int iPixelSize = g_plibclient->pfnGetPixelSize( &ppfi->pf );
				int iPitch = iPixelSize * iWidth;
				int iImageSize = iPitch * iHeight;
				pFile->hImage = SYS_AllocStreamMemory( iImageSize );
				if ( pFile->hImage != NULL )
				{
					int iInputPixelSize = (fh.img.iPixelSize + 7) / 8;
					int iInputPitch = iInputPixelSize * iWidth;

					char* out = ( char* )SYS_malloc( iPitch + iInputPitch );
					if ( out != NULL )
					{
						char* in = &out[iPitch];

						for (int y = 0; y < iHeight; y++)
						{
							if (bRLE)
							{
								if (!_TGA_ReadRLEPixels(stream, ( byte_t* )in, iWidth, iInputPixelSize))
								{
									goto finish;
								}
							}
							else
							{
								if (!SYS_fread(in, iInputPitch, 1, stream))
								{
									goto finish;
								}
							}

							g_plibclient->pfnDecodePixels( ppfi->eInPixelFormat, out, in, (fh.img.flags & TGA_INVERSELEFTRIGHT)? -iWidth: iWidth, 1, 0 );
							int iLine = (fh.img.flags & TGA_INVERSETOPBOTTOM)? y: (iHeight - 1) - y;
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



int GetNumSets( H_FILE hFile )
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
		*ppf = pFile->ppfiPal->pf;
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
	GetNumSets,
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

