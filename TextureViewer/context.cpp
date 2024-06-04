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
#include <stdio.h>

#include "../shared/utils.h"
#include "../shared/plib.h"

#include "system.h"

#include "settings.h"

#include "settingsbuf.h"
#include "keyvalue.h"

#include "context.h"

#include "viewer.h"

#include "filelist.h"
#include "processing.h"

// the core

// NOTE: this module may require refactoring since some old subsystems have been
// removed lately


extern PLibFuncs_t* g_plibfuncs;

CContext* g_pContext;



ProcessingOptions_t g_fo;

void ResetProcessingOptions( void )
{
	g_fo.bEnableProcessing = false;
	g_fo.afShuffleMask = DEFAULT_SHUFFLE_MASK;
	g_fo.bOverrideGamma = false;
	g_fo.flRangeMin = 0.0;
	g_fo.flRangeMax = 1.0;
	g_fo.eRotateType = RT_0;
	g_fo.bFlipWidth = false;
	g_fo.bFlipHeight = false;
	g_fo.gmInputGamma.eGamma = GM_SRGB;
	g_fo.gmInputGamma.flGamma = 2.2f;
	g_fo.gmOutputGamma.eGamma = GM_SRGB;
	g_fo.gmOutputGamma.flGamma = 2.2f;
}




//
// CFile
//

// file flags
#define FILE_MARKED 0x01


CFile::CFile( const wchar_t* pszFileName, CFormat* pFormat )
{
	m_pszFileName = AllocStringW( pszFileName );
	m_pFormat = pFormat;
	m_hkvbufSettings = pFormat->AllocSettingsBuffer();
	m_hkvbufMetadata = NULL;
	m_iSet = 0;
	m_iTexture = 0;
	m_iMIPMap = 0;
	m_iSlice = 0;
	m_bLoaded = false;
	m_hFile = NULL;
	m_hSet = NULL;
}


CFile::~CFile()
{
	if ( IsLoaded() )
	{
		__DEBUG_BREAK;
	}

	FreeMemory( m_pszFileName );

	KeyValue_DeleteKeyValueBuffer( m_hkvbufSettings );
}


int CFile::GetId( void )
{
	return m_iId;
}


void CFile::SetId( int iId )
{
	m_iId = iId;
}


void CFile::Load( wchar_t* pszSourcePath )
{
	if ( IsLoaded() )
	{
		__DEBUG_BREAK;
	}

	wchar_t szPath[MAX_PATH];
	wsprintf( szPath, L"%s/%s", pszSourcePath, m_pszFileName );

	m_hkvbufMetadata = KeyValue_CreateKeyValueBuffer();

	HF stream = SYS_OpenFile( szPath );
	if ( stream )
	{
		// TODO: settings
		m_hFile = m_pFormat->GetFuncs()->pfnLoadFile( stream, m_hkvbufSettings, m_hkvbufMetadata );
		if ( m_hFile )
		{
			// XXX: we do not support sets currently, but maybe in the future...
			m_hSet = m_pFormat->GetFuncs()->pfnLoadSet( m_hFile, 0 );
		}

		SYS_CloseFile( stream );
	}

	m_bLoaded = true;
}


bool CFile::IsLoaded( void )
{
	return m_bLoaded;
}


bool CFile::IsValidFile( void )
{
	return ( m_hFile != NULL );
}


void CFile::Unload( void )
{
	if ( !IsLoaded() )
	{
		__DEBUG_BREAK;
	}

	if ( IsValidFile() )
	{
		m_pFormat->GetFuncs()->pfnFreeSet( m_hSet );
		m_pFormat->GetFuncs()->pfnFreeFile( m_hFile );
		m_hFile = NULL;
	}

	KeyValue_DeleteKeyValueBuffer( m_hkvbufMetadata );
	m_hkvbufMetadata = NULL;

	m_bLoaded = false;
}

int CFile::GetNumTextures( void )
{
	if ( !IsValidFile() )
	{
		__DEBUG_BREAK;
	}

	return m_pFormat->GetFuncs()->pfnGetArraySize( m_hSet );
}


int CFile::GetTexture( void )
{
	return m_iTexture;
}


void CFile::SetTexture( int iTexture )
{
	if ( m_iTexture != iTexture )
	{
		m_iTexture = iTexture;
	}
}


int CFile::GetNumMIPMaps( void )
{
	if ( !IsValidFile() )
	{
		__DEBUG_BREAK;
	}

	return m_pFormat->GetFuncs()->pfnGetNumMIPMaps( m_hSet );
}


int CFile::GetMIPMap( void )
{
	return m_iMIPMap;
}


void CFile::SetMIPMap( int iMIPMap )
{
	if ( m_iMIPMap != iMIPMap )
	{
		m_iMIPMap = iMIPMap;
	}
}


int CFile::GetNumSlices( void )
{
	if ( !IsValidFile() )
	{
		__DEBUG_BREAK;
	}

	return m_pFormat->GetFuncs()->pfnGetImageDepth( m_hSet, m_iMIPMap );
}

void CFile::SetSlice( int iSlice )
{
	if ( m_iSlice != iSlice )
	{
		m_iSlice = iSlice;
	}
}


int CFile::GetSlice( void )
{
	return m_iSlice;
}


//// TODO:
//bool CFile::IsValidSlice( void )
//{
//	if ( IsValidFile() )
//	{
//		return m_iSlice < GetImageDepth();
//	}
//	else
//	{
//		return false;
//	}
//}


const char* CFile::GetImageFormatStr( void )
{
	if ( !IsValidFile() )
	{
		__DEBUG_BREAK;
	}

	return m_pFormat->GetFuncs()->pfnGetFormatStr( m_hSet );
}


void CFile::GetImageInputGamma( Gamma_t* pcs )
{
	if ( !IsValidFile() )
	{
		__DEBUG_BREAK;
	}

	m_pFormat->GetFuncs()->pfnGetInputGamma( m_hSet, pcs );
}


void CFile::GetImageOutputGamma( Gamma_t* pcs )
{
	if ( !IsValidFile() )
	{
		__DEBUG_BREAK;
	}

	m_pFormat->GetFuncs()->pfnGetOutputGamma( m_hSet, pcs );
}


int CFile::GetImageFlags( void )
{
	if ( !IsValidFile() )
	{
		__DEBUG_BREAK;
	}

	return m_pFormat->GetFuncs()->pfnGetImageFlags( m_hSet );
}


int CFile::GetNumPaletteColors( void )
{
	if ( !IsValidFile() )
	{
		__DEBUG_BREAK;
	}

	return m_pFormat->GetFuncs()->pfnGetNumPaletteColors( m_hSet );
}


