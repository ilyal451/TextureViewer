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

#include "libpng/png.h"


SystemFuncs_t* g_sys;
PLibClientFuncs_t* g_plibclient;


typedef struct PNGPixelFormatInfo_s
{
	int eColorType;
	int nBPP;
	const char* pszFormatStr;
	int eInPixelFormat;
	PixelFormat_t pf;
	char aiBitDepths[NUM_CHANNELS];
	int flags;
	int eGamma;
} PNGPixelFormatInfo_t;


PNGPixelFormatInfo_t g_apngPF[] =
{
	{ PNG_COLOR_TYPE_GRAY,			 1,	"PNG 1-bit grayscale",					IN_L1M, { -1, CL_L, CS_8BIT, CT_UNORM }, { 1, 1, 1, 0 }, IMAGE_GRAYSCALE, GM_SRGB },
	{ PNG_COLOR_TYPE_GRAY,			 2,	"PNG 2-bit grayscale",					IN_L2M, { -1, CL_L, CS_8BIT, CT_UNORM }, { 2, 2, 2, 0 }, IMAGE_GRAYSCALE, GM_SRGB },
	{ PNG_COLOR_TYPE_GRAY,			 4,	"PNG 4-bit grayscale",					IN_L4M, { -1, CL_L, CS_8BIT, CT_UNORM }, { 4, 4, 4, 0 }, IMAGE_GRAYSCALE, GM_SRGB },
	{ PNG_COLOR_TYPE_GRAY,			 8,	"PNG 8-bit grayscale",					IN_L8, { -1, CL_L, CS_8BIT, CT_UNORM }, { 8, 8, 8, 0 }, IMAGE_GRAYSCALE, GM_SRGB },
	{ PNG_COLOR_TYPE_GRAY_ALPHA,	 8,	"PNG 16-bit grayscale + alpha",			IN_L8A8, { -1, CL_LA, CS_8BIT, CT_UNORM }, { 8, 8, 8, 8 }, IMAGE_GRAYSCALE | IMAGE_HAS_ALPHA, GM_SRGB },
	{ PNG_COLOR_TYPE_GRAY,			16,	"PNG 16-bit grayscale",					IN_L16, { -1, CL_L, CS_16BIT, CT_UNORM }, { 16, 16, 16, 0 }, IMAGE_GRAYSCALE, GM_LINEAR },
	{ PNG_COLOR_TYPE_GRAY_ALPHA,	16,	"PNG 32-bit grayscale + alpha",			IN_L16A16, { -1, CL_LA, CS_16BIT, CT_UNORM }, { 16, 16, 16, 16 }, IMAGE_GRAYSCALE | IMAGE_HAS_ALPHA, GM_LINEAR },
	{ PNG_COLOR_TYPE_PALETTE,		 1,	"PNG 1-bit indexed color",				IN_P1M, { -1, CL_P, CS_8BIT, CT_UINT }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ PNG_COLOR_TYPE_PALETTE,		 2,	"PNG 2-bit indexed color",				IN_P2M, { -1, CL_P, CS_8BIT, CT_UINT }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ PNG_COLOR_TYPE_PALETTE,		 4,	"PNG 4-bit indexed color",				IN_P4M, { -1, CL_P, CS_8BIT, CT_UINT }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ PNG_COLOR_TYPE_PALETTE,		 8,	"PNG 8-bit indexed color",				IN_P8, { -1, CL_P, CS_8BIT, CT_UINT }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ PNG_COLOR_TYPE_RGB,			 8,	"PNG 24-bit RGB",						IN_R8G8B8_UNORM, { PF_B8G8R8X8_UNORM }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ PNG_COLOR_TYPE_RGB_ALPHA,		 8,	"PNG 32-bit RGB + alpha",				IN_R8G8B8A8_UNORM, { PF_B8G8R8A8_UNORM }, { 8, 8, 8, 8 }, IMAGE_HAS_ALPHA, GM_SRGB },
	{ PNG_COLOR_TYPE_RGB,			16,	"PNG 48-bit RGB",						IN_R16G16B16, { -1, CL_RGB, CS_16BIT, CT_UNORM }, { 16, 16, 16, 0 }, 0, GM_LINEAR },
	{ PNG_COLOR_TYPE_RGB_ALPHA,		16,	"PNG 64-bit RGB + alpha",				IN_R16G16B16A16, { -1, CL_RGBA, CS_16BIT, CT_UNORM }, { 16, 16, 16, 16 }, IMAGE_HAS_ALPHA, GM_LINEAR },
};
int g_nPNGFormats = sizeof( g_apngPF ) / sizeof( PNGPixelFormatInfo_t );


