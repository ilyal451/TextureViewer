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


typedef unsigned char byte_t;

namespace TextureFormats {
enum ETextureFormat
{
	B8G8R8A8,
	B8G8R8X8,
	G8R8,
	R8,
	L8, // needed? (we have R8, L8 should just replicate the value over the 4 comps)
	A8, // same here

	DXT1 = 64,
	DXT3,
	DXT5,
};
}

typedef struct PixelFormatInfo_s
{
	int eTextureFormat;
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
	{ TextureFormats::B8G8R8A8,		4,	"B8G8R8A8",			IN_B8G8R8A8_UNORM, { PF_B8G8R8A8_UNORM }, { 8, 8, 8, 8 }, 0, GM_SRGB },
	{ TextureFormats::B8G8R8X8,		4,	"B8G8R8X8",			IN_B8G8R8X8_UNORM, { PF_B8G8R8X8_UNORM }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ TextureFormats::G8R8,			2,	"G8R8",				IN_G8R8, { -1, CL_GR, CS_8BIT, CT_UNORM }, { 8, 8, 0, 0 }, 0, GM_LINEAR },
	{ TextureFormats::R8,			1,	"R8",				IN_R8, { -1, CL_R, CS_8BIT, CT_UNORM }, { 8, 0, 0, 0 }, 0, GM_LINEAR },
	{ TextureFormats::L8,			1,	"L8",				IN_L8, { -1, CL_L, CS_8BIT, CT_UNORM }, { 8, 8, 8, 0 }, IMAGE_GRAYSCALE, GM_LINEAR },
	{ TextureFormats::A8,			1,	"A8",				IN_A8, { -1, CL_A, CS_8BIT, CT_UNORM }, { 0, 0, 0, 8 }, 0, GM_LINEAR },
	{ TextureFormats::DXT1,			4,	"DXT1",				IN_BC1, { PF_B8G8R8A8_UNORM }, { 8, 8, 8, 1 }, 0, GM_SRGB },
	{ TextureFormats::DXT3,			4,	"DXT3",				IN_BC2, { PF_B8G8R8A8_UNORM }, { 8, 8, 8, 8 }, 0, GM_SRGB },
	{ TextureFormats::DXT5,			4,	"DXT5",				IN_BC3, { PF_B8G8R8A8_UNORM }, { 8, 8, 8, 8 }, 0, GM_SRGB },

};
int g_nFormats = sizeof( g_apfi ) / sizeof( PixelFormatInfo_t );


PixelFormatInfo_t* _Tex_PixelFormat( int eTextureFormat )
{
	for ( int i = 0; i < g_nFormats; i++ )
	{
		if ( g_apfi[i].eTextureFormat == eTextureFormat )
		{
			return &g_apfi[i];
		}
	}

	return NULL;
}



typedef struct LevelInfo_s
{
	int iWidth;
	int iHeight;
	int iDepth;
} LevelInfo_t;

typedef struct File_s
{
	PixelFormatInfo_t* ppfi;
	int nImages;
	int nLevels;
	LevelInfo_t* asLevelInfo;
	HMEM hImage;
} File_t;


#include <pshpack1.h>
struct TexFileHeader_t
{
	int iVersion;
	unsigned char eFormat;
	unsigned char flags;
	unsigned char reserved1;
	unsigned char reserved2;
	unsigned short iWidth;
	unsigned short iHeight;
	unsigned short iDepth;
	unsigned char nLevels;
	unsigned char reserved3;
};
#include <poppack.h>