void CFile::GetPaletteData( void* buffer )
{
	// TODO:
}


int CFile::GetImageWidth( void )
{
	if ( !IsValidFile() )
	{
		__DEBUG_BREAK;
	}

	return m_pFormat->GetFuncs()->pfnGetImageWidth( m_hSet, m_iMIPMap );
}


int CFile::GetImageHeight( void )
{
	if ( !IsValidFile() )
	{
		__DEBUG_BREAK;
	}

	return m_pFormat->GetFuncs()->pfnGetImageHeight( m_hSet, m_iMIPMap );
}


int GetRotateOpts( ProcessingOptions_t& fo )
{
	int opts = 0;

	if ( fo.eRotateType == RT_90CW )
		opts |= DPO_SWAP_XY | DPO_SWAP_LEFT_RIGHT;
	else if ( fo.eRotateType == RT_90CCW )
		opts |= DPO_SWAP_XY | DPO_SWAP_TOP_BOTTOM;
	else if ( fo.eRotateType == RT_180 )
		opts |= DPO_SWAP_LEFT_RIGHT | DPO_SWAP_TOP_BOTTOM;
	if ( fo.bFlipWidth )
	{
		if ( opts & DPO_SWAP_XY )
			opts ^= DPO_SWAP_TOP_BOTTOM;
		else
			opts ^= DPO_SWAP_LEFT_RIGHT;
	}
	if ( fo.bFlipHeight )
	{
		if ( opts & DPO_SWAP_XY )
			opts ^= DPO_SWAP_LEFT_RIGHT;
		else
			opts ^= DPO_SWAP_TOP_BOTTOM;
	}

	return opts;
}


bool FSwapXY( ProcessingOptions_t& fo )
{
	return fo.bEnableProcessing && ( fo.eRotateType == RT_90CW || fo.eRotateType == RT_90CCW );
}


int CFile::GetRotatedImageWidth( ProcessingOptions_t& fo )
{
	return FSwapXY( fo ) ? GetImageHeight() : GetImageWidth();
}


int CFile::GetRotatedImageHeight( ProcessingOptions_t& fo )
{
	return FSwapXY( fo ) ? GetImageWidth() : GetImageHeight();
}


int CFile::GetMaxBufferWidth( void )
{
	return GetImageWidth();
}


int CFile::GetMaxBufferHeight( void )
{
	int iWidth = GetImageWidth();
	int iHeight = GetImageHeight();
	int v = 0x10000 / iWidth;
	return min( iHeight, max( 1, v ) );
}

/*
int CFile::GetMaxRotatedBufferWidth( ProcessingOptions_t& fo )
{
	return FSwapXY( fo ) ? GetMaxBufferHeight() : GetMaxBufferWidth();
}


int CFile::GetMaxRotatedBufferHeight( ProcessingOptions_t& fo )
{
	return FSwapXY( fo ) ? GetMaxBufferWidth() : GetMaxBufferHeight();
}
*/

bool CFile::IsValidImage( void )
{
	return true;
}


int CFile::GetImageOffsetPixels( void )
{
	const FormatFuncs_t* pFuncs = m_pFormat->GetFuncs();
	int nTextures = GetNumTextures();
	int iMIPMapOffsetPixels = 0;
	int iPitchPixels;
	int iSliceSizePixels;
	int iImageSizePixels;
	for ( int i = 0; i <= m_iMIPMap; i++ )
	{
		iPitchPixels = pFuncs->pfnGetImageWidth( m_hSet, i );
		iSliceSizePixels = iPitchPixels * pFuncs->pfnGetImageHeight( m_hSet, i );
		iImageSizePixels = iSliceSizePixels * pFuncs->pfnGetImageDepth( m_hSet, i );
		if ( i == m_iMIPMap )
			break;
		iMIPMapOffsetPixels += iImageSizePixels * nTextures;
	}

	return iMIPMapOffsetPixels + iImageSizePixels * m_iTexture + iSliceSizePixels * m_iSlice;
}


#define OUTPUT_PIXEL_SIZE 4
#define OUTPUT_PIXEL_SIZE_FLOAT 16

// processing info (can't store it locally, alas)
bool g_bAnalysisPass = false;
Rect_t g_rectAnalysisRect;
float g_flRangeMin;
float g_flRangeMax;
Gamma_t g_gmInputGamma;
Gamma_t g_gmOutputGamma;
int g_aiChannelMask[NUM_CHANNELS];
int g_iOutputWidth;
int g_iOutputHeight;
void* g_pOutputBuffer;
void* g_imageBufferOutput;
Rect_t g_rectImageOutput;

// TODO: make a thread context finally...
float g_aflAnalysisResponseMin[MAX_THREADS][NUM_CHANNELS];
float g_aflAnalysisResponseMax[MAX_THREADS][NUM_CHANNELS];

#define PROCESS_PIXELS_NUM_BUFFERS 4
#define PROCESS_PIXELS_BUFFER_CHECK 0xCBAE459F // just some random signature...

// shared for all threads
typedef struct ContextInfo_s
{
	int iImageWidth;
	int iImageHeight;
	int iRotatedWidth;
	int iRotatedHeight;
	void* paletteBuffer;
	int unpackOpts;
	unsigned int afShuffleMask;
	bool bProcess;
	bool bSwapXY;
	int nImageStreams;
	int aiPixelSize[MAX_STREAMS];
	int aiImageLineSize[MAX_STREAMS];
	int aiRectOffset[MAX_STREAMS]; // the calculated offset to the image pixels (left,top position of the crop rect)
	int aiRectLineSize[MAX_STREAMS];
	PFNUNPACKPIXELS apfnUnpackPixels[MAX_STREAMS];
	HMEM ahImageBits[MAX_STREAMS];
} ContextInfo_t;

// unique for each thread
typedef struct ThreadStruct_s
{
	ContextInfo_t* pci;
	int iStartLine;
	int nLines;
	char* readBuf;
	char* imageBuffer;
} ThreadStruct_t;


void GetOutputRectPos( Rect_t &rect, int iWidth, int iHeight, int iStartLine, int nLines, int opts )
{
	if ( opts & DPO_SWAP_XY )
	{
		if ( opts & DPO_SWAP_LEFT_RIGHT )
		{
			rect.right = iHeight - iStartLine;
			rect.left = rect.right - nLines;
			rect.top = 0;
			rect.bottom = iWidth;
		}
		else
		{
			// simply swap xy
			rect.left = iStartLine;
			rect.right = rect.left + nLines;
			rect.top = 0;
			rect.bottom = iWidth;
		}
	}
	else
	{
		if ( opts & DPO_SWAP_TOP_BOTTOM )
		{
			rect.left = 0;
			rect.right = iWidth;
			rect.bottom = iHeight - iStartLine;
			rect.top = rect.bottom - nLines;
		}
		else
		{
			// set the original
			rect.left = 0;
			rect.right = iWidth;
			rect.top = iStartLine;
			rect.bottom = rect.top + nLines;
		}
	}
}


