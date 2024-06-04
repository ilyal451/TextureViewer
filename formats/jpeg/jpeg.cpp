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

// libjpeg
#include "libjpeg/jpeglib.h"

SystemFuncs_t* g_sys;
PLibClientFuncs_t* g_plibclient;


//
// EXIF
//

typedef unsigned char Byte_t;
typedef unsigned short UInt16_t;
typedef unsigned int UInt32_t;

// reads exif data from the image

// we need only the orientation currently
struct SEXIFData
{
	int eOrientation;
};


UInt16_t SwapBytes16( UInt16_t iValue, bool bSwap )
{
	if ( bSwap )
	{
		UInt16_t iResult;

		Byte_t* pSrc = ( Byte_t* )&iValue;
		Byte_t* pDst = ( Byte_t* )&iResult;

		pDst[0] = pSrc[1];
		pDst[1] = pSrc[0];

		return iResult;
	}
	else
	{
		return iValue;
	}
}


UInt32_t SwapBytes32( UInt32_t iValue, bool bSwap )
{
	if ( bSwap )
	{
		UInt32_t iResult;

		Byte_t* pSrc = ( Byte_t* )&iValue;
		Byte_t* pDst = ( Byte_t* )&iResult;

		pDst[0] = pSrc[3];
		pDst[1] = pSrc[2];
		pDst[2] = pSrc[1];
		pDst[3] = pSrc[0];

		return iResult;
	}
	else
	{
		return iValue;
	}
}


UInt16_t ReadUInt16( HF stream, bool bSwap )
{
	UInt16_t v;
	SYS_fread( &v, 2, 1, stream );
	v = SwapBytes16( v, bSwap );
	return v;
}


UInt32_t ReadUInt32( HF stream, bool bSwap )
{
	UInt32_t v;
	SYS_fread( &v, 4, 1, stream );
	v = SwapBytes32( v, bSwap );
	return v;
}

#define TIFF_BYTE_ORDER_II 'II'
#define TIFF_BYTE_ORDER_MM 'MM'

#define TIFF_SIGNATURE 42
//
//#pragma pack(push, 1)
//
//typedef struct tiff_file_header_s
//{
//	short sByteOrder; // 'II' or 'MM' ('MM' is uncovered by this version)
//	short magic; // 42
//	unsigned int iIFDOffset;
//} tiff_file_header_t;
//
//typedef struct tiff_ifd_entry_s
//{
//	short eTag;
//	short eType;
//	int iCount;
//	unsigned int value;
//} tiff_ifd_entry_t;
//
//#pragma pack(pop)

enum
{
	TIFF_ENTRY_TYPE_BYTE = 1,
	TIFF_ENTRY_TYPE_ASCII,
	TIFF_ENTRY_TYPE_SHORT,
	TIFF_ENTRY_TYPE_LONG,
	TIFF_ENTRY_TYPE_RATIONAL
};


bool ReadEXIF( HF stream, int iSize, SEXIFData* psEXIFData )
{
	char sig[6];
	SYS_fread( &sig, 6, 1, stream );
	if ( memcmp( sig, "Exif\0\0", 6 ) != 0 )
	{
		return false;
	}

	UInt16_t sByteOrder = ReadUInt16( stream, false );
	bool bSwap = sByteOrder == TIFF_BYTE_ORDER_MM;

	UInt16_t magic = ReadUInt16( stream, bSwap );
	if ( magic != TIFF_SIGNATURE )
	{
		return false;
	}

	UInt32_t iIFDOffset = ReadUInt32( stream, bSwap );
	if ( iIFDOffset > 8 )
	{
		// skip to the offset
		SYS_fseek( stream, iIFDOffset - 8, SEEK_CUR );
	}

	UInt16_t nEntries = ReadUInt16( stream, bSwap );
	for ( int i = 0; i < nEntries; i++ )
	{
		UInt16_t eTag = ReadUInt16( stream, bSwap );
		UInt16_t eType = ReadUInt16( stream, bSwap );
		UInt32_t iCount = ReadUInt32( stream, bSwap );
		UInt32_t value = ReadUInt32( stream, bSwap );
		if ( eTag == 0x0112 ) // orientation
		{
			psEXIFData->eOrientation = value;
		}
	}

	return true;
}


