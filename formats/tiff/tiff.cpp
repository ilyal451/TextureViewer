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

#include "libtiff/tiff.h"
#include "libtiff/tiffio.h"


SystemFuncs_t* g_sys;
PLibClientFuncs_t* g_plibclient;


tmsize_t _TIFFReadProc(thandle_t handle, void* buffer, tmsize_t size)
{
	// XXX: large files?
	return SYS_ReadFile( ( HF )handle, buffer, size );
}


tmsize_t _TIFFWriteProc(thandle_t handle, void* buffer, tmsize_t size)
{
	return 0;
}


toff_t _TIFFSeekProc(thandle_t handle, toff_t offset, int whence)
{
	HF stream = ( HF )handle;
	FileSize_t fs;

	if (whence == SEEK_SET)
	{
		fs.iSize = offset;
	}
	else
	{
		if (whence == SEEK_CUR)
		{
			SYS_GetFilePointer(stream, &fs);
			__int64 soff = *( __int64* )&offset; // treat as signed
			if ( soff < 0 )
			{
				fs.iSize -= ( -soff );
			}
			else
			{
				fs.iSize += soff;
			}
		}
		else if (whence == SEEK_END)
		{
			SYS_GetFileSize(stream, &fs);
			fs.iSize -= offset;
		}
		else
		{
			__asm int 3;
		}
	}

	SYS_SetFilePointer(stream, &fs);

	SYS_GetFilePointer(stream, &fs);

	return fs.iSize;
}


int _TIFFCloseProc(thandle_t handle)
{
	return 1;
}


toff_t _TIFFSizeProc(thandle_t handle)
{
	FileSize_t fs;
	SYS_GetFileSize( ( HF )handle, &fs );
	return fs.iSize;
}

/*
int _TIFFMapFileProc(thandle_t handle, void** base, toff_t* size)
{
	FileSize_t fs;
	SYS_GetFileSize( ( HF )handle, &fs );
	void* p = SYS_malloc( fs.iSizeLow );
	SYS_ReadFile( ( HF )handle, p, fs.iSizeLow );
	*base = p;
	return 1;
}


void _TIFFUnmapFileProc(thandle_t handle, void* base, toff_t size)
{
	SYS_free( base );
}
*/

TIFF* _TIFFOpen( HF stream )
{
	return TIFFClientOpen( "", "r", ( thandle_t )stream, _TIFFReadProc, _TIFFWriteProc, _TIFFSeekProc, _TIFFCloseProc, _TIFFSizeProc, NULL, NULL );
}




typedef struct PixelFormatInfo_s
{
	char ePhotometric;
	char nBitsPerSample;
	char eSampleFormat;
	char nSamplesPerPixel;
	char eExtraSampleFormat;
	char eFillOrder;
	char bTransferFunction;
	char bPalette;
	const char* pszFormatStr;
	int eInPixelFormat;
	PixelFormat_t pf;
	char aiBitDepths[NUM_CHANNELS];
	short flags;
	short eGamma;
} PixelFormatInfo_t;