// this is called when the processing is on
void ProcessPixelsThreadStart1( int iThread, void* param )
{
	ThreadStruct_t* pts = ( ThreadStruct_t* )param;
	ContextInfo_t* pci = pts->pci;

	// setup the rect
	Rect_t rect;
	GetOutputRectPos( rect, pci->iImageWidth, pci->iImageHeight, pts->iStartLine, pts->nLines, pci->unpackOpts );
	int iWidth = rect.right-rect.left;
	int iHeight = rect.bottom-rect.top;
	int nPixels = iWidth * iHeight;

	// mux the pixels
	for ( int iStream = 0; iStream < pci->nImageStreams; iStream++ )
	{
		HMEM hImageBits = pci->ahImageBits[iStream];
		int iImageLineSize = pci->aiImageLineSize[iStream];
		int iRectLineSize = pci->aiRectLineSize[iStream];
		int iOffset = pci->aiRectOffset[iStream];
		iOffset += pts->iStartLine * iImageLineSize;
		int iReadBufOffset = 0;
		for ( int iLine = 0; iLine < pts->nLines; iLine++ )
		{
			SYS_ReadStreamMemory( hImageBits, iOffset, iRectLineSize, &pts->readBuf[iReadBufOffset] );
			iOffset += iImageLineSize;
			iReadBufOffset += iRectLineSize;
		}

		pci->apfnUnpackPixels[iStream]( pts->imageBuffer, pts->readBuf, iWidth, iHeight, 0, pts->nLines, pci->paletteBuffer, pci->unpackOpts );
	}

	if ( pci->afShuffleMask != DEFAULT_SHUFFLE_MASK )
	{
		g_plibfuncs->pfnShuffleChannelsFloat( pts->imageBuffer, nPixels, pci->afShuffleMask );
	}

	//if ( g_bAnalysisPass && ( g_eAnalysisSource == ANALYZE_INPUT ) && g_bAnalysisIncludeGamma )
	//{
	//	g_plibfuncs->pfnAnalyzeResponse( pts->imageBuffer, &rect, &g_rectAnalysisRect, g_aflAnalysisResponseMin[iThread], g_aflAnalysisResponseMax[iThread], g_po.flAnalysisTopLimit, g_po.flAnalysisBottomLimit );
	//	return;
	//}

	g_plibfuncs->pfnDecodeGamma( pts->imageBuffer, nPixels, &g_gmInputGamma );

	if ( g_bAnalysisPass && ( g_eAnalysisSource == ANALYZE_INPUT ) )
	{
		g_plibfuncs->pfnAnalyzeResponse( pts->imageBuffer, &rect, &g_rectAnalysisRect, g_aflAnalysisResponseMin[iThread], g_aflAnalysisResponseMax[iThread], g_flAnalysisTopLimit, g_flAnalysisBottomLimit );
		return;
	}

	//if ( g_po.afEnabled & ENABLE_FLAG_RANGE )
	{
		if ( g_flRangeMin != 0 || g_flRangeMax != 1 )
		{
			g_plibfuncs->pfnExpandRange( pts->imageBuffer, nPixels, g_flRangeMin, g_flRangeMax );
		}
	}

	if ( g_bAnalysisPass && ( g_eAnalysisSource == ANALYZE_OUTPUT ) )
	{
		g_plibfuncs->pfnAnalyzeResponse( pts->imageBuffer, &rect, &g_rectAnalysisRect, g_aflAnalysisResponseMin[iThread], g_aflAnalysisResponseMax[iThread], g_flAnalysisTopLimit, g_flAnalysisBottomLimit );
		return;
	}

	g_plibfuncs->pfnEncodeGamma( pts->imageBuffer, nPixels, &g_gmOutputGamma );

	//if ( g_bAnalysisPass && ( g_po.eAnalysisSource == ANALYZE_OUTPUT ) && g_po.bAnalysisIncludeGamma )
	//{
	//	g_plibfuncs->pfnAnalyzeResponse( pts->imageBuffer, &rect, &g_rectAnalysisRect, g_aflAnalysisResponseMin[iThread], g_aflAnalysisResponseMax[iThread], g_po.flAnalysisTopLimit, g_po.flAnalysisBottomLimit );
	//	return;
	//}

	g_plibfuncs->pfnConvertOutputFloat( g_pOutputBuffer, pts->imageBuffer, g_iOutputWidth, g_iOutputHeight, &rect, g_aiChannelMask );
}


// with no processing
void ProcessPixelsSimpleThreadStart( int iThread, void* param )
{
	ThreadStruct_t* pts = ( ThreadStruct_t* )param;
	ContextInfo_t* pci = pts->pci;

	for ( int iStream = 0; iStream < pci->nImageStreams; iStream++ )
	{
		HMEM hImageBits = pci->ahImageBits[iStream];
		int iImageLineSize = pci->aiImageLineSize[iStream];
		int iRectLineSize = pci->aiRectLineSize[iStream];
		int iOffset = pci->aiRectOffset[iStream];
		iOffset += pts->iStartLine * iImageLineSize;
		int iReadBufOffset = 0;
		for ( int iLine = 0; iLine < pts->nLines; iLine++ )
		{
			SYS_ReadStreamMemory( hImageBits, iOffset, iRectLineSize, &pts->readBuf[iReadBufOffset] );
			iOffset += iImageLineSize;
			iReadBufOffset += iRectLineSize;
		}

		pci->apfnUnpackPixels[iStream]( g_pOutputBuffer, pts->readBuf, g_iOutputWidth, g_iOutputHeight, pts->iStartLine, pts->nLines, pci->paletteBuffer, pci->unpackOpts );
	}
}