bool ReadEXIFData( HF stream, SEXIFData* psEXIFData )
{
	bool bStatus = false;

	// set defaults
	psEXIFData->eOrientation = 1;

	while ( true )
	{
		Byte_t sig;
		SYS_fread( &sig, 1, 1, stream );
		if ( sig != 0xFF )
		{
			break;
		}

		Byte_t tag;
		SYS_fread( &tag, 1, 1, stream );
		if ( tag == 0xD8 )
		{
			// JPEG start
			continue;
		}
		if ( tag == 0xD9 )
		{
			// JPEG end
			break;
		}

		UInt16_t iSize = ReadUInt16( stream, true );
		iSize -= 2;

		if ( tag == 0xE1 )
		{
			// EXIF
			bStatus = ReadEXIF( stream, iSize, psEXIFData );
			break;
		}
		else
		{
			// skip the tag
			SYS_fseek( stream, iSize, SEEK_CUR );
		}
	}

	return bStatus;
}




#include <setjmp.h>

typedef struct error_mgr_s
{
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
} error_mgr_t;


void _J_ErrorExit (j_common_ptr cinfo)
{
	error_mgr_t* pmgr = (error_mgr_t*)cinfo->err;
	
	longjmp(pmgr->setjmp_buffer, 1);
}


typedef struct JPEGFile_s
{
	int iWidth;
	int iHeight;
	int eChannelLayout;
	const char* pszFormatStr;
	//int eGamma;
	int flags;
	HMEM hImageBits;
} File_t;


namespace CopyFlags {
enum ECopyFlag
{
	SWAP_XY = 0x01,
	SWAP_LEFT_RIGHT = 0x02,
	SWAP_TOP_BOTTOM = 0x04,
};
}


#pragma pack(push, 1)
struct JPGRGB
{
	Byte_t r, g, b;
};
struct JPGLuminance
{
	Byte_t l;
};
#pragma pack(pop)


template<typename T>
void CopyPixels( T* dst, const T* src, int iImageWidth, int iImageHeight, int iLine, int flags )
{
	int iImagePitch = iImageWidth;
	int iAddX;
	int iAddY;
	const T* pSrc = src;
	T* pDst = dst;

	if ( flags & CopyFlags::SWAP_XY )
	{
		if ( flags & CopyFlags::SWAP_TOP_BOTTOM )
		{
			iAddX = -iImagePitch;
			iAddY = (iImagePitch*iImageHeight);
			pSrc += iImagePitch*(iImageHeight-1);
		}
		else
		{
			iAddX = iImagePitch;
			iAddY = -(iImagePitch*iImageHeight);
		}

		if ( flags & CopyFlags::SWAP_LEFT_RIGHT )
		{
			iAddY += -1;
			pSrc += (iImageWidth-1)*1;
		}
		else
		{
			iAddY += 1;
		}
	}
	else
	{
		if ( flags & CopyFlags::SWAP_LEFT_RIGHT )
		{
			iAddX = -1;
			iAddY = iImagePitch;
			pSrc += iImagePitch-1;
		}
		else
		{
			iAddX = 1;
			iAddY = -iImagePitch;
		}

		if ( flags & CopyFlags::SWAP_TOP_BOTTOM )
		{
			iAddY += -iImagePitch;
			pSrc += (iImageHeight-1)*iImagePitch;
		}
		else
		{
			iAddY += iImagePitch;
		}
	}

	int iWidth = ( flags & CopyFlags::SWAP_XY ) ? iImageHeight : iImageWidth;
	int iHeight = ( flags & CopyFlags::SWAP_XY ) ? iImageWidth : iImageHeight;
	
	//pSrc += ( ( iAddX * iWidth ) + iAddY ) * iLine;
	for ( int iY = 0; iY < iLine; iY++ )
	{
		pSrc += iAddX * iWidth;
		pSrc += iAddY;
	}
	//for ( int iY = 0; iY < iHeight; iY++ )
	//{
		for ( int iX = 0; iX < iWidth; iX++ )
		{
			*pDst = *pSrc;

			pDst += 1;
			pSrc += iAddX;
		}

	//	pSrc += iAddY;
	//}
}


