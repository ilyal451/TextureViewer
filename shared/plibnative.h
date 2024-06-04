

#pragma once

// routines to the native (GDI) format: BGRX8888

#include "../types.h"

typedef unsigned int uint_t;
typedef unsigned short word_t;
typedef unsigned char byte_t;
typedef unsigned char bool_t;
typedef struct rect_s
{
	int left, top, right, bottom;
} rect_t;

typedef enum SharpenTypes
{
	SHARPEN_NONE,
	SHARPEN_X4, // 4 pixels
	SHARPEN_X8, // 8 pixels
	SHARPEN_X8A, // 6(8) pixels (4 + (4 / 2))
};

typedef void *HTF;

typedef struct PLibNativeFuncs
{
	void ( *pfnShrinkPixelsPoint )( byte_t* dst, int iDstWidth, int iDstHeight, byte_t* src, int iSrcWidth, int iSrcHeight );
	void ( *pfnShrinkPixelsLinear )( byte_t* dst, int iDstWidth, int iDstHeight, byte_t* src, int iSrcWidth, int iSrcHeight, int eSharpeningType, int iBlurControl );
	void ( *pfnCopyPixels )( uint_t* dst, int iDstWidth, int iDstHeight, rect_t* prectDst, uint_t* src, int iSrcWidth, int iSrcHeight, rect_t* prectSrc );
	void ( *pfnStretchPixelsPoint )( uint_t* dst, int iDstWidth, int iDstHeight, rect_t* prectDst, uint_t* src, int iSrcWidth, int iSrcHeight, rect_t* prectSrc );

	void ( *pfnExtractRed )( void* dst, int iCount );
	void ( *pfnExtractGreen )( void* dst, int iCount );
	void ( *pfnExtractBlue )( void* dst, int iCount );
	void ( *pfnExtractAlpha )( void* dst, int iCount );
	void ( *pfnOverlayAlpha )( void* dst, int iCount, uint_t clrAlpha, int iOpacity );

} PLibNativeFuncs_t;

typedef PLibNativeFuncs_t* ( *PFNPLIBGETNATIVEFUNCS )( void );