// this func can be quite simple if you get rid of the processing feature (ugh)
void CFile::GetImageData( void* buffer )
{
	if ( !IsValidImage() )
		__DEBUG_BREAK;

	const FormatFuncs_t* pFuncs = m_pFormat->GetFuncs();
	PixelFormat_t pf;

	ContextInfo_t ci;

	// setup the processing options
	if ( g_fo.bEnableProcessing )
	{
		ci.unpackOpts = 0;
		ci.unpackOpts |= GetRotateOpts( g_fo );
		ci.bSwapXY = ( ci.unpackOpts & DPO_SWAP_XY ) != 0;

		ci.afShuffleMask = g_fo.afShuffleMask;
		bool bDoShuffle = ( ci.afShuffleMask != DEFAULT_SHUFFLE_MASK );
		g_flRangeMin = g_fo.flRangeMin;
		g_flRangeMax = g_fo.flRangeMax;
		bool bDoRange = ( g_flRangeMin != 0 ) || ( g_flRangeMax != 1 );
		if ( g_fo.bOverrideGamma )
		{
			g_gmInputGamma = g_fo.gmInputGamma;
			g_gmOutputGamma = g_fo.gmOutputGamma;
		}
		else
		{
			GetImageInputGamma( &g_gmInputGamma );
			GetImageOutputGamma( &g_gmOutputGamma );
		}
		bool bDoGamma = ( g_gmInputGamma.eGamma != g_gmOutputGamma.eGamma ) || 
			( g_gmInputGamma.eGamma == GM_SPECIFY && ( g_gmInputGamma.flGamma != g_gmOutputGamma.flGamma ) );
		ci.bProcess = g_bAnalysisPass || bDoShuffle || bDoRange || bDoGamma;
	}
	else
	{
		ci.unpackOpts = 0;
		ci.bSwapXY = false;

		ci.afShuffleMask = DEFAULT_SHUFFLE_MASK;
		g_flRangeMin = 0;
		g_flRangeMax = 1;
		GetImageInputGamma( &g_gmInputGamma );
		GetImageOutputGamma( &g_gmOutputGamma );
		ci.bProcess = false;
	}

	int paletteOpts = ci.unpackOpts;

	ci.paletteBuffer = NULL;

	// if there's a palette
	int nPaletteStreams = pFuncs->pfnGetNumStreams( m_hSet, ST_PALETTE );
	if ( nPaletteStreams )
	{
		int iPaletteSize = GetNumPaletteColors();
		if ( iPaletteSize )
		{
			// alloc an aligned buffer
			int iBufferSize = ci.bProcess ? iPaletteSize * OUTPUT_PIXEL_SIZE_FLOAT : iPaletteSize * OUTPUT_PIXEL_SIZE;
			ci.paletteBuffer = SYS_AllocBuffer( iBufferSize );

			// determine the size for the temporary buffer
			int iMaxSize = 0;
			for ( int iStream = 0; iStream < nPaletteStreams; iStream++ )
			{
				pFuncs->pfnGetStreamPixelFormat( m_hSet, ST_PALETTE, iStream, &pf );
				int iPixelSize = g_plibfuncs->pfnGetPixelSize( &pf );
				int iSize = iPaletteSize * iPixelSize;
				if ( iSize > iMaxSize )
				{
					iMaxSize = iSize;
				}
			}
			void* readBuf = malloc( iMaxSize );

			// for each palette stream
			for ( int iStream = 0; iStream < nPaletteStreams; iStream++ )
			{
				// mux them together
				pFuncs->pfnGetStreamPixelFormat( m_hSet, ST_PALETTE, iStream, &pf );
				PFNUNPACKPIXELS pfnUnpackPixels = ci.bProcess ? g_plibfuncs->pfnGetUnpacker_Float( &pf, iStream ) : g_plibfuncs->pfnGetUnpacker( &pf, iStream );
				int iPixelSize = g_plibfuncs->pfnGetPixelSize( &pf );
				HMEM hPaletteBits = pFuncs->pfnGetStreamBits( m_hSet, ST_PALETTE, iStream );
				SYS_ReadStreamMemory( hPaletteBits, 0, iPaletteSize * iPixelSize, readBuf );

				pfnUnpackPixels( ci.paletteBuffer, readBuf, iPaletteSize, 1, 0, 1, NULL, paletteOpts );
			}

			free( readBuf );
		}
	}

	// set the output width/height depending on the processing options
	ci.iImageWidth = GetImageWidth();
	ci.iImageHeight = GetImageHeight();
	if ( FSwapXY( g_fo ) )
	{
		ci.iRotatedWidth = ci.iImageHeight;
		ci.iRotatedHeight = ci.iImageWidth;
	}
	else
	{
		ci.iRotatedWidth = ci.iImageWidth;
		ci.iRotatedHeight = ci.iImageHeight;
	}

	int nLines = ci.iImageHeight;
	int nChunkLines = GetMaxBufferHeight(); // XXX:
	int nChunkPixels = nChunkLines * ci.iImageWidth;
	int iImageOffsetPixels = GetImageOffsetPixels();

	ThreadStruct_t ats[PROCESS_PIXELS_NUM_BUFFERS];
	for ( int i = 0; i < PROCESS_PIXELS_NUM_BUFFERS; i++ )
	{
		ats[i].pci = &ci;
	}

	char* imageBuffers = NULL;
	if ( ci.bProcess )
	{
		int iSizeReq = OUTPUT_PIXEL_SIZE_FLOAT * nChunkPixels;
		imageBuffers = ( char* )SYS_AllocBuffer( iSizeReq * PROCESS_PIXELS_NUM_BUFFERS );
		for ( int i = 0; i < PROCESS_PIXELS_NUM_BUFFERS; i++ )
		{
			ats[i].imageBuffer = &imageBuffers[i*iSizeReq];
		}
	}

	ci.nImageStreams = pFuncs->pfnGetNumStreams( m_hSet, ST_IMAGE );

	// determine the size for the temporary buffer
	int iMaxSize = 0;
	for ( int iStream = 0; iStream < ci.nImageStreams; iStream++ )
	{
		pFuncs->pfnGetStreamPixelFormat( m_hSet, ST_IMAGE, iStream, &pf );
		ci.aiPixelSize[iStream] = g_plibfuncs->pfnGetPixelSize( &pf );
		ci.aiImageLineSize[iStream] = ci.iImageWidth * ci.aiPixelSize[iStream];
		ci.aiRectOffset[iStream] = iImageOffsetPixels * ci.aiPixelSize[iStream];
		ci.aiRectLineSize[iStream] = ci.iImageWidth * ci.aiPixelSize[iStream];
		ci.apfnUnpackPixels[iStream] = ci.bProcess ? g_plibfuncs->pfnGetUnpacker_Float( &pf, iStream ) : g_plibfuncs->pfnGetUnpacker( &pf, iStream );
		ci.ahImageBits[iStream] = pFuncs->pfnGetStreamBits( m_hSet, ST_IMAGE, iStream );
		int iSize = nChunkPixels * ci.aiPixelSize[iStream];
		if ( iSize > iMaxSize )
		{
			iMaxSize = iSize;
		}
	}
	// TODO: provide checks for malloc
	char* readBufs = ( char* )malloc( iMaxSize * PROCESS_PIXELS_NUM_BUFFERS );
	for ( int i = 0; i < PROCESS_PIXELS_NUM_BUFFERS; i++ )
	{
		ats[i].readBuf = &readBufs[i*iMaxSize];
	}

	const ImageParams_t* paramsOut = g_pContext->GetOutputParams();
	g_iOutputWidth = paramsOut->iImageWidth;
	g_iOutputHeight = paramsOut->iImageHeight;
	g_pOutputBuffer = buffer;
	for ( int i = 0; i < NUM_CHANNELS; i++ )
	{
		g_aiChannelMask[i] = ( paramsOut->aci[i].flags & CHANNEL_VALID ) ? -1 : 0;
	}

	// do the processing, finally
	int nLinesProcessed = 0;
	while ( nLinesProcessed < nLines )
	{
		int iBuffer = 0;
		for ( ; iBuffer < PROCESS_PIXELS_NUM_BUFFERS; iBuffer++ )
		{
			int nLinesToProcess = min( nChunkLines, nLines - nLinesProcessed );

			ats[iBuffer].iStartLine = nLinesProcessed;
			ats[iBuffer].nLines = nLinesToProcess;
			if ( ci.bProcess )
			{
				SYS_CallThread( ProcessPixelsThreadStart1, ( void* )&ats[iBuffer] );
			}
			else
			{
				SYS_CallThread( ProcessPixelsSimpleThreadStart, ( void* )&ats[iBuffer] );
			}

			nLinesProcessed += nLinesToProcess;
			if ( nLinesProcessed >= nLines )
			{
				break;
			}
		}

		// wait till we can reuse the buffers
		SYS_WaitForAllThreads();
	}

	free( readBufs );

	if ( imageBuffers )
	{
		SYS_FreeBuffer( imageBuffers );
	}

	if ( ci.paletteBuffer )
	{
		SYS_FreeBuffer( ci.paletteBuffer );
	}
}