PixelFormatInfo_t g_apfi[] =
{
	{ PHOTOMETRIC_PALETTE,			 1, 1, 1, -1,  1,	0, 1, "TIFF 1-bit indexed",						IN_P1M, { -1, CL_P, CS_8BIT, CT_UINT }, { 16, 16, 16, 0 }, 0, GM_LINEAR },
	{ PHOTOMETRIC_PALETTE,			 2, 1, 1, -1,  1,	0, 1, "TIFF 2-bit indexed",						IN_P2M, { -1, CL_P, CS_8BIT, CT_UINT }, { 16, 16, 16, 0 }, 0, GM_LINEAR },
	{ PHOTOMETRIC_PALETTE,			 4, 1, 1, -1,  1,	0, 1, "TIFF 4-bit indexed",						IN_P4M, { -1, CL_P, CS_8BIT, CT_UINT }, { 16, 16, 16, 0 }, 0, GM_LINEAR },
	{ PHOTOMETRIC_PALETTE,			 8, 1, 1, -1, -1,	0, 1, "TIFF 8-bit indexed",						IN_P8, { -1, CL_P, CS_8BIT, CT_UINT }, { 16, 16, 16, 0 }, 0, GM_LINEAR },
	{ PHOTOMETRIC_MINISBLACK,		 1, 1, 1, -1,  1,	0, 0, "TIFF 1-bit grayscale",					IN_L1M, { -1, CL_L, CS_8BIT, CT_UNORM }, { 1, 1, 1, 0 }, IMAGE_GRAYSCALE, GM_SRGB },
	{ PHOTOMETRIC_MINISBLACK,		 2, 1, 1, -1,  1,	0, 0, "TIFF 2-bit grayscale",					IN_L2M, { -1, CL_L, CS_8BIT, CT_UNORM }, { 2, 2, 2, 0 }, IMAGE_GRAYSCALE, GM_SRGB },
	{ PHOTOMETRIC_MINISBLACK,		 4, 1, 1, -1,  1,	0, 0, "TIFF 4-bit grayscale",					IN_L4M, { -1, CL_L, CS_8BIT, CT_UNORM }, { 4, 4, 4, 0 }, IMAGE_GRAYSCALE, GM_SRGB },
	{ PHOTOMETRIC_MINISBLACK,		 8, 1, 1, -1, -1,	0, 0, "TIFF 8-bit grayscale",					IN_L8, { -1, CL_L, CS_8BIT, CT_UNORM }, { 8, 8, 8, 0 }, IMAGE_GRAYSCALE, GM_SRGB },
	{ PHOTOMETRIC_MINISBLACK,		 8, 1, 1, -1, -1,	1, 0, "TIFF 8-bit grayscale (TF)",				-1, { -1, CL_L, CS_16BIT, CT_UNORM }, { 16, 16, 16, 0 }, IMAGE_GRAYSCALE, GM_LINEAR },
	{ PHOTOMETRIC_MINISBLACK,		 8, 1, 2,  1, -1,	0, 0, "TIFF 16-bit grayscale + alpha(P)",		IN_L8A8, { -1, CL_LA, CS_8BIT, CT_UNORM }, { 8, 8, 8, 8 }, IMAGE_GRAYSCALE | IMAGE_HAS_ALPHA | IMAGE_PREMULTIPLIED_ALPHA, GM_SRGB }, // XXX: don't move to the bottom, it relies on the order
	{ PHOTOMETRIC_MINISBLACK,		 8, 1, 2, -1, -1,	0, 0, "TIFF 16-bit grayscale + alpha",			IN_L8A8, { -1, CL_LA, CS_8BIT, CT_UNORM }, { 8, 8, 8, 8 }, IMAGE_GRAYSCALE | IMAGE_HAS_ALPHA, GM_SRGB },
	{ PHOTOMETRIC_MINISBLACK,		16, 1, 1, -1, -1,	0, 0, "TIFF 16-bit grayscale",					IN_L16, { -1, CL_L, CS_16BIT, CT_UNORM }, { 16, 16, 16, 0 }, IMAGE_GRAYSCALE, GM_LINEAR },
	{ PHOTOMETRIC_MINISBLACK,		16, 3, 1, -1, -1,	0, 0, "TIFF 16-bit grayscale (float)",			IN_L16, { -1, CL_L, CS_16BIT, CT_FLOAT }, { 16, 16, 16, 0 }, IMAGE_GRAYSCALE, GM_LINEAR },
	{ PHOTOMETRIC_MINISBLACK,		32, 3, 1, -1, -1,	0, 0, "TIFF 32-bit grayscale (float)",			IN_L32, { -1, CL_L, CS_32BIT, CT_FLOAT }, { 32, 32, 32, 0 }, IMAGE_GRAYSCALE, GM_LINEAR },
	{ PHOTOMETRIC_RGB,				 8, 1, 3, -1, -1,	0, 0, "TIFF 24-bit RGB",						IN_R8G8B8_UNORM, { PF_B8G8R8X8_UNORM }, { 8, 8, 8, 0 }, 0, GM_SRGB },
	{ PHOTOMETRIC_RGB,				 8, 1, 3, -1, -1,	1, 0, "TIFF 24-bit RGB (TF)",					-1, { -1, CL_RGB, CS_16BIT, CT_UNORM }, { 16, 16, 16, 0 }, 0, GM_LINEAR },
	{ PHOTOMETRIC_RGB,				16, 1, 3, -1, -1,	0, 0, "TIFF 48-bit RGB",						IN_R16G16B16, { -1, CL_RGB, CS_16BIT, CT_UNORM }, { 16, 16, 16, 0 }, 0, GM_LINEAR },
	{ PHOTOMETRIC_RGB,				16, 3, 3, -1, -1,	0, 0, "TIFF 48-bit RGB (float)",				IN_R16G16B16, { -1, CL_RGB, CS_16BIT, CT_FLOAT }, { 16, 16, 16, 0 }, 0, GM_LINEAR },
	{ PHOTOMETRIC_RGB,				32, 3, 3, -1, -1,	0, 0, "TIFF 96-bit RGB (float)",				IN_R32G32B32, { -1, CL_RGB, CS_32BIT, CT_FLOAT }, { 32, 32, 32, 0 }, 0, GM_LINEAR },
	{ PHOTOMETRIC_RGB,				 8, 1, 4,  1, -1,	0, 0, "TIFF 32-bit RGB + alpha(P)",				IN_R8G8B8A8_UNORM, { PF_B8G8R8A8_UNORM }, { 8, 8, 8, 8 }, IMAGE_HAS_ALPHA | IMAGE_PREMULTIPLIED_ALPHA, GM_SRGB }, // XXX: don't move to the bottom, it relies on the order
	{ PHOTOMETRIC_RGB,				 8, 1, 4, -1, -1,	0, 0, "TIFF 32-bit RGB + alpha",				IN_R8G8B8A8_UNORM, { PF_B8G8R8A8_UNORM }, { 8, 8, 8, 8 }, IMAGE_HAS_ALPHA, GM_SRGB },
	{ PHOTOMETRIC_RGB,				16, 1, 4, -1, -1,	0, 0, "TIFF 64-bit RGB + alpha",				IN_R16G16B16A16, { -1, CL_RGBA, CS_16BIT, CT_UNORM }, { 16, 16, 16, 16 }, IMAGE_HAS_ALPHA, GM_LINEAR },
	{ PHOTOMETRIC_RGB,				16, 3, 4, -1, -1,	0, 0, "TIFF 64-bit RGB + alpha (float)",		IN_R16G16B16A16, { -1, CL_RGBA, CS_16BIT, CT_FLOAT }, { 16, 16, 16, 16 }, IMAGE_HAS_ALPHA, GM_LINEAR },
	{ PHOTOMETRIC_RGB,				32, 3, 4, -1, -1,	0, 0, "TIFF 128-bit RGB + alpha (float)",		IN_R32G32B32A32, { -1, CL_RGBA, CS_32BIT, CT_FLOAT }, { 32, 32, 32, 32 }, IMAGE_HAS_ALPHA, GM_LINEAR },
};
int g_nTIFFFormats = sizeof( g_apfi ) / sizeof( PixelFormatInfo_t );