void RotateImage( File_t* pImage, int eOrientation )
{
	bool bSwap = false;
	int afCopyFlags = 0;

	switch ( eOrientation )
	{
	case 1: // 0
		bSwap = false;
		afCopyFlags = 0;
		break;
	case 6: // 90
		bSwap = true;
		afCopyFlags = CopyFlags::SWAP_XY | CopyFlags::SWAP_TOP_BOTTOM;
		break;
	case 3: // 180
		bSwap = false;
		afCopyFlags = CopyFlags::SWAP_LEFT_RIGHT | CopyFlags::SWAP_TOP_BOTTOM;
		break;
	case 8: // 270
		bSwap = true;
		afCopyFlags = CopyFlags::SWAP_XY | CopyFlags::SWAP_LEFT_RIGHT;
		break;
	}

	UInt32_t iPixelSize = ( pImage->eChannelLayout == CL_L ) ? 1 : 3;
	int iScanSize = iPixelSize * pImage->iWidth;
	int iDataSize = iScanSize * pImage->iHeight;
	Byte_t* pBits = ( Byte_t* )SYS_malloc( iDataSize );
	if ( pBits )
	{
		int iDstWidth = bSwap ? pImage->iHeight : pImage->iWidth;
		int iDstHeight = bSwap ? pImage->iWidth : pImage->iHeight;
		int iDstScanSize = iPixelSize * iDstWidth;
		Byte_t* pScan = ( Byte_t* )SYS_malloc( iDstScanSize );
		if ( pScan )
		{
			SYS_ReadStreamMemory( pImage->hImageBits, 0, iDataSize, pBits );

			for ( int i = 0; i < iDstHeight; i++ )
			{
				if ( pImage->eChannelLayout == CL_L )
				{
					CopyPixels<JPGLuminance>( ( JPGLuminance* )pScan, ( const JPGLuminance* )pBits, pImage->iWidth, pImage->iHeight, i, afCopyFlags );
				}
				else
				{
					CopyPixels<JPGRGB>( ( JPGRGB* )pScan, ( const JPGRGB* )pBits, pImage->iWidth, pImage->iHeight, i, afCopyFlags );
				}

				int iOffset = iDstScanSize * i;
				SYS_WriteStreamMemory( pImage->hImageBits, iOffset, iDstScanSize, pScan );
			}

			if ( bSwap )
			{
				int iTemp = pImage->iWidth;
				pImage->iWidth = pImage->iHeight;
				pImage->iHeight = iTemp;
			}

			SYS_free( pScan );
		}

		SYS_free( pBits );
	}
}