void CFile::Read( SETTINGS settings )
{
	int flags = Settings_ReadByte( settings );
	if ( flags & FILE_MARKED )
	{
		g_pFileList->SetMarked( GetId(), true );
	}
}


void CFile::Write( SETTINGS settings )
{
	int flags = 0;
	if ( g_pFileList->IsMarked( GetId() ) )
	{
		flags |= FILE_MARKED;
	}
	Settings_WriteByte( settings, flags );
}


KEYVALUEBUFFER CFile::GetSettingsBuffer( void )
{
	return m_hkvbufSettings;
}


KEYVALUEBUFFER CFile::GetMetadataBuffer( void )
{
	return m_hkvbufMetadata;
}


void CFile::DoAnalysis( Rect_t* prect, int flags, float* aflMin, float* aflMax )
{
	g_bAnalysisPass = true;

	// the analysis rect
	if ( prect == NULL )
	{
		g_rectAnalysisRect.left = 0;
		g_rectAnalysisRect.right = GetRotatedImageWidth( g_fo );
		g_rectAnalysisRect.top = 0;
		g_rectAnalysisRect.bottom = GetRotatedImageHeight( g_fo );
	}
	else
	{
		g_rectAnalysisRect = *prect;
	}

	// init the limits
	for ( int i = 0; i < SYS_GetNumThreads(); i++ )
	{
		for ( int j = 0; j < NUM_CHANNELS; j++ )
		{
			g_aflAnalysisResponseMin[i][j] = g_flAnalysisTopLimit;
			g_aflAnalysisResponseMax[i][j] = g_flAnalysisBottomLimit;
		}
	}

	// will trigger the analysis
	GetImageData( NULL );

	// dump the results
	for ( int i = 0; i < NUM_CHANNELS; i++ )
	{
		aflMin[i] = g_flAnalysisTopLimit;
		aflMax[i] = g_flAnalysisBottomLimit;
	}
	for ( int i = 0; i < SYS_GetNumThreads(); i++ )
	{
		for ( int j = 0; j < NUM_CHANNELS; j++ )
		{
			if ( aflMin[j] > g_aflAnalysisResponseMin[i][j] )
			{
				aflMin[j] = g_aflAnalysisResponseMin[i][j];
			}
			if ( aflMax[j] < g_aflAnalysisResponseMax[i][j] )
			{
				aflMax[j] = g_aflAnalysisResponseMax[i][j];
			}
		}
	}

	g_bAnalysisPass = false;
}


void CFile::ReadChannelInfo( ChannelInfo_t* aciOut )
{
	ChannelInfo_t aci[NUM_CHANNELS];

	// reset
	for ( int i = 0; i < NUM_CHANNELS; i++ )
	{
		aci[i].flags = 0;
	}

	if ( !IsValidFile() )
	{
		return;
	}

	const FormatFuncs_t* pFuncs = m_pFormat->GetFuncs();

	int nImageStreams = pFuncs->pfnGetNumStreams( m_hSet, ST_IMAGE );
	if ( nImageStreams ) // XXX:
	{
		PixelFormat_t pfImage;
		for ( int iImageStream = 0; iImageStream < nImageStreams; iImageStream++ )
		{
			pFuncs->pfnGetStreamPixelFormat( m_hSet, ST_IMAGE, iImageStream, &pfImage );
			if ( pfImage.eChannelLayout == CL_P )
			{
				// XXX: for every P stream we've got a palette
				int nPaletteStreams = pFuncs->pfnGetNumStreams( m_hSet, ST_PALETTE );
				if ( nPaletteStreams )
				{
					PixelFormat_t pfPalette;
					for ( int iPaletteStream = 0; iPaletteStream < nPaletteStreams; iPaletteStream++ )
					{
						pFuncs->pfnGetStreamPixelFormat( m_hSet, ST_PALETTE, iPaletteStream, &pfPalette );
						g_plibfuncs->pfnGetChannelInfo( &pfPalette, aci );
					}
				}
			}
			else
			{
				g_plibfuncs->pfnGetChannelInfo( &pfImage, aci );
			}
		}
	}

	// reset
	for ( int i = 0; i < NUM_CHANNELS; i++ )
	{
		aci[i].iOriginalCapacity = pFuncs->pfnGetOriginalBitDepth( m_hSet, i );
	}

	// shuffle
	for ( int i = 0; i < NUM_CHANNELS; i++ )
	{
		unsigned int iOutChannel = ( g_fo.afShuffleMask >> ( i * 2 ) ) & 3;
		aciOut[i] = aci[iOutChannel];
	}
}



//
// CContext
//