PNGPixelFormatInfo_t* _PNG_PixelFormat(int eColorType, int nBPP)
{
	for ( int i = 0; i < g_nPNGFormats; i++ )
	{
		if ( g_apngPF[i].eColorType == eColorType )
		{
			if ( g_apngPF[i].nBPP == nBPP )
			{
				return &g_apngPF[i];
			}
		}
	}

	return NULL;
}


void user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	if (SYS_fread(data, length, 1, (HF)png_get_io_ptr(png_ptr)) != 1)
	{
		png_error(png_ptr, "unexpected end of file");
	}
}


typedef struct PNGFile_s
{
	PNGPixelFormatInfo_t* ppfi;
	bool bOverrideGamma;
	bool bPMAlpha;
	Gamma_t gm; // overrides the ppfi definition
	int nColors;
	HMEM hPalette;
	int iWidth;
	int iHeight;
	HMEM hImage;
	HMEM hTransparency;
} File_t;


H_FILE LoadFile( HF fp, KEYVALUEBUFFER hkvbufSettings, KEYVALUEBUFFER hkvbufMetadata )
{
	unsigned char header[8];		
	SYS_fread(header, 8, 1, fp);

	if (png_sig_cmp(header, 0, 8) == 0)
	{
		png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if (png_ptr != NULL)
		{
			png_infop info_ptr = png_create_info_struct(png_ptr);
			if (info_ptr != NULL)
			{
				if (setjmp(png_jmpbuf(png_ptr)))
				{
					goto _j_skip1;
				}

				//png_init_io(png_ptr, fp);
				png_set_read_fn(png_ptr, (void *)fp, user_read_data);
				png_set_sig_bytes(png_ptr, 8);

				png_read_info(png_ptr, info_ptr);

				int iWidth = png_get_image_width(png_ptr, info_ptr);
				int iHeight = png_get_image_height(png_ptr, info_ptr);
				int color_type = png_get_color_type(png_ptr, info_ptr);
				int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
				int channels = png_get_channels(png_ptr, info_ptr);
				int interlace_type = png_get_interlace_type(png_ptr, info_ptr);

				PNGPixelFormatInfo_t* ppfi = _PNG_PixelFormat(color_type, bit_depth);
				if (ppfi != NULL)
				{
					if (setjmp(png_jmpbuf(png_ptr)))
					{
						goto _j_skip2;
					}

					// XXX: for some reason it swaps only grayscale colors
					if ( bit_depth == 16 )
					//	&& ( color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA ) )
					{
						png_set_swap(png_ptr);
						png_read_update_info(png_ptr, info_ptr);
					}

					File_t* pFile = (File_t*)SYS_malloc(sizeof(File_t));
					if (pFile != NULL)
					{
						pFile->ppfi = ppfi;
						pFile->bOverrideGamma = false;
						pFile->bPMAlpha = false;
						pFile->iWidth = iWidth;
						pFile->iHeight = iHeight;
						pFile->nColors = 0;
						pFile->hPalette = NULL;
						pFile->hImage = NULL;
						pFile->hTransparency = NULL;

						if ( color_type == PNG_COLOR_TYPE_PALETTE )
						{
							if ( png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE) )
							{
								int nColors;
								png_color* pal;
								png_get_PLTE(png_ptr, info_ptr, &pal, &nColors);

								pFile->nColors = nColors;
								int iPaletteSize = nColors * 3;
								pFile->hPalette = SYS_AllocStreamMemory( iPaletteSize );
								if ( pFile->hPalette == NULL )
								{
									goto err1;
								}
								SYS_WriteStreamMemory( pFile->hPalette, 0, iPaletteSize, pal );
							}
							else
							{
								goto err1;
							}
						}

						/*
						if ( png_get_valid(png_ptr, info_ptr, PNG_INFO_gAMA) )
						{
							int iGamma;
							png_get_gAMA_fixed(png_ptr, info_ptr, &iGamma);
							if ( iGamma == 100000 )
							{
								pFile->gm.eGamma = GM_LINEAR;
							}
							else if ( iGamma == 45454 || iGamma == 45455 )
							{
								pFile->gm.eGamma = GM_SRGB;
							}
							else
							{
								pFile->gm.eGamma = GM_SPECIFY;
								pFile->gm.flGamma = 1.0 / ( (float)iGamma / 100000 );
							}
							pFile->bOverrideGamma = true;
						}
						*/

						if ( png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS) )
						{
							// TODO:
						}

						int iScanlineSize = png_get_rowbytes(png_ptr, info_ptr);

						int iPixelSize = g_plibclient->pfnGetPixelSize( &ppfi->pf );
						int iPitch = iPixelSize * iWidth;
						int iImageSize = iPitch * iHeight;

						pFile->hImage = SYS_AllocStreamMemory(iImageSize);
						if (pFile->hImage != NULL)
						{
							if ( interlace_type == PNG_INTERLACE_NONE )
							{
								char* out = ( char* )SYS_malloc( iPitch + iScanlineSize );
								if ( out != NULL )
								{
									char* in = &out[iPitch];

									if (setjmp(png_jmpbuf(png_ptr)))
									{
										goto _j_skip3;
									}

									png_bytep row_pointer = (png_bytep)in;
									for (int y = 0; y < iHeight; y++)
									{
										png_read_row(png_ptr, row_pointer, NULL);
										g_plibclient->pfnDecodePixels( ppfi->eInPixelFormat, out, in, iWidth, 1, 0 );
										int iOffset = y * iPitch;
										SYS_WriteStreamMemory( pFile->hImage, iOffset, iPitch, out );
									}

									png_read_end(png_ptr, NULL);

									SYS_free(out);

									png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

									return ( H_FILE )pFile;

_j_skip3:
									SYS_free(out);
								}
							}
							else if ( interlace_type == PNG_INTERLACE_ADAM7 )
							{
								// in this mode read the entire image at once
								int iImageSize = iHeight * iScanlineSize;
								char* out = ( char* )SYS_malloc( iPitch + iImageSize );
								if ( out != NULL )
								{
									char* in = &out[iPitch];

									png_bytep* row_pointers = (png_bytep*)SYS_malloc(sizeof(png_bytep) * iHeight);
									if ( row_pointers != NULL )
									{
										for (int y = 0; y < iHeight; y++)
										{
											row_pointers[y] = (unsigned char*)&in[y * iScanlineSize];
										}

										if (setjmp(png_jmpbuf(png_ptr)))
										{
											goto _j_skip4;
										}

										png_read_image(png_ptr, row_pointers);

										png_read_end(png_ptr, NULL);

										for (int y = 0; y < iHeight; y++)
										{
											g_plibclient->pfnDecodePixels( ppfi->eInPixelFormat, out, &in[y * iScanlineSize], iWidth, 1, 0 );
											int iOffset = y * iPitch;
											SYS_WriteStreamMemory( pFile->hImage, iOffset, iPitch, out );
										}

										SYS_free( row_pointers );

										SYS_free(out);

										png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

										return ( H_FILE )pFile;

_j_skip4:
										SYS_free( row_pointers );
									}

									SYS_free(out);
								}
							}

							SYS_FreeStreamMemory( pFile->hImage );
						}
err1:
						if ( pFile->hPalette != NULL )
						{
							SYS_FreeStreamMemory( pFile->hPalette );
						}

						if ( pFile->hTransparency != NULL )
						{
							SYS_FreeStreamMemory( pFile->hTransparency );
						}

						SYS_free(pFile);
					}
_j_skip2:
					;
				}

_j_skip1:
				;
			}

			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
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

	if ( pFile->hTransparency != NULL )
	{
		SYS_FreeStreamMemory( pFile->hTransparency );
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
	return ( H_SET )hFile; // only one set here
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
	if ( pFile->bOverrideGamma )
	{
		*pgm = pFile->gm;
	}
	else
	{
		pgm->eGamma = pFile->ppfi->eGamma;
	}
}


void GetOutputGamma( H_SET hSet, Gamma_t* pgm )
{
	File_t* pFile = ( File_t* )hSet;
	if ( pFile->bOverrideGamma )
	{
		*pgm = pFile->gm;
	}
	else
	{
		pgm->eGamma = pFile->ppfi->eGamma;
	}
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
		if ( pFile->hPalette != NULL )
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if ( pFile->hTransparency != NULL )
		{
			return 2;
		}
		else
		{
			return 1;
		}
	}
}


void GetStreamPixelFormat( H_SET hSet, int eStreamType, int iStream, PixelFormat_t* ppf )
{
	File_t* pFile = ( File_t* )hSet;

	if ( eStreamType == ST_PALETTE )
	{
		ppf->ePixelFormat = -1;
		ppf->eChannelLayout = CL_RGB;
		ppf->iComponentSize = CS_8BIT;
		ppf->eDataFormat = CT_UNORM;
	}
	else
	{
		if ( iStream == 0 )
		{
			*ppf = pFile->ppfi->pf;
		}
		else
		{
			ppf->ePixelFormat = -1; // PF_A1_UNORM
			ppf->eChannelLayout = CL_A;
			ppf->iComponentSize = CS_8BIT;
			ppf->eDataFormat = CT_UNORM;
		}
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
		if ( iStream == 0 )
		{
			return pFile->hImage;
		}
		else
		{
			return pFile->hTransparency;
		}
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

