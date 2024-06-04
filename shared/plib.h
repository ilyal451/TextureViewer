
#pragma once

// the pixel library core (used by the viewer)
// this will unpack from a general format to BGRX8888 (native format)
// also it does some internal stuff like shuffling the channels, analyzing, etc.

// Note: some funcs have become obsolete as some parts of the viewer have been
// removed lately...

#include "../types.h"

// opts
#define DPO_INT_NORM		0x01	// treat int as a normalized number (signed or unsigned); will map the integer to the range 0.0/1.0 or -1.0/1.0 (depending on the type)
#define DPO_SIGNED_RANGE	0x02	// convert singed values to unsigned by shifting and compressing the range (so that -1.0 becomes 0.0, 0.0 becomes 0.5, etc.)
#define DPO_SWAP_XY			0x10
#define DPO_SWAP_LEFT_RIGHT	0x20
#define DPO_SWAP_TOP_BOTTOM	0x40


typedef void ( *PFNUNPACKPIXELS )( void* pOut, void* pIn, int iImageWidth, int iImageHeight, int iStartLine, int nLines, void* pPal, int opts );

typedef struct PLibFuncs
{
	PFNUNPACKPIXELS ( *pfnGetUnpacker )( PixelFormat_t* ppf, int bOverlay );
	PFNUNPACKPIXELS ( *pfnGetUnpacker_Float )( PixelFormat_t* ppf, int bOverlay );
	int ( *pfnGetPixelSize )( PixelFormat_t* ppf );
	void ( *pfnGetChannelInfo )( PixelFormat_t* ppf, ChannelInfo_t* aci );

	void ( *pfnExpandRange )( void* pIn, int iCount, float flRangeMin, float flRangeMax );
	void ( *pfnScale )( void* pIn, int iCount, float* afl );
	void ( *pfnExponentiate )( void* pIn, int iCount, float fl );

	void ( *pfnAnalyzeResponse )( void* buffer, Rect_t* prectIn, Rect_t* prectAnalysis, float* aflResponseMin, float* aflResponseMax, float flTopLimit, float flBottomLimit );

	void ( *pfnShuffleChannelsFloat )( void* pIn, int iCount, unsigned int afMask );
	void ( *pfnShuffleChannels )( void* pIn, int iCount, unsigned int afMask );

	// pIn is 4*float for RGBA
	void ( *pfnDecodeGamma )( void* pIn, int iCount, Gamma_t* pcs );
	void ( *pfnEncodeGamma )( void* pIn, int iCount, Gamma_t* pcs );
	void ( *pfnConvertGamma )( void* pIn, int iCount, Gamma_t* pcs );

	// XXX: this one should be obsolete
	// pOut is 4*byte for RGBA
	// pIn is 4*float for RGBA
	// iWidth and iHeight is the image dimensions for the output buffer
	// prect is the image part to copy (the width and height of the rect are the dimensions of the input buffer)
	void ( *pfnConvertOutputFloat )( void* pOut, void* pIn, int iWidth, int iHeight, Rect_t* prect, int* aiChannelMask );

} PLibFuncs_t;

typedef PLibFuncs_t* ( *PFNPLIBGETFUNCS )( void );
typedef void ( *PFNPLIBINITLIBRARY )( void );