CContext::CContext( void )
{
	g_pContext = this;

	m_pszSourcePath = NULL;

	m_nFiles = 0;
	m_apFile = ( CFile** )malloc( MAX_CONTEXT_FILES * sizeof( CFile* ) );
	for ( int iFile = 0; iFile < MAX_CONTEXT_FILES; iFile++ )
	{
		m_apFile[iFile] = NULL;
	}

	m_iFile = -1;
	m_iNewFile = -1;
	for ( int iPrecacheFile = 0; iPrecacheFile < NUM_PRECACHE_FILES; iPrecacheFile++ )
	{
		m_aiIndices[iPrecacheFile] = -1;
	}

	g_pFileList->SetNumItems(MAX_CONTEXT_FILES);
}


CContext::~CContext()
{
	Reset(); // XXX:

	if ( m_pszSourcePath != NULL )
	{
		FreeMemory( m_pszSourcePath );
	}
}


void CContext::Reset( void )
{
	// make sure no thread is running
	CancelJob();

	// free precaches
	if ( false ) // used to be singlethreaded
	{
		if ( m_iFile != -1 )
		{
			FreeFile( m_iFile );
		}
	}
	else
	{
		for ( int i = 0; i < NUM_PRECACHE_FILES; i++ )
		{
			if ( m_aiIndices[i] != -1 )
			{
				FreeFile( m_aiIndices[i] );
				m_aiIndices[i] = -1;
			}
		}
	}

	m_iFile = -1;

	// free files
	for ( int iFile = 0; iFile < m_nFiles; iFile++ )
	{
		delete m_apFile[iFile];
	}
	m_nFiles = 0;

	// free the source path
	FreeString( m_pszSourcePath );
	m_pszSourcePath = NULL;

	g_pFileList->Clear();
	g_pFileList->SetNumItems(MAX_CONTEXT_FILES);
}


bool CContext::SetSourcePath( const wchar_t* pszPath )
{
	DWORD attrs = GetFileAttributes( pszPath );
	if ( attrs == -1 )
	{
		return false;
	}
	if ( !( attrs & FILE_ATTRIBUTE_DIRECTORY ) )
	{
		return false;
	}

	Reset();

	m_pszSourcePath = AllocStringW( pszPath );
	GeneralizePath( m_pszSourcePath );

	return true;
}


const wchar_t* CContext::GetSourcePath( void )
{
	return m_pszSourcePath;
}


void CContext::PopulateSourceList( const wchar_t* pszFilter )
{
	wchar_t szSearchPath[MAX_PATH];
	//wchar_t* pszFileName;
	HANDLE hFind;
	WIN32_FIND_DATA fd;
	int i;

	if ( !m_pszSourcePath )
	{
		return;
	}

	wsprintf( szSearchPath, L"%s\\%s", m_pszSourcePath, pszFilter == NULL ? L"*" : pszFilter );

	hFind = FindFirstFile( szSearchPath, &fd );
	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			// skip .. and .
			for ( i = 0; fd.cFileName[i] == '.'; i++ )
			{
				// do nothing
			}
			if ( fd.cFileName[i] == '\0' )
			{
				continue;
			}

			if ( !( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
			{
				AddFile( fd.cFileName );
			}
		}
		while ( FindNextFile( hFind, &fd ) );

		FindClose( hFind );
	}
}


int CContext::AddFile( const wchar_t* pszFileName )
{
	if ( GetNumFiles() >= MAX_CONTEXT_FILES )
	{
		NONFATAL_ERROR( L"Failed to add %s. Too many files (%d max).", pszFileName, MAX_CONTEXT_FILES );
		return -1;
	}

	// TODO: since we use the save to project now, check exists

	const wchar_t* pszExt = GetExtension( pszFileName );
	if ( pszExt != NULL )
	{
		int iFormat = FindFormatFor( pszExt );
		if ( iFormat != -1 )
		{
			CFormat* pFormat = GetFormat( iFormat );
			pFormat->LoadDLL(); // ensure the dll is loaded here

			int iFile = m_nFiles++;
			m_apFile[iFile] = new CFile( pszFileName, pFormat );
			m_apFile[iFile]->SetId( iFile );

			g_pFileList->AddItem( pszFileName, iFile );

			return iFile;
		}
	}

	return -1;
}


// XXX: note, we aren't removing files currently
// (the file list dialog in reality just hides them)
void CContext::RemoveFile( int iFile )
{
	// TODO: make sure any job is completed

	if ( iFile < 0 || iFile >= m_nFiles )
	{
		__DEBUG_BREAK;
	}

	// unload if loaded
	// XXX: until the MT is fixed use this
	if ( false )
	{
		if ( iFile == m_iFile )
		{
			FreeFile( m_iFile );
			// do not uninit the id, it is used later (below)
		}
	}
	else
	{
		for ( int i = 0; i < NUM_PRECACHE_FILES; i++ )
		{
			if ( m_aiIndices[i] == iFile )
			{
				FreeFile( m_aiIndices[i] );
				m_aiIndices[i] = -1;
			}
		}
	}

	delete m_apFile[iFile];

	// shift the things up
	for ( int i = iFile; i < m_nFiles-1; i++ )
	{
		m_apFile[i] = m_apFile[i+1];
		m_apFile[i]->SetId( i );
	}

	// fix the ids
	if ( m_iFile > iFile )
	{
		m_iFile--;
	}
	for ( int i = 0; i < NUM_PRECACHE_FILES; i++ )
	{
		if ( m_aiIndices[i] > iFile )
		{
			m_aiIndices[i]--;
		}
	}

	m_nFiles--;

	if ( m_iFile == iFile )
	{
		m_iFile = -1; // assume already freed
		//SetFile( min( iFile, m_nFiles-1 ) ); -- do not call self
	}
}


int CContext::GetNumFiles( void )
{
	return m_nFiles;
}


int CContext::GetFile( void )
{
	if ( m_iFile == -1 )
	{
		__DEBUG_BREAK;
	}
	return m_iFile;
}


const wchar_t* CContext::GetFileName( int iFile )
{
	return m_apFile[iFile]->GetFileName();
}


CFile* CContext::GetFile( int iFile )
{
	return m_apFile[iFile];
}


void CContext::LoadFile( int iFile )
{
	m_apFile[iFile]->Load( m_pszSourcePath );
}


void CContext::FreeFile( int iFile )
{
	m_apFile[iFile]->Unload();
}


HANDLE g_hThreadStartEvent1;
HANDLE g_hJobDoneEvent;
HANDLE g_hFinishedExecutionEvent;
int g_bStopJob;
CRITICAL_SECTION g_csLoader;
HANDLE g_hLoaderThread;
CContext* g_pLoaderContext;