H_FILE LoadFile( HF stream, KEYVALUEBUFFER hkvbufSettings, KEYVALUEBUFFER hkvbufMetadata )
{
	TexFileHeader_t fh;
	if (SYS_fread(&fh, sizeof(TexFileHeader_t), 1, stream))
	{
		PixelFormatInfo_t* ppfi = _Tex_PixelFormat( fh.eFormat );
		if ( ppfi )
		{
			int iWidth = fh.iWidth;
			int iHeight = fh.iHeight;
			int iDepth = fh.iDepth;
			int nImages = 1;
			int nLevels = fh.nLevels;

			File_t* pFile = (File_t*)SYS_malloc(sizeof(File_t));
			if (pFile != NULL)
			{
				pFile->ppfi = ppfi;
				pFile->nImages = nImages;
				pFile->nLevels = nLevels;
				pFile->hImage = NULL;
				pFile->asLevelInfo = (LevelInfo_t*)SYS_malloc(nLevels*sizeof(LevelInfo_t));
				if ( pFile->asLevelInfo )
				{
					// gather the level infos
					int iTotalSize = 0;
					int iLevel = 0;
					for ( ; iLevel < nLevels; )
					{
						int iMIPWidth = max( 1, iWidth >> iLevel );
						int iMIPHeight = max( 1, iHeight >> iLevel );
						int iMIPDepth = max( 1, iDepth >> iLevel );
						// add to the total size
						int iSize = ppfi->iPixelSize * iMIPWidth * iMIPHeight * iMIPDepth * nImages;
						iTotalSize += iSize;
						pFile->asLevelInfo[iLevel].iWidth = iMIPWidth;
						pFile->asLevelInfo[iLevel].iHeight = iMIPHeight;
						pFile->asLevelInfo[iLevel].iDepth = iMIPDepth;
						iLevel++;
						if ( iMIPWidth <= 1 && iMIPHeight <= 1 && iMIPDepth <= 1 )
						{
							break;
						}
					}
					if ( iLevel == nLevels )
					{
						pFile->hImage = SYS_AllocStreamMemory(iTotalSize);
						if ( pFile->hImage != NULL )
						{
							// for the DXTn formats this is tricky to calculate, so we have a func
							int iHeightGranularity = g_plibclient->pfnGetHeightGranularity(ppfi->eInPixelFormat);

							// read the data
							for (int iImage = 0; iImage < nImages; iImage++)
							{
								int iLevelOffsetPixels = 0;
								for (int iLevel = 0; iLevel < nLevels; iLevel++)
								{
									LevelInfo_t* psLevelInfo = &pFile->asLevelInfo[iLevel];

									int iPitchPixels = psLevelInfo->iWidth;
									int iSliceSizePixels = iPitchPixels * psLevelInfo->iHeight;
									int iImageSizePixels = iSliceSizePixels * psLevelInfo->iDepth;
									int iImageOffsetPixels = iLevelOffsetPixels + iImageSizePixels * iImage;
									iLevelOffsetPixels += iImageSizePixels * nImages;

									int iInputChunkSize = g_plibclient->pfnGetInputSize(ppfi->eInPixelFormat, psLevelInfo->iWidth, iHeightGranularity, 0);
									int iOutputChunkSize = ppfi->iPixelSize * psLevelInfo->iWidth * iHeightGranularity;
									byte_t* in = (byte_t*)SYS_malloc(iInputChunkSize+iOutputChunkSize); // alloc two buffers at once
									if (in == NULL)
									{
										goto err3;
									}
									byte_t* out = &in[iInputChunkSize];

									for (int iSlice = 0; iSlice < psLevelInfo->iDepth; iSlice++)
									{
										int iSliceOffsetPixels = iImageOffsetPixels + iSlice * iSliceSizePixels;

										for (int y = 0; y < psLevelInfo->iHeight; y += iHeightGranularity)
										{
											int iOffsetPixels = iSliceOffsetPixels + y * iPitchPixels;

											if (SYS_fread(in, iInputChunkSize, 1, stream))
											{
												int nLines = min(iHeightGranularity, psLevelInfo->iHeight - y);
												g_plibclient->pfnDecodePixels( ppfi->eInPixelFormat, out, in, psLevelInfo->iWidth, nLines, 0 );
												int iOffset = iOffsetPixels * ppfi->iPixelSize;
												int iSize = nLines * iPitchPixels * ppfi->iPixelSize;
												SYS_WriteStreamMemory( pFile->hImage, iOffset, iSize, out );
											}
											else
											{
												goto finish; // XXX: on eof just skip everything beyond we've already read
											}
										}
									}
finish:
									SYS_free(in);
								}
							}

							return ( H_FILE )pFile;

err3:
							SYS_FreeStreamMemory( pFile->hImage );
						}
					}

					SYS_free( pFile->asLevelInfo );
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

	SYS_FreeStreamMemory( pFile->hImage );

	SYS_free( pFile->asLevelInfo );

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
	File_t* pFile = ( File_t* )hSet;
	return pFile->nImages;
}


int GetNumLevels( H_SET hSet )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->nLevels;
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


int GetImageWidth( H_SET hSet, int iLevel )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->asLevelInfo[iLevel].iWidth;
}


int GetImageHeight( H_SET hSet, int iLevel )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->asLevelInfo[iLevel].iHeight;
}


int GetImageDepth( H_SET hSet, int iLevel )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->asLevelInfo[iLevel].iDepth;
}


int GetNumPaletteColors( H_SET hSet )
{
	return 0;
}


int GetNumStreams( H_SET hSet, int eStreamType )
{
	File_t* pFile = ( File_t* )hSet;

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
		//
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
		return NULL;
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
	GetNumLevels,
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