PixelFormatInfo_t* _TIFF_PixelFormat( int ePhotometric, int nBitsPerSample, int nSamplesPerPixel, int eSampleFormat, int eExtraSampleFormat, int eFillOrder, bool bTransferFunction )
{
	for ( int i = 0; i < g_nTIFFFormats; i++ )
	{
		if ( g_apfi[i].ePhotometric == ePhotometric )
		{
			if ( g_apfi[i].nBitsPerSample == nBitsPerSample )
			{
				if ( g_apfi[i].eSampleFormat == eSampleFormat )
				{
					if ( g_apfi[i].nSamplesPerPixel == nSamplesPerPixel )
					{
						if ( g_apfi[i].eExtraSampleFormat == -1 || g_apfi[i].eExtraSampleFormat == eExtraSampleFormat )
						{
							if ( g_apfi[i].eFillOrder == -1 || g_apfi[i].eFillOrder == eFillOrder )
							{
								if ( ( bool )g_apfi[i].bTransferFunction == bTransferFunction )
								{
									return &g_apfi[i];
								}
							}
						}
					}
				}
			}
		}
	}

	return NULL;
}


void SwapColors( void* buffer, int iCount, int iSampleSize )
{
	if ( iSampleSize == 1 )
	{
		unsigned char* a = ( unsigned char* )buffer;
		for ( int i = 0; i < iCount; i++ )
		{
			a[i] = ~a[i];
		}
	}
	else if ( iSampleSize == 2 )
	{
		unsigned short* a = ( unsigned short* )buffer;
		for ( int i = 0; i < iCount; i++ )
		{
			a[i] = ~a[i];
		}
	}
}