void CContext::CancelJob( void )
{
	// make sure the thread won't go into anymore loops
	ResetEvent( g_hThreadStartEvent1 );

	// (if) the thread is working yet, a critical section is required
	EnterCriticalSection( &g_csLoader );
	// this will tell the thread to finish its job as quick as possible
	g_bStopJob = 1;
	LeaveCriticalSection( &g_csLoader );

	// wait for the thread to stop
	WaitForSingleObject( g_hFinishedExecutionEvent, INFINITE );
}


/*
-- this used to be the single threaded version
-- still may be used for debug
void CContext::SetFile( int iFile )
{
	if ( m_iFile != -1 )
	{
		FreeFile( m_iFile );
	}

	m_iFile = iFile;

	if ( m_iFile != -1 )
	{
		LoadFile( m_iFile );
		UpdateInputParams( false, false );
	}

	g_pFileList->SetSel( m_iFile );
}
*/


void CContext::SetFile( int iFile )
{
	// cancel the job and wait for the thread to finish
	CancelJob();

	// now we can freely modify the frame
	m_iNewFile = iFile;
	g_bStopJob = 0;
	g_pLoaderContext = this; // XXX: is there any need for this?

	// the thread will set these when it finishes
	ResetEvent( g_hJobDoneEvent );
	ResetEvent( g_hFinishedExecutionEvent );

	// let the thread to begin
	SetEvent( g_hThreadStartEvent1 );

	// wait for the file is loaded
	WaitForSingleObject( g_hJobDoneEvent, INFINITE );

	if ( m_iFile != -1 )
	{
		UpdateInputParams( false, false );
		g_pFileSettings->SetFile( GetFile( m_iFile ) );
		
		// XXX:
		//g_pFileInfo->SetMetadataBuffer( GetFile( m_iFile )->GetMetadataBuffer() );

	}

	g_pFileList->SetSel( m_iFile );	

}


void CContext::LoaderThread( void )
{
	// this will switch precache on and off
	int nFilesToPrecache = g_bPrecacheFiles ? NUM_PRECACHE_FILES : 1;

	// determine the direction of the change
	int iDir;
	if ( m_iFile == -1 || m_iNewFile == -1 )
	{
		iDir = 1; // forward
	}
	else
	{
		// special wrap around cases
		if ( ( m_iFile == 0 ) && ( m_iNewFile == ( m_nFiles - 1 ) ) )
			iDir = -1;
		else if ( ( m_iFile == ( m_nFiles - 1 ) ) && ( m_iNewFile == 0 ) )
			iDir = 1;
		else
		{
			// the standard case
			// XXX: make it always forward when the difference is more than 1 file?
			iDir = m_iNewFile - m_iFile;
			if ( iDir < 0 )
				iDir = -1;
			else
				iDir = 1;
		}
	}

	// determine the new indices (the frame)
	int aiNewIndices[NUM_PRECACHE_FILES];
	for ( int i = 0; i < NUM_PRECACHE_FILES; i++ )
	{
		aiNewIndices[i] = -1;
	}
	for ( int i = 0; i < nFilesToPrecache; i++ )
	{
		if ( m_iNewFile == -1 )
		{
			// do nothing
		}
		else if ( i >= m_nFiles ) // HACK: no point in loading more indices than the amount of the files we have
		{
			// do nothing
		}
		else
		{
			int iFile;
			switch ( i )
			{
			case 0:
				iFile = m_iNewFile; // the requested file
				break;
			case 1:
				iFile = m_iNewFile - iDir; // the previous one
				break;
			case 2:
				iFile = m_iNewFile + iDir; // the next one
				break;
			case 3:
				iFile = m_iNewFile - ( iDir * 2 ); // the tail (optimization, when you move the frame back and forth this will prevent the constant swapping)
				break;
			default:
				__DEBUG_BREAK;
			}
			if ( iFile < 0 )
				iFile = m_nFiles + iFile;
			if ( iFile >= m_nFiles )
				iFile = iFile - m_nFiles;
			if ( iFile < 0 || iFile >= m_nFiles )
				__DEBUG_BREAK;

			aiNewIndices[i] = iFile;
		}
	}

	// free the files that need to be freed (that are outside of the frame)
	// (mem usage issue, used to be made at the end of the proc in the past)
	for ( int i = 0; i < NUM_PRECACHE_FILES; i++ )
	{
		if ( m_aiIndices[i] != -1 )
		{
			bool bFound = false;
			for ( int j = 0; j < nFilesToPrecache; j++ )
			{
				if ( aiNewIndices[j] == m_aiIndices[i] )
				{
					bFound = true;
					break;
				}
			}
			if ( !bFound )
			{
				FreeFile( m_aiIndices[i] );
				m_aiIndices[i] = -1;
			}
		}
	}

	// load the files that are not loaded yet
	int bStop = 0;
	for ( int i = 0; i < nFilesToPrecache; i++ )
	{
		if ( aiNewIndices[i] != -1 )
		{
			bool bLoaded = false;

			// search for this index in the old indices
			for ( int j = 0; j < NUM_PRECACHE_FILES; j++ )
			{
				if ( aiNewIndices[i] == m_aiIndices[j] )
				{
					// tell the cleaner that we don't want to clean this yet
					m_aiIndices[j] = -1;

					bLoaded = true;

					break;
				}
			}

			// load if not loaded yet
			if ( !bLoaded )
			{
				if ( !bStop )
				{
					// if the process hasn't been interrupted by the main thread load the file
					LoadFile( aiNewIndices[i] );
				}
				else
				{
					// otherwise finish it quickly
					aiNewIndices[i] = -1;
				}
			}
		}

		// if this is the first file (the requested one) it's time to give it away now
		if ( i == 0 )
		{
			// since it may be polling the value, put a section here
			// it actually can't since it's blocking in another place, but for any case...
			//EnterCriticalSection( &g_csLoader );
			if ( aiNewIndices[i] != m_iNewFile )
				__DEBUG_BREAK;
			m_iFile = aiNewIndices[i];
			//LeaveCriticalSection( &g_csLoader );

			// OK, now we can grab it
			SetEvent( g_hJobDoneEvent );
		}

		if ( !bStop )
		{
			// poll the stop job indicator
			EnterCriticalSection( &g_csLoader );
			bStop = g_bStopJob;
			LeaveCriticalSection( &g_csLoader );
		}
	}

	// copy the new indices
	for ( int i = 0; i < NUM_PRECACHE_FILES; i++ )
	{
		m_aiIndices[i] = aiNewIndices[i];
	}
}


DWORD WINAPI _LoaderThread(LPVOID param)
{
	while ( true )
	{
		WaitForSingleObject( g_hThreadStartEvent1, INFINITE );
		ResetEvent( g_hThreadStartEvent1 );

		g_pLoaderContext->LoaderThread();

		SetEvent( g_hFinishedExecutionEvent );
	}

	return 0;
}