H_FILE LoadFile( HF stream, KEYVALUEBUFFER hkvbufSettings, KEYVALUEBUFFER hkvbufMetadata )
{
	FileSize_t fs;
	SYS_GetFileSize(stream, &fs);

	if (fs.iSizeHigh == 0 && fs.iSizeLow != 0)
	{
		File_t* pFile = (File_t*)SYS_malloc(sizeof(File_t));
		if ( pFile != NULL )
		{
			void* pFileData = SYS_MapFile(stream);
			if (pFileData != NULL)
			{
				struct jpeg_decompress_struct cinfo;
				error_mgr_t jerr;

				jpeg_create_decompress(&cinfo);

				cinfo.err = jpeg_std_error(&jerr.pub);
				jerr.pub.error_exit = _J_ErrorExit;

				if (setjmp(jerr.setjmp_buffer))
				{
					goto _j_skip1;
				}

				jpeg_mem_src(&cinfo, (unsigned char*)pFileData, fs.iSizeLow);

				int eState = jpeg_read_header(&cinfo, TRUE);

				if (eState = JPEG_HEADER_OK)
				{
					pFile->iWidth = cinfo.image_width;
					pFile->iHeight = cinfo.image_height;

					// TODO: more options?
					bool bPassed = false;
					if ( cinfo.num_components == 3 )
					{
						pFile->eChannelLayout = CL_RGB;
						if ( cinfo.jpeg_color_space == JCS_UNKNOWN )
						{
							// none, unsupported
						}
						else if ( cinfo.jpeg_color_space == JCS_RGB )
						{
							pFile->pszFormatStr = "JPEG 24-bit RGB";
							bPassed = true;
						}
						else if ( cinfo.jpeg_color_space == JCS_YCbCr )
						{
							pFile->pszFormatStr = "JPEG 24-bit RGB YCC";
							bPassed = true;
						}
						else if ( cinfo.jpeg_color_space == JCS_BG_RGB )
						{
							pFile->pszFormatStr = "JPEG 24-bit RGB (big gamut)";
							bPassed = true;
						}
						else if ( cinfo.jpeg_color_space == JCS_BG_YCC )
						{
							pFile->pszFormatStr = "JPEG 24-bit RGB YCC (big gamut)";
							bPassed = true;
						}
						pFile->flags = 0;
						cinfo.out_color_space = JCS_RGB;
					}
					/*
					else if (cinfo.num_components == 4)
					{
						pFile->eChannelLayout = CL_RGBA;
						if ( cinfo.jpeg_color_space == JCS_CMYK )
						{
							pFile->pszFormatStr = "JPEG 32-bit CMYK";
							bPassed = true;
						}
						else if ( cinfo.jpeg_color_space == JCS_YCCK )
						{
							pFile->pszFormatStr = "JPEG 32-bit YCCK";
							bPassed = true;
						}
						pFile->flags = 0;
						cinfo.out_color_space = JCS_CMYK;
					}
					*/
					else if (cinfo.num_components == 1)
					{
						pFile->eChannelLayout = CL_L;
						if ( cinfo.jpeg_color_space == JCS_UNKNOWN )
						{
							// none, unsupported
						}
						else if ( cinfo.jpeg_color_space == JCS_GRAYSCALE )
						{
							pFile->pszFormatStr = "JPEG 8-bit grayscale";
							bPassed = true;
						}
						pFile->flags = IMAGE_GRAYSCALE;
						cinfo.out_color_space = JCS_GRAYSCALE;
					}


					if ( bPassed )
					{
						PixelFormat_t pf = { -1, pFile->eChannelLayout, 1, CT_UNORM };
						int iDstPixelSize = g_plibclient->pfnGetPixelSize( &pf );
						int iDstPitch = pFile->iWidth * iDstPixelSize;
						int iDstSize = pFile->iHeight * iDstPitch;

						pFile->hImageBits = SYS_AllocStreamMemory( iDstSize );
						if ( pFile->hImageBits != NULL )
						{
							int iSrcPixelSize = cinfo.num_components;
							int iSrcPitch = iSrcPixelSize * pFile->iWidth;
							int iSrcSize = iSrcPitch * pFile->iHeight;

							char* out = ( char* )SYS_malloc( iDstPitch + iSrcPitch );// get two buffers at once
							if ( out != NULL )
							{
								char* in = &out[iDstPitch];

								if (setjmp(jerr.setjmp_buffer))
								{
									goto _j_skip2;
								}
								
								jpeg_start_decompress(&cinfo);

								if (setjmp(jerr.setjmp_buffer))
								{
									goto _j_skip3;
								}

								// TODO: read more scanlines at once
								for (int y = 0; y < pFile->iHeight; y++)
								{
									jpeg_read_scanlines(&cinfo, (JSAMPARRAY)&in, 1);

									int iDstOffset = iDstPitch * y;
									SYS_WriteStreamMemory( pFile->hImageBits, iDstOffset, iDstPitch, in );
								}

//_j_skip3: -- if you put it here, it will enter a infinite loop
								jpeg_finish_decompress(&cinfo);
// XXX:
_j_skip3:
_j_skip2:
								SYS_free(out);

								jpeg_destroy_decompress(&cinfo);

								SYS_UnmapFile(pFileData);

								// read EXIF data
								FileSize_t fs;
								fs.iSize = 0;
								SYS_SetFilePointer( stream, &fs );
								SEXIFData sEXIFData;
								ReadEXIFData( stream, &sEXIFData );
								// do we need to rotate it?
								if ( sEXIFData.eOrientation != -1 )
								{
									RotateImage( pFile, sEXIFData.eOrientation );
								}

								return ( H_FILE )pFile;
							}

							SYS_FreeStreamMemory( pFile->hImageBits );
						}
					}
				}

_j_skip1:
				jpeg_destroy_decompress(&cinfo);

				SYS_UnmapFile(pFileData);
			}

			SYS_free(pFile);
		}
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


const char* GetImageFormatStr( H_SET hSet )
{
	File_t* pFile = ( File_t* )hSet;
	return pFile->pszFormatStr;
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
	return pFile->flags;
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
		ppf->eChannelLayout = pFile->eChannelLayout;
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
	GetImageFormatStr,
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