void ConvertWithTransferFunction( short* out, unsigned char* in, int iBitDepth, int iCount, int iSampleCount, short* altR, short* altG, short* altB )
{
	if ( iSampleCount == 1 )
	{
		if ( iBitDepth == 8 )
		{
			for ( int i = 0; i < iCount; i++ )
			{
				out[i] = altR[in[i]];
			}
		}
	}
	else if ( iSampleCount == 3 )
	{
		if ( iBitDepth == 8 )
		{
			for ( int i = 0; i < iCount; i++ )
			{
				int iPixel = i*3;
				out[iPixel+0] = altR[in[iPixel+0]];
				out[iPixel+1] = altG[in[iPixel+1]];
				out[iPixel+2] = altB[in[iPixel+2]];
			}
		}
	}
}



typedef struct TIFFFile_s
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
	TIFF* h = _TIFFOpen( stream );
	if ( h )
	{
		// read image info
		int iWidth = 0;
		TIFFGetField(h, TIFFTAG_IMAGEWIDTH, &iWidth);
		int iHeight = 0;
		TIFFGetField(h, TIFFTAG_IMAGELENGTH, &iHeight);
		//int iDepth;
		//TIFFGetField(h, TIFFTAG_IMAGEDEPTH, &iDepth);
		short nBitsPerSample = 0;
		TIFFGetField(h, TIFFTAG_BITSPERSAMPLE, &nBitsPerSample);
		short eCompression = COMPRESSION_NONE;
		TIFFGetField(h, TIFFTAG_COMPRESSION, &eCompression);
		short ePhotometric = -1;
		TIFFGetField(h, TIFFTAG_PHOTOMETRIC, &ePhotometric);
		short eOrientation = ORIENTATION_TOPLEFT;
		TIFFGetField(h, TIFFTAG_ORIENTATION, &eOrientation);
		short eFillOrder = FILLORDER_MSB2LSB;
		TIFFGetField(h, TIFFTAG_FILLORDER, &eFillOrder);
		short nSamplesPerPixel = 0;
		TIFFGetField(h, TIFFTAG_SAMPLESPERPIXEL, &nSamplesPerPixel);
		short eExtaSampleFormat = 0;
		short nExtraSamples = 0;
		short* aeExtraSampleInfo = NULL;
		TIFFGetField(h, TIFFTAG_EXTRASAMPLES, &nExtraSamples, &aeExtraSampleInfo);
		if ( nExtraSamples )
		{
			eExtaSampleFormat = aeExtraSampleInfo[0];
		}
		int nRowsPerStrip = 1;
		TIFFGetField(h, TIFFTAG_ROWSPERSTRIP, &nRowsPerStrip);
		int iTileWidth = 0;
		TIFFGetField(h, TIFFTAG_TILEWIDTH, &iTileWidth);
		int iTileHeight = 0;
		TIFFGetField(h, TIFFTAG_TILELENGTH, &iTileHeight);
		short ePlanarConfig = PLANARCONFIG_CONTIG;
		TIFFGetField(h, TIFFTAG_PLANARCONFIG, &ePlanarConfig);
		short eSampleFormat = 0;
		TIFFGetField(h, TIFFTAG_SAMPLEFORMAT, &eSampleFormat);
		if ( eSampleFormat < 1 || eSampleFormat >= 4 )
		{
			// so 32-bit samples are float by default, the rest is UNORM
			eSampleFormat = nBitsPerSample == 32 ? SAMPLEFORMAT_IEEEFP : SAMPLEFORMAT_UINT;
		}
		short* aiTF[3] = { NULL, NULL, NULL };
		TIFFGetField(h, TIFFTAG_TRANSFERFUNCTION, &aiTF[0], &aiTF[1], &aiTF[2]);

		bool bSwapTopBottom = ( eOrientation == ORIENTATION_BOTRIGHT ) || ( eOrientation == ORIENTATION_BOTLEFT );
		bool bSwapLeftRight = ( eOrientation == ORIENTATION_TOPRIGHT ) || ( eOrientation == ORIENTATION_BOTRIGHT );
		bool bSwapColors = ( ePhotometric == PHOTOMETRIC_MINISWHITE );
		bool bTransferFunction = aiTF[0] != NULL;

		if ( ePlanarConfig == PLANARCONFIG_CONTIG ) // TODO: isn't supported yet
		{
			int ePFPhotometric = ePhotometric == PHOTOMETRIC_MINISWHITE ? PHOTOMETRIC_MINISBLACK : ePhotometric;
			PixelFormatInfo_t* ppfi = _TIFF_PixelFormat( ePFPhotometric, nBitsPerSample, nSamplesPerPixel, eSampleFormat, eExtaSampleFormat, eFillOrder, bTransferFunction );
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

					int iPixelSize = g_plibclient->pfnGetPixelSize( &ppfi->pf );
					int iPitch = iPixelSize * iWidth;
					int iImageSize = iPitch * iHeight;

					int iScanlineSize = TIFFScanlineSize(h);
					if ( iScanlineSize != iPitch )
					{
						// XXX: the packed formats may not support this
						//__asm int 3;
					}

					pFile->hImage = SYS_AllocStreamMemory(iImageSize);
					if (pFile->hImage != NULL)
					{
						if ( ePhotometric == PHOTOMETRIC_PALETTE )
						{
							pFile->nColors = 1 << nBitsPerSample;
							int iPaletteSize = pFile->nColors * 8;
							pFile->hPalette = SYS_AllocStreamMemory(iPaletteSize);
							if ( pFile->hPalette == NULL )
							{
								goto err1;
							}
							short* pal = ( short* )SYS_malloc( iPaletteSize );
							if ( pal == NULL )
							{
								goto err2;
							}
							short* ap[3];
							TIFFGetField(h, TIFFTAG_COLORMAP, &ap[0], &ap[1], &ap[2]);
							for ( int i = 0; i < pFile->nColors; i++ )
							{
								int iColor = i*3;
								pal[iColor+0] = ap[0][i];
								pal[iColor+1] = ap[1][i];
								pal[iColor+2] = ap[2][i];
							}
							SYS_WriteStreamMemory( pFile->hPalette, 0, iPaletteSize, pal );
							SYS_free( pal );
						}

						if ( iTileWidth != 0 ) // read tiles
						{
							if ( iTileHeight != 0 )
							{
								int iTileSize = TIFFTileSize(h);
								int iTileScanlineSize = iTileSize / iTileHeight;
								int nColumns = ( iWidth + ( iTileWidth - 1 ) ) / iTileWidth;
								int nRows = ( iHeight + ( iTileHeight - 1 ) ) / iTileHeight;
								char* tileBuf = (char*)SYS_malloc( nColumns * iTileSize );
								if ( tileBuf != NULL )
								{
									char* out = ( char* )SYS_malloc( iPitch + iScanlineSize );
									if ( out != NULL )
									{
										char* in = &out[iPitch];
										for ( int iRow = 0; iRow < nRows; iRow++ )
										{
											int y = iRow * iTileHeight;
											for ( int iColumn = 0; iColumn < nColumns; iColumn++ )
											{
												int x = iColumn * iTileWidth;
												int iTileOffset = iColumn * iTileSize;
												if ( TIFFReadTile( h, &tileBuf[iTileOffset], x, y, 0, NULL ) == -1 )
												{
													goto err3;
												}
											}
											int nLines = min( iTileHeight, iHeight - y );
											for ( int iLine = 0; iLine < nLines; iLine++ )
											{
												for ( int iColumn = 0; iColumn < nColumns; iColumn++ )
												{
													int iTileOffset = iColumn * iTileSize;
													int iLineOffset = iLine * iTileScanlineSize;
													int iBufferOffset = iColumn * iTileScanlineSize;
													int iMaxSize = min( iTileScanlineSize, iScanlineSize - iBufferOffset );
													memcpy( &in[iBufferOffset], &tileBuf[iTileOffset+iLineOffset], iMaxSize );
												}
												if ( bTransferFunction )
												{
													ConvertWithTransferFunction( ( short* )out, ( unsigned char* )in, ppfi->nBitsPerSample, iWidth, ppfi->nSamplesPerPixel, aiTF[0], aiTF[1], aiTF[2] );
												}
												else
												{
													g_plibclient->pfnDecodePixels( ppfi->eInPixelFormat, out, in, bSwapLeftRight ? -iWidth : iWidth, 1, 0 );
												}
												if ( bSwapColors )
												{
													SwapColors( out, iWidth, ppfi->pf.iComponentSize );
												}
												int iOffset = bSwapTopBottom ? ( iHeight - ( y + iLine ) - 1 ) * iPitch : ( y + iLine ) * iPitch;
												SYS_WriteStreamMemory( pFile->hImage, iOffset, iPitch, out );
											}
										}
err3:
										SYS_free( out );
										SYS_free( tileBuf );

										TIFFClose( h );

										return ( H_FILE )pFile;
									}

									SYS_free( tileBuf );
								}
							}
						}
						else //if ( nRowsPerStrip != 1 ) // read strips
						{
							int iStripSize = TIFFStripSize( h );
							if ( iStripSize != iScanlineSize * min( iHeight, nRowsPerStrip ) )
							{
								__asm int 3;
							}
							char* out = ( char* )SYS_malloc( iPitch + iStripSize );
							if ( out != NULL )
							{
								char* in = &out[iPitch];
								int nStrips = TIFFNumberOfStrips( h );
								for ( int iStrip = 0; iStrip < nStrips; iStrip++ )
								{
									int y = iStrip * nRowsPerStrip;
									if ( TIFFReadEncodedStrip( h, iStrip, in, -1 ) == -1 )
									{
										break;
									}
									int nLines = min( nRowsPerStrip, iHeight - y );
									for ( int iLine = 0; iLine < nLines; iLine++ )
									{
										int iInOffset = iScanlineSize * iLine;
										if ( bTransferFunction )
										{
											ConvertWithTransferFunction( ( short* )out, ( unsigned char* )&in[iInOffset], ppfi->nBitsPerSample, iWidth, ppfi->nSamplesPerPixel, aiTF[0], aiTF[1], aiTF[2] );
										}
										else
										{
											g_plibclient->pfnDecodePixels( ppfi->eInPixelFormat, out, &in[iInOffset], bSwapLeftRight ? -iWidth : iWidth, 1, 0 );
										}
										if ( bSwapColors )
										{
											SwapColors( out, iWidth, ppfi->pf.iComponentSize );
										}
										int iOffset = bSwapTopBottom ? ( iHeight - ( y + iLine ) - 1 ) * iPitch : ( y + iLine ) * iPitch;
										SYS_WriteStreamMemory( pFile->hImage, iOffset, iPitch, out );
									}
								}

								SYS_free( out );

								TIFFClose( h );

								return ( H_FILE )pFile;
							}
						}
						/*
						else
						{
							// read scanlines
							int iScanlineSize = TIFFScanlineSize(h);
							if ( iScanlineSize != iPitch )
							{
								__asm int 3;
							}

							void* buffer = SYS_malloc( iScanlineSize );
							if ( buffer != NULL )
							{
								for ( int y = 0; y < iHeight; y++ )
								{
									if ( TIFFReadScanline( h, buffer, y, 0 ) == -1 )
									{
										break;
									}
									int iOffset = y * iPitch;
									SYS_WriteStreamMemory( pFile->hImage, iOffset, iPitch, buffer );
								}

								SYS_free( buffer );

								TIFFClose( h );

								return ( H_FILE )pFile;
							}
						}
						*/
err2:
						if ( pFile->hPalette != NULL )
						{
							SYS_FreeStreamMemory( pFile->hPalette );
						}
err1:
						SYS_FreeStreamMemory( pFile->hImage );
					}

					SYS_free( pFile );
				}
			}
		}

		TIFFClose( h );
	}

	return NULL;
}


void FreeFile( H_FILE hFile )
{
	File_t* pFile = (File_t*)hFile;

	SYS_FreeStreamMemory( pFile->hImage );

	SYS_free( pFile );
}



int GetNumSets( H_FILE hFile )
{
	return 1;
}


H_SET LoadSet( H_FILE hFile, int iSet )
{
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
		ppf->ePixelFormat = -1;
		ppf->eChannelLayout = CL_RGB;
		ppf->iComponentSize = CS_16BIT;
		ppf->eDataFormat = CT_UNORM;
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