void InitImageLoader( void )
{
	g_hThreadStartEvent1 = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_hJobDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_hFinishedExecutionEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	InitializeCriticalSection( &g_csLoader );

	DWORD tid;
	g_hLoaderThread = CreateThread(NULL, 0, _LoaderThread, NULL, 0, &tid);
    SetThreadPriority(g_hLoaderThread, THREAD_PRIORITY_LOWEST);
}


//
//
//


void GetDefaultGamma( Gamma_t* pcs );

// make sure this is called either:
// - the file has changed
// - the texture/mip-map has changed
// - (image rotated?)
// in overall this should be called when the output image dimensions have changed
void CContext::UpdateInputParams( bool bForeground, bool bUpdateImage )
{
	if ( GetFile() == -1 )
	{
		__DEBUG_BREAK;
	}

	CFile* pFile = GetFile( GetFile() );

	ImageParams_t params;
	if ( pFile->IsValidFile() )
	{
		pFile->ReadChannelInfo( params.aci );
		params.flags = pFile->GetImageFlags();

		params.iImageWidth = pFile->GetRotatedImageWidth( g_fo );
		params.iImageHeight = pFile->GetRotatedImageHeight( g_fo );
		//params.iMaxBufferWidth = pFile->GetMaxRotatedBufferWidth( g_fo );
		//params.iMaxBufferHeight = pFile->GetMaxRotatedBufferHeight( g_fo );
	}
	else
	{
		// zero reset
		for ( int i = 0; i < NUM_CHANNELS; i++ )
		{
			params.aci[i].flags = 0;
		}
		params.flags = 0;
		params.iImageWidth = 0;
		params.iImageHeight = 0;
		//params.iMaxBufferWidth = 0;
		//params.iMaxBufferHeight = 0;
	}

	memcpy( &m_outputParams, &params, sizeof( ImageParams_t ) );
	if ( bUpdateImage )
	{
		g_pTextureViewer->UpdateImage();
	}
}


const ImageParams_t* CContext::GetOutputParams( void )
{
	return &m_outputParams;
}




//
// load/save
//


#define CONTEXT_FILE_VERSION 1


void _CorruptedSettingsMessageExit( void )
{
	FATAL_ERROR( L"The settings file seems to be corrupted or of a different version" );
}


// the debug marks are used to check if the save file is indeed valid
void ReadMark( SETTINGS settings )
{
	char mark = Settings_ReadByte( settings );
	if ( mark != -1 )
	{
		// invalid settings file
		_CorruptedSettingsMessageExit();
	}
}


void WriteMark( SETTINGS settings )
{
	Settings_WriteByte( settings, -1 );
}


void ReadString( SETTINGS settings, char* buffer, int iBufferMax )
{
	int iLen = Settings_ReadShort( settings );
	if ( iLen+1 > iBufferMax )
	{
		_CorruptedSettingsMessageExit();
	}
	Settings_Read( settings, buffer, iLen );
	buffer[iLen] = '\0';
}


void WriteString( SETTINGS settings, const char* psz )
{
	int iLen = ( int )strlen( psz );
	Settings_WriteShort( settings, iLen );
	Settings_Write( settings, psz, iLen );
}


void ReadStringW( SETTINGS settings, wchar_t* buffer, int iBufferMax )
{
	int iLen = Settings_ReadShort( settings );
	if ( iLen+1 > iBufferMax )
	{
		_CorruptedSettingsMessageExit();
	}
	Settings_Read( settings, buffer, iLen*2 );
	buffer[iLen] = '\0';
}


void WriteStringW( SETTINGS settings, const wchar_t* psz )
{
	int iLen = ( int )wcslen( psz );
	Settings_WriteShort( settings, iLen ); // XXX: int to short
	Settings_Write( settings, psz, iLen*2 );
}


void CContext::Load( const wchar_t* pszFileName )
{
	wchar_t szW[MAX_PATH];
	//char sz[MAX_PATH];

	int iSize;
	void* buffer = ReadFileToBuffer( pszFileName, &iSize );
	if ( buffer == NULL )
	{
		NONFATAL_ERROR( L"Couldn't load %s", pszFileName );
		return;
	}

	SETTINGS settings = Settings_CreateRead( buffer, iSize );

	// check file version
	unsigned char version = Settings_ReadByte( settings );
	if ( version != CONTEXT_FILE_VERSION )
	{
		// newer version?
		NONFATAL_ERROR( L"Unsupported project settings file version" );
		Settings_Destroy( settings );
		free( buffer );
		return;
	}

	// the source path
	ReadStringW( settings, szW, MAX_PATH );
	SetSourcePath( szW );

	// the files
	int nFiles = Settings_ReadShort( settings );
	for ( int i = 0; i < nFiles; i++ )
	{
		ReadStringW( settings, szW, MAX_PATH );
		int iFile = AddFile( szW );
		if ( iFile == -1 )
		{
			// missing file or plugin
			__DEBUG_BREAK;
		}
		else
		{
			if ( iFile != i )
				__DEBUG_BREAK;
			GetFile( iFile )->Read( settings );
		}
	}

	// check correct
	ReadMark( settings );

	// the current file
	int iCurrentFile = Settings_ReadShort( settings );

	// should be a EOF
	if ( !Settings_IsEOF( settings ) )
	{
		_CorruptedSettingsMessageExit();
	}

	Settings_Destroy( settings );
	free( buffer );

	g_pTextureViewer->SetFile( iCurrentFile );
}


void CContext::Save( const wchar_t* pszFileName )
{
	SETTINGS settings = Settings_CreateWrite( false );
	if ( settings == NULL )
	{
		__DEBUG_BREAK;
	}

	// file version
	Settings_WriteByte( settings, CONTEXT_FILE_VERSION );

	// source path
	WriteStringW( settings, GetSourcePath() );

	// files
	Settings_WriteShort( settings, m_nFiles );
	for ( int iFile = 0; iFile < m_nFiles; iFile++ )
	{
		WriteStringW( settings, GetFileName( iFile ) );
		GetFile( iFile )->Write( settings );
	}

	// a debug mark (still needed?)
	WriteMark( settings );

	// current file
	Settings_WriteShort( settings, m_iFile );

	if ( !WriteFileFromBuffer( pszFileName, Settings_GetBuffer( settings ), Settings_GetPos( settings ) ) )
	{
		NONFATAL_ERROR( L"Couldn't write to %s", pszFileName );
	}

	Settings_Destroy( settings );
}


void InitContext( void )
{
	ResetProcessingOptions();

	CContext* pContext = new CContext();
}


