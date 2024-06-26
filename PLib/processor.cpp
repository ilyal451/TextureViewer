/*
PLib (Pixel Library), part of the Texture Viewer project
http://imagetools.itch.io/texview
Copyright (c) 2013-2024 Ilya Lyutin

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

#include "../shared/plib.h"

#define DEFINE_UNPACKER(name) \
	void PLIB_Unpack##name( void* pOut, void* pIn, int iImageWidth, int iImageHeight, int iStartLine, int nLines, void* pPal, int opts ); \
	void PLIB_UnpackO##name( void* pOut, void* pIn, int iImageWidth, int iImageHeight, int iStartLine, int nLines, void* pPal, int opts ); \
	void PLIB_UnpackF##name( void* pOut, void* pIn, int iImageWidth, int iImageHeight, int iStartLine, int nLines, void* pPal, int opts ); \
	void PLIB_UnpackFO##name( void* pOut, void* pIn, int iImageWidth, int iImageHeight, int iStartLine, int nLines, void* pPal, int opts )

extern "C" {

DEFINE_UNPACKER( VOID );

//DEFINE_UNPACKER( B8G8R8X8_UNORM );
//DEFINE_UNPACKER( B8G8R8A8_UNORM );
DEFINE_UNPACKER( R9G9B9E5_FLOAT );

DEFINE_UNPACKER( R8_UNORM );
DEFINE_UNPACKER( R8_SNORM );
DEFINE_UNPACKER( R8_UINT );
DEFINE_UNPACKER( R8_SINT );
DEFINE_UNPACKER( R16_UNORM );
DEFINE_UNPACKER( R16_SNORM );
DEFINE_UNPACKER( R16_UINT );
DEFINE_UNPACKER( R16_SINT );
DEFINE_UNPACKER( R16_FLOAT );
DEFINE_UNPACKER( R32_UNORM );
DEFINE_UNPACKER( R32_SNORM );
DEFINE_UNPACKER( R32_UINT );
DEFINE_UNPACKER( R32_SINT );
DEFINE_UNPACKER( R32_FLOAT );

DEFINE_UNPACKER( G8_UNORM );
DEFINE_UNPACKER( G8_SNORM );
DEFINE_UNPACKER( G8_UINT );
DEFINE_UNPACKER( G8_SINT );
DEFINE_UNPACKER( G16_UNORM );
DEFINE_UNPACKER( G16_SNORM );
DEFINE_UNPACKER( G16_UINT );
DEFINE_UNPACKER( G16_SINT );
DEFINE_UNPACKER( G16_FLOAT );
DEFINE_UNPACKER( G32_UNORM );
DEFINE_UNPACKER( G32_SNORM );
DEFINE_UNPACKER( G32_UINT );
DEFINE_UNPACKER( G32_SINT );
DEFINE_UNPACKER( G32_FLOAT );

DEFINE_UNPACKER( B8_UNORM );
DEFINE_UNPACKER( B8_SNORM );
DEFINE_UNPACKER( B8_UINT );
DEFINE_UNPACKER( B8_SINT );
DEFINE_UNPACKER( B16_UNORM );
DEFINE_UNPACKER( B16_SNORM );
DEFINE_UNPACKER( B16_UINT );
DEFINE_UNPACKER( B16_SINT );
DEFINE_UNPACKER( B16_FLOAT );
DEFINE_UNPACKER( B32_UNORM );
DEFINE_UNPACKER( B32_SNORM );
DEFINE_UNPACKER( B32_UINT );
DEFINE_UNPACKER( B32_SINT );
DEFINE_UNPACKER( B32_FLOAT );

DEFINE_UNPACKER( A8_UNORM );
DEFINE_UNPACKER( A8_SNORM );
DEFINE_UNPACKER( A8_UINT );
DEFINE_UNPACKER( A8_SINT );
DEFINE_UNPACKER( A16_UNORM );
DEFINE_UNPACKER( A16_SNORM );
DEFINE_UNPACKER( A16_UINT );
DEFINE_UNPACKER( A16_SINT );
DEFINE_UNPACKER( A16_FLOAT );
DEFINE_UNPACKER( A32_UNORM );
DEFINE_UNPACKER( A32_SNORM );
DEFINE_UNPACKER( A32_UINT );
DEFINE_UNPACKER( A32_SINT );
DEFINE_UNPACKER( A32_FLOAT );

DEFINE_UNPACKER( L8_UNORM );
DEFINE_UNPACKER( L8_SNORM );
DEFINE_UNPACKER( L8_UINT );
DEFINE_UNPACKER( L8_SINT );
DEFINE_UNPACKER( L16_UNORM );
DEFINE_UNPACKER( L16_SNORM );
DEFINE_UNPACKER( L16_UINT );
DEFINE_UNPACKER( L16_SINT );
DEFINE_UNPACKER( L16_FLOAT );
DEFINE_UNPACKER( L32_UNORM );
DEFINE_UNPACKER( L32_SNORM );
DEFINE_UNPACKER( L32_UINT );
DEFINE_UNPACKER( L32_SINT );
DEFINE_UNPACKER( L32_FLOAT );

DEFINE_UNPACKER( L8A8_UNORM );
DEFINE_UNPACKER( L8A8_SNORM );
DEFINE_UNPACKER( L8A8_UINT );
DEFINE_UNPACKER( L8A8_SINT );
DEFINE_UNPACKER( L16A16_UNORM );
DEFINE_UNPACKER( L16A16_SNORM );
DEFINE_UNPACKER( L16A16_UINT );
DEFINE_UNPACKER( L16A16_SINT );
DEFINE_UNPACKER( L16A16_FLOAT );
DEFINE_UNPACKER( L32A32_UNORM );
DEFINE_UNPACKER( L32A32_SNORM );
DEFINE_UNPACKER( L32A32_UINT );
DEFINE_UNPACKER( L32A32_SINT );
DEFINE_UNPACKER( L32A32_FLOAT );

DEFINE_UNPACKER( P8_UINT );
DEFINE_UNPACKER( P16_UINT );
DEFINE_UNPACKER( P32_UINT );

DEFINE_UNPACKER( I8_UNORM );
DEFINE_UNPACKER( I8_SNORM );
DEFINE_UNPACKER( I8_UINT );
DEFINE_UNPACKER( I8_SINT );
DEFINE_UNPACKER( I16_UNORM );
DEFINE_UNPACKER( I16_SNORM );
DEFINE_UNPACKER( I16_UINT );
DEFINE_UNPACKER( I16_SINT );
DEFINE_UNPACKER( I16_FLOAT );
DEFINE_UNPACKER( I32_UNORM );
DEFINE_UNPACKER( I32_SNORM );
DEFINE_UNPACKER( I32_UINT );
DEFINE_UNPACKER( I32_SINT );
DEFINE_UNPACKER( I32_FLOAT );

DEFINE_UNPACKER( R8G8_UNORM );
DEFINE_UNPACKER( R8G8_SNORM );
DEFINE_UNPACKER( R8G8_UINT );
DEFINE_UNPACKER( R8G8_SINT );
DEFINE_UNPACKER( R16G16_UNORM );
DEFINE_UNPACKER( R16G16_SNORM );
DEFINE_UNPACKER( R16G16_UINT );
DEFINE_UNPACKER( R16G16_SINT );
DEFINE_UNPACKER( R16G16_FLOAT );
DEFINE_UNPACKER( R32G32_UNORM );
DEFINE_UNPACKER( R32G32_SNORM );
DEFINE_UNPACKER( R32G32_UINT );
DEFINE_UNPACKER( R32G32_SINT );
DEFINE_UNPACKER( R32G32_FLOAT );

DEFINE_UNPACKER( G8R8_UNORM );
DEFINE_UNPACKER( G8R8_SNORM );
DEFINE_UNPACKER( G8R8_UINT );
DEFINE_UNPACKER( G8R8_SINT );
DEFINE_UNPACKER( G16R16_UNORM );
DEFINE_UNPACKER( G16R16_SNORM );
DEFINE_UNPACKER( G16R16_UINT );
DEFINE_UNPACKER( G16R16_SINT );
DEFINE_UNPACKER( G16R16_FLOAT );
DEFINE_UNPACKER( G32R32_UNORM );
DEFINE_UNPACKER( G32R32_SNORM );
DEFINE_UNPACKER( G32R32_UINT );
DEFINE_UNPACKER( G32R32_SINT );
DEFINE_UNPACKER( G32R32_FLOAT );

DEFINE_UNPACKER( R8G8B8_UNORM );
DEFINE_UNPACKER( R8G8B8_SNORM );
DEFINE_UNPACKER( R8G8B8_UINT );
DEFINE_UNPACKER( R8G8B8_SINT );
DEFINE_UNPACKER( R16G16B16_UNORM );
DEFINE_UNPACKER( R16G16B16_SNORM );
DEFINE_UNPACKER( R16G16B16_UINT );
DEFINE_UNPACKER( R16G16B16_SINT );
DEFINE_UNPACKER( R16G16B16_FLOAT );
DEFINE_UNPACKER( R32G32B32_UNORM );
DEFINE_UNPACKER( R32G32B32_SNORM );
DEFINE_UNPACKER( R32G32B32_UINT );
DEFINE_UNPACKER( R32G32B32_SINT );
DEFINE_UNPACKER( R32G32B32_FLOAT );

DEFINE_UNPACKER( B8G8R8_UNORM );
DEFINE_UNPACKER( B8G8R8_SNORM );
DEFINE_UNPACKER( B8G8R8_UINT );
DEFINE_UNPACKER( B8G8R8_SINT );
DEFINE_UNPACKER( B16G16R16_UNORM );
DEFINE_UNPACKER( B16G16R16_SNORM );
DEFINE_UNPACKER( B16G16R16_UINT );
DEFINE_UNPACKER( B16G16R16_SINT );
DEFINE_UNPACKER( B16G16R16_FLOAT );
DEFINE_UNPACKER( B32G32R32_UNORM );
DEFINE_UNPACKER( B32G32R32_SNORM );
DEFINE_UNPACKER( B32G32R32_UINT );
DEFINE_UNPACKER( B32G32R32_SINT );
DEFINE_UNPACKER( B32G32R32_FLOAT );

DEFINE_UNPACKER( R8G8B8X8_UNORM );
DEFINE_UNPACKER( R8G8B8X8_SNORM );
DEFINE_UNPACKER( R8G8B8X8_UINT );
DEFINE_UNPACKER( R8G8B8X8_SINT );
DEFINE_UNPACKER( R16G16B16X16_UNORM );
DEFINE_UNPACKER( R16G16B16X16_SNORM );
DEFINE_UNPACKER( R16G16B16X16_UINT );
DEFINE_UNPACKER( R16G16B16X16_SINT );
DEFINE_UNPACKER( R16G16B16X16_FLOAT );
DEFINE_UNPACKER( R32G32B32X32_UNORM );
DEFINE_UNPACKER( R32G32B32X32_SNORM );
DEFINE_UNPACKER( R32G32B32X32_UINT );
DEFINE_UNPACKER( R32G32B32X32_SINT );
DEFINE_UNPACKER( R32G32B32X32_FLOAT );

DEFINE_UNPACKER( B8G8R8X8_UNORM );
DEFINE_UNPACKER( B8G8R8X8_SNORM );
DEFINE_UNPACKER( B8G8R8X8_UINT );
DEFINE_UNPACKER( B8G8R8X8_SINT );
DEFINE_UNPACKER( B16G16R16X16_UNORM );
DEFINE_UNPACKER( B16G16R16X16_SNORM );
DEFINE_UNPACKER( B16G16R16X16_UINT );
DEFINE_UNPACKER( B16G16R16X16_SINT );
DEFINE_UNPACKER( B16G16R16X16_FLOAT );
DEFINE_UNPACKER( B32G32R32X32_UNORM );
DEFINE_UNPACKER( B32G32R32X32_SNORM );
DEFINE_UNPACKER( B32G32R32X32_UINT );
DEFINE_UNPACKER( B32G32R32X32_SINT );
DEFINE_UNPACKER( B32G32R32X32_FLOAT );

DEFINE_UNPACKER( R8G8B8A8_UNORM );
DEFINE_UNPACKER( R8G8B8A8_SNORM );
DEFINE_UNPACKER( R8G8B8A8_UINT );
DEFINE_UNPACKER( R8G8B8A8_SINT );
DEFINE_UNPACKER( R16G16B16A16_UNORM );
DEFINE_UNPACKER( R16G16B16A16_SNORM );
DEFINE_UNPACKER( R16G16B16A16_UINT );
DEFINE_UNPACKER( R16G16B16A16_SINT );
DEFINE_UNPACKER( R16G16B16A16_FLOAT );
DEFINE_UNPACKER( R32G32B32A32_UNORM );
DEFINE_UNPACKER( R32G32B32A32_SNORM );
DEFINE_UNPACKER( R32G32B32A32_UINT );
DEFINE_UNPACKER( R32G32B32A32_SINT );
DEFINE_UNPACKER( R32G32B32A32_FLOAT );

DEFINE_UNPACKER( B8G8R8A8_UNORM );
DEFINE_UNPACKER( B8G8R8A8_SNORM );
DEFINE_UNPACKER( B8G8R8A8_UINT );
DEFINE_UNPACKER( B8G8R8A8_SINT );
DEFINE_UNPACKER( B16G16R16A16_UNORM );
DEFINE_UNPACKER( B16G16R16A16_SNORM );
DEFINE_UNPACKER( B16G16R16A16_UINT );
DEFINE_UNPACKER( B16G16R16A16_SINT );
DEFINE_UNPACKER( B16G16R16A16_FLOAT );
DEFINE_UNPACKER( B32G32R32A32_UNORM );
DEFINE_UNPACKER( B32G32R32A32_SNORM );
DEFINE_UNPACKER( B32G32R32A32_UINT );
DEFINE_UNPACKER( B32G32R32A32_SINT );
DEFINE_UNPACKER( B32G32R32A32_FLOAT );

}




//typedef void ( *PFNUNPACKPIXELS )( void* pOut, void* pIn, int nPixels, void* pPal, int opts );
PFNUNPACKPIXELS g_apfnUnpackPixels[NUM_CHANNEL_LAYOUTS][NUM_COMPONENT_SIZES][NUM_COMPONENT_TYPES];
PFNUNPACKPIXELS g_apfnUnpackPixelsO[NUM_CHANNEL_LAYOUTS][NUM_COMPONENT_SIZES][NUM_COMPONENT_TYPES];
//typedef void ( *PFNPROCESSPIXELS )( void* pOut, void* pIn, int nPixels, void* pfn );
PFNUNPACKPIXELS g_apfnUnpackPixelsF[NUM_CHANNEL_LAYOUTS][NUM_COMPONENT_SIZES][NUM_COMPONENT_TYPES];
PFNUNPACKPIXELS g_apfnUnpackPixelsFO[NUM_CHANNEL_LAYOUTS][NUM_COMPONENT_SIZES][NUM_COMPONENT_TYPES];
#define SET_UNPACKER(layout, size, type, name) \
	g_apfnUnpackPixels[layout][size][type] = PLIB_Unpack##name; \
	g_apfnUnpackPixelsO[layout][size][type] = PLIB_UnpackO##name; \
	g_apfnUnpackPixelsF[layout][size][type] = PLIB_UnpackF##name; \
	g_apfnUnpackPixelsFO[layout][size][type] = PLIB_UnpackFO##name;

PFNUNPACKPIXELS g_apfnPF_UnpackPixels[NUM_PIXEL_FORMATS];
PFNUNPACKPIXELS g_apfnPF_UnpackPixelsO[NUM_PIXEL_FORMATS];
PFNUNPACKPIXELS g_apfnPF_UnpackPixelsF[NUM_PIXEL_FORMATS];
PFNUNPACKPIXELS g_apfnPF_UnpackPixelsFO[NUM_PIXEL_FORMATS];
#define SET_PF_UNPACKER(fmt, name) \
	g_apfnPF_UnpackPixels[fmt] = PLIB_Unpack##name; \
	g_apfnPF_UnpackPixelsO[fmt] = PLIB_UnpackO##name; \
	g_apfnPF_UnpackPixelsF[fmt] = PLIB_UnpackF##name; \
	g_apfnPF_UnpackPixelsFO[fmt] = PLIB_UnpackFO##name;

int g_aiPixelFormatPixelSize[NUM_PIXEL_FORMATS];
int g_aiPixelFormatValidChannelMask[NUM_PIXEL_FORMATS];
#define SET_PF_INFO(fmt, size, mask) \
	g_aiPixelFormatPixelSize[fmt] = size; \
	g_aiPixelFormatValidChannelMask[fmt] = mask;

int g_aiChannelLayoutPixelSize[NUM_CHANNEL_LAYOUTS];
int g_aiChannelLayoutValidChannelMask[NUM_CHANNEL_LAYOUTS];
#define SET_CL_INFO(fmt, size, mask) \
	g_aiChannelLayoutPixelSize[fmt] = size; \
	g_aiChannelLayoutValidChannelMask[fmt] = mask;


void _PLIB_InitProcessor( void )
{
	SET_UNPACKER( CL_R, CS_8BIT, CT_UNORM, R8_UNORM );
	SET_UNPACKER( CL_R, CS_8BIT, CT_SNORM, R8_SNORM );
	SET_UNPACKER( CL_R, CS_8BIT, CT_UINT, R8_UINT );
	SET_UNPACKER( CL_R, CS_8BIT, CT_SINT, R8_SINT );
	SET_UNPACKER( CL_R, CS_8BIT, CT_FLOAT, VOID );
	SET_UNPACKER( CL_R, CS_16BIT, CT_UNORM, R16_UNORM );
	SET_UNPACKER( CL_R, CS_16BIT, CT_SNORM, R16_SNORM );
	SET_UNPACKER( CL_R, CS_16BIT, CT_UINT, R16_UINT );
	SET_UNPACKER( CL_R, CS_16BIT, CT_SINT, R16_SINT );
	SET_UNPACKER( CL_R, CS_16BIT, CT_FLOAT, R16_FLOAT );
	SET_UNPACKER( CL_R, CS_32BIT, CT_UNORM, R32_UNORM );
	SET_UNPACKER( CL_R, CS_32BIT, CT_SNORM, R32_SNORM );
	SET_UNPACKER( CL_R, CS_32BIT, CT_UINT, R32_UINT );
	SET_UNPACKER( CL_R, CS_32BIT, CT_SINT, R32_SINT );
	SET_UNPACKER( CL_R, CS_32BIT, CT_FLOAT, R32_FLOAT );

	SET_UNPACKER( CL_G, CS_8BIT, CT_UNORM, G8_UNORM );
	SET_UNPACKER( CL_G, CS_8BIT, CT_SNORM, G8_SNORM );
	SET_UNPACKER( CL_G, CS_8BIT, CT_UINT, G8_UINT );
	SET_UNPACKER( CL_G, CS_8BIT, CT_SINT, G8_SINT );
	SET_UNPACKER( CL_G, CS_8BIT, CT_FLOAT, VOID );
	SET_UNPACKER( CL_G, CS_16BIT, CT_UNORM, G16_UNORM );
	SET_UNPACKER( CL_G, CS_16BIT, CT_SNORM, G16_SNORM );
	SET_UNPACKER( CL_G, CS_16BIT, CT_UINT, G16_UINT );
	SET_UNPACKER( CL_G, CS_16BIT, CT_SINT, G16_SINT );
	SET_UNPACKER( CL_G, CS_16BIT, CT_FLOAT, G16_FLOAT );
	SET_UNPACKER( CL_G, CS_32BIT, CT_UNORM, G32_UNORM );
	SET_UNPACKER( CL_G, CS_32BIT, CT_SNORM, G32_SNORM );
	SET_UNPACKER( CL_G, CS_32BIT, CT_UINT, G32_UINT );
	SET_UNPACKER( CL_G, CS_32BIT, CT_SINT, G32_SINT );
	SET_UNPACKER( CL_G, CS_32BIT, CT_FLOAT, G32_FLOAT );

	SET_UNPACKER( CL_B, CS_8BIT, CT_UNORM, B8_UNORM );
	SET_UNPACKER( CL_B, CS_8BIT, CT_SNORM, B8_SNORM );
	SET_UNPACKER( CL_B, CS_8BIT, CT_UINT, B8_UINT );
	SET_UNPACKER( CL_B, CS_8BIT, CT_SINT, B8_SINT );
	SET_UNPACKER( CL_B, CS_8BIT, CT_FLOAT, VOID );
	SET_UNPACKER( CL_B, CS_16BIT, CT_UNORM, B16_UNORM );
	SET_UNPACKER( CL_B, CS_16BIT, CT_SNORM, B16_SNORM );
	SET_UNPACKER( CL_B, CS_16BIT, CT_UINT, B16_UINT );
	SET_UNPACKER( CL_B, CS_16BIT, CT_SINT, B16_SINT );
	SET_UNPACKER( CL_B, CS_16BIT, CT_FLOAT, B16_FLOAT );
	SET_UNPACKER( CL_B, CS_32BIT, CT_UNORM, B32_UNORM );
	SET_UNPACKER( CL_B, CS_32BIT, CT_SNORM, B32_SNORM );
	SET_UNPACKER( CL_B, CS_32BIT, CT_UINT, B32_UINT );
	SET_UNPACKER( CL_B, CS_32BIT, CT_SINT, B32_SINT );
	SET_UNPACKER( CL_B, CS_32BIT, CT_FLOAT, B32_FLOAT );

	SET_UNPACKER( CL_A, CS_8BIT, CT_UNORM, A8_UNORM );
	SET_UNPACKER( CL_A, CS_8BIT, CT_SNORM, A8_SNORM );
	SET_UNPACKER( CL_A, CS_8BIT, CT_UINT, A8_UINT );
	SET_UNPACKER( CL_A, CS_8BIT, CT_SINT, A8_SINT );
	SET_UNPACKER( CL_A, CS_8BIT, CT_FLOAT, VOID );
	SET_UNPACKER( CL_A, CS_16BIT, CT_UNORM, A16_UNORM );
	SET_UNPACKER( CL_A, CS_16BIT, CT_SNORM, A16_SNORM );
	SET_UNPACKER( CL_A, CS_16BIT, CT_UINT, A16_UINT );
	SET_UNPACKER( CL_A, CS_16BIT, CT_SINT, A16_SINT );
	SET_UNPACKER( CL_A, CS_16BIT, CT_FLOAT, A16_FLOAT );
	SET_UNPACKER( CL_A, CS_32BIT, CT_UNORM, A32_UNORM );
	SET_UNPACKER( CL_A, CS_32BIT, CT_SNORM, A32_SNORM );
	SET_UNPACKER( CL_A, CS_32BIT, CT_UINT, A32_UINT );
	SET_UNPACKER( CL_A, CS_32BIT, CT_SINT, A32_SINT );
	SET_UNPACKER( CL_A, CS_32BIT, CT_FLOAT, A32_FLOAT );

	SET_UNPACKER( CL_L, CS_8BIT, CT_UNORM, L8_UNORM );
	SET_UNPACKER( CL_L, CS_8BIT, CT_SNORM, L8_SNORM );
	SET_UNPACKER( CL_L, CS_8BIT, CT_UINT, L8_UINT );
	SET_UNPACKER( CL_L, CS_8BIT, CT_SINT, L8_SINT );
	SET_UNPACKER( CL_L, CS_8BIT, CT_FLOAT, VOID );
	SET_UNPACKER( CL_L, CS_16BIT, CT_UNORM, L16_UNORM );
	SET_UNPACKER( CL_L, CS_16BIT, CT_SNORM, L16_SNORM );
	SET_UNPACKER( CL_L, CS_16BIT, CT_UINT, L16_UINT );
	SET_UNPACKER( CL_L, CS_16BIT, CT_SINT, L16_SINT );
	SET_UNPACKER( CL_L, CS_16BIT, CT_FLOAT, L16_FLOAT );
	SET_UNPACKER( CL_L, CS_32BIT, CT_UNORM, L32_UNORM );
	SET_UNPACKER( CL_L, CS_32BIT, CT_SNORM, L32_SNORM );
	SET_UNPACKER( CL_L, CS_32BIT, CT_UINT, L32_UINT );
	SET_UNPACKER( CL_L, CS_32BIT, CT_SINT, L32_SINT );
	SET_UNPACKER( CL_L, CS_32BIT, CT_FLOAT, L32_FLOAT );

	SET_UNPACKER( CL_LA, CS_8BIT, CT_UNORM, L8A8_UNORM );
	SET_UNPACKER( CL_LA, CS_8BIT, CT_SNORM, L8A8_SNORM );
	SET_UNPACKER( CL_LA, CS_8BIT, CT_UINT, L8A8_UINT );
	SET_UNPACKER( CL_LA, CS_8BIT, CT_SINT, L8A8_SINT );
	SET_UNPACKER( CL_LA, CS_8BIT, CT_FLOAT, VOID );
	SET_UNPACKER( CL_LA, CS_16BIT, CT_UNORM, L16A16_UNORM );
	SET_UNPACKER( CL_LA, CS_16BIT, CT_SNORM, L16A16_SNORM );
	SET_UNPACKER( CL_LA, CS_16BIT, CT_UINT, L16A16_UINT );
	SET_UNPACKER( CL_LA, CS_16BIT, CT_SINT, L16A16_SINT );
	SET_UNPACKER( CL_LA, CS_16BIT, CT_FLOAT, L16A16_FLOAT );
	SET_UNPACKER( CL_LA, CS_32BIT, CT_UNORM, L32A32_UNORM );
	SET_UNPACKER( CL_LA, CS_32BIT, CT_SNORM, L32A32_SNORM );
	SET_UNPACKER( CL_LA, CS_32BIT, CT_UINT, L32A32_UINT );
	SET_UNPACKER( CL_LA, CS_32BIT, CT_SINT, L32A32_SINT );
	SET_UNPACKER( CL_LA, CS_32BIT, CT_FLOAT, L32A32_FLOAT );

	SET_UNPACKER( CL_P, CS_8BIT, CT_UNORM, VOID );
	SET_UNPACKER( CL_P, CS_8BIT, CT_SNORM, VOID );
	SET_UNPACKER( CL_P, CS_8BIT, CT_UINT, P8_UINT );
	SET_UNPACKER( CL_P, CS_8BIT, CT_SINT, VOID );
	SET_UNPACKER( CL_P, CS_8BIT, CT_FLOAT, VOID );
	SET_UNPACKER( CL_P, CS_16BIT, CT_UNORM, VOID );
	SET_UNPACKER( CL_P, CS_16BIT, CT_SNORM, VOID );
	SET_UNPACKER( CL_P, CS_16BIT, CT_UINT, P16_UINT );
	SET_UNPACKER( CL_P, CS_16BIT, CT_SINT, VOID );
	SET_UNPACKER( CL_P, CS_16BIT, CT_FLOAT, VOID );
	SET_UNPACKER( CL_P, CS_32BIT, CT_UNORM, VOID );
	SET_UNPACKER( CL_P, CS_32BIT, CT_SNORM, VOID );
	SET_UNPACKER( CL_P, CS_32BIT, CT_UINT, P32_UINT );
	SET_UNPACKER( CL_P, CS_32BIT, CT_SINT, VOID );
	SET_UNPACKER( CL_P, CS_32BIT, CT_FLOAT, VOID );

	SET_UNPACKER( CL_I, CS_8BIT, CT_UNORM, I8_UNORM );
	SET_UNPACKER( CL_I, CS_8BIT, CT_SNORM, I8_SNORM );
	SET_UNPACKER( CL_I, CS_8BIT, CT_UINT, I8_UINT );
	SET_UNPACKER( CL_I, CS_8BIT, CT_SINT, I8_SINT );
	SET_UNPACKER( CL_I, CS_8BIT, CT_FLOAT, VOID );
	SET_UNPACKER( CL_I, CS_16BIT, CT_UNORM, I16_UNORM );
	SET_UNPACKER( CL_I, CS_16BIT, CT_SNORM, I16_SNORM );
	SET_UNPACKER( CL_I, CS_16BIT, CT_UINT, I16_UINT );
	SET_UNPACKER( CL_I, CS_16BIT, CT_SINT, I16_SINT );
	SET_UNPACKER( CL_I, CS_16BIT, CT_FLOAT, I16_FLOAT );
	SET_UNPACKER( CL_I, CS_32BIT, CT_UNORM, I32_UNORM );
	SET_UNPACKER( CL_I, CS_32BIT, CT_SNORM, I32_SNORM );
	SET_UNPACKER( CL_I, CS_32BIT, CT_UINT, I32_UINT );
	SET_UNPACKER( CL_I, CS_32BIT, CT_SINT, I32_SINT );
	SET_UNPACKER( CL_I, CS_32BIT, CT_FLOAT, I32_FLOAT );

	SET_UNPACKER( CL_RG, CS_8BIT, CT_UNORM, R8G8_UNORM );
	SET_UNPACKER( CL_RG, CS_8BIT, CT_SNORM, R8G8_SNORM );
	SET_UNPACKER( CL_RG, CS_8BIT, CT_UINT, R8G8_UINT );
	SET_UNPACKER( CL_RG, CS_8BIT, CT_SINT, R8G8_SINT );
	SET_UNPACKER( CL_RG, CS_8BIT, CT_FLOAT, VOID );
	SET_UNPACKER( CL_RG, CS_16BIT, CT_UNORM, R16G16_UNORM );
	SET_UNPACKER( CL_RG, CS_16BIT, CT_SNORM, R16G16_SNORM );
	SET_UNPACKER( CL_RG, CS_16BIT, CT_UINT, R16G16_UINT );
	SET_UNPACKER( CL_RG, CS_16BIT, CT_SINT, R16G16_SINT );
	SET_UNPACKER( CL_RG, CS_16BIT, CT_FLOAT, R16G16_FLOAT );
	SET_UNPACKER( CL_RG, CS_32BIT, CT_UNORM, R32G32_UNORM );
	SET_UNPACKER( CL_RG, CS_32BIT, CT_SNORM, R32G32_SNORM );
	SET_UNPACKER( CL_RG, CS_32BIT, CT_UINT, R32G32_UINT );
	SET_UNPACKER( CL_RG, CS_32BIT, CT_SINT, R32G32_SINT );
	SET_UNPACKER( CL_RG, CS_32BIT, CT_FLOAT, R32G32_FLOAT );

	SET_UNPACKER( CL_GR, CS_8BIT, CT_UNORM, G8R8_UNORM );
	SET_UNPACKER( CL_GR, CS_8BIT, CT_SNORM, G8R8_SNORM );
	SET_UNPACKER( CL_GR, CS_8BIT, CT_UINT, G8R8_UINT );
	SET_UNPACKER( CL_GR, CS_8BIT, CT_SINT, G8R8_SINT );
	SET_UNPACKER( CL_GR, CS_8BIT, CT_FLOAT, VOID );
	SET_UNPACKER( CL_GR, CS_16BIT, CT_UNORM, G16R16_UNORM );
	SET_UNPACKER( CL_GR, CS_16BIT, CT_SNORM, G16R16_SNORM );
	SET_UNPACKER( CL_GR, CS_16BIT, CT_UINT, G16R16_UINT );
	SET_UNPACKER( CL_GR, CS_16BIT, CT_SINT, G16R16_SINT );
	SET_UNPACKER( CL_GR, CS_16BIT, CT_FLOAT, G16R16_FLOAT );
	SET_UNPACKER( CL_GR, CS_32BIT, CT_UNORM, G32R32_UNORM );
	SET_UNPACKER( CL_GR, CS_32BIT, CT_SNORM, G32R32_SNORM );
	SET_UNPACKER( CL_GR, CS_32BIT, CT_UINT, G32R32_UINT );
	SET_UNPACKER( CL_GR, CS_32BIT, CT_SINT, G32R32_SINT );
	SET_UNPACKER( CL_GR, CS_32BIT, CT_FLOAT, G32R32_FLOAT );

	SET_UNPACKER( CL_RGB, CS_8BIT, CT_UNORM, R8G8B8_UNORM );
	SET_UNPACKER( CL_RGB, CS_8BIT, CT_SNORM, R8G8B8_SNORM );
	SET_UNPACKER( CL_RGB, CS_8BIT, CT_UINT, R8G8B8_UINT );
	SET_UNPACKER( CL_RGB, CS_8BIT, CT_SINT, R8G8B8_SINT );
	SET_UNPACKER( CL_RGB, CS_8BIT, CT_FLOAT, VOID );
	SET_UNPACKER( CL_RGB, CS_16BIT, CT_UNORM, R16G16B16_UNORM );
	SET_UNPACKER( CL_RGB, CS_16BIT, CT_SNORM, R16G16B16_SNORM );
	SET_UNPACKER( CL_RGB, CS_16BIT, CT_UINT, R16G16B16_UINT );
	SET_UNPACKER( CL_RGB, CS_16BIT, CT_SINT, R16G16B16_SINT );
	SET_UNPACKER( CL_RGB, CS_16BIT, CT_FLOAT, R16G16B16_FLOAT );
	SET_UNPACKER( CL_RGB, CS_32BIT, CT_UNORM, R32G32B32_UNORM );
	SET_UNPACKER( CL_RGB, CS_32BIT, CT_SNORM, R32G32B32_SNORM );
	SET_UNPACKER( CL_RGB, CS_32BIT, CT_UINT, R32G32B32_UINT );
	SET_UNPACKER( CL_RGB, CS_32BIT, CT_SINT, R32G32B32_SINT );
	SET_UNPACKER( CL_RGB, CS_32BIT, CT_FLOAT, R32G32B32_FLOAT );

	SET_UNPACKER( CL_BGR, CS_8BIT, CT_UNORM, B8G8R8_UNORM );
	SET_UNPACKER( CL_BGR, CS_8BIT, CT_SNORM, B8G8R8_SNORM );
	SET_UNPACKER( CL_BGR, CS_8BIT, CT_UINT, B8G8R8_UINT );
	SET_UNPACKER( CL_BGR, CS_8BIT, CT_SINT, B8G8R8_SINT );
	SET_UNPACKER( CL_BGR, CS_8BIT, CT_FLOAT, VOID );
	SET_UNPACKER( CL_BGR, CS_16BIT, CT_UNORM, B16G16R16_UNORM );
	SET_UNPACKER( CL_BGR, CS_16BIT, CT_SNORM, B16G16R16_SNORM );
	SET_UNPACKER( CL_BGR, CS_16BIT, CT_UINT, B16G16R16_UINT );
	SET_UNPACKER( CL_BGR, CS_16BIT, CT_SINT, B16G16R16_SINT );
	SET_UNPACKER( CL_BGR, CS_16BIT, CT_FLOAT, B16G16R16_FLOAT );
	SET_UNPACKER( CL_BGR, CS_32BIT, CT_UNORM, B32G32R32_UNORM );
	SET_UNPACKER( CL_BGR, CS_32BIT, CT_SNORM, B32G32R32_SNORM );
	SET_UNPACKER( CL_BGR, CS_32BIT, CT_UINT, B32G32R32_UINT );
	SET_UNPACKER( CL_BGR, CS_32BIT, CT_SINT, B32G32R32_SINT );
	SET_UNPACKER( CL_BGR, CS_32BIT, CT_FLOAT, B32G32R32_FLOAT );

	SET_UNPACKER( CL_RGBX, CS_8BIT, CT_UNORM, R8G8B8X8_UNORM );
	SET_UNPACKER( CL_RGBX, CS_8BIT, CT_SNORM, R8G8B8X8_SNORM );
	SET_UNPACKER( CL_RGBX, CS_8BIT, CT_UINT, R8G8B8X8_UINT );
	SET_UNPACKER( CL_RGBX, CS_8BIT, CT_SINT, R8G8B8X8_SINT );
	SET_UNPACKER( CL_RGBX, CS_8BIT, CT_FLOAT, VOID );
	SET_UNPACKER( CL_RGBX, CS_16BIT, CT_UNORM, R16G16B16X16_UNORM );
	SET_UNPACKER( CL_RGBX, CS_16BIT, CT_SNORM, R16G16B16X16_SNORM );
	SET_UNPACKER( CL_RGBX, CS_16BIT, CT_UINT, R16G16B16X16_UINT );
	SET_UNPACKER( CL_RGBX, CS_16BIT, CT_SINT, R16G16B16X16_SINT );
	SET_UNPACKER( CL_RGBX, CS_16BIT, CT_FLOAT, R16G16B16X16_FLOAT );
	SET_UNPACKER( CL_RGBX, CS_32BIT, CT_UNORM, R32G32B32X32_UNORM );
	SET_UNPACKER( CL_RGBX, CS_32BIT, CT_SNORM, R32G32B32X32_SNORM );
	SET_UNPACKER( CL_RGBX, CS_32BIT, CT_UINT, R32G32B32X32_UINT );
	SET_UNPACKER( CL_RGBX, CS_32BIT, CT_SINT, R32G32B32X32_SINT );
	SET_UNPACKER( CL_RGBX, CS_32BIT, CT_FLOAT, R32G32B32X32_FLOAT );

	SET_UNPACKER( CL_BGRX, CS_8BIT, CT_UNORM, B8G8R8X8_UNORM );
	SET_UNPACKER( CL_BGRX, CS_8BIT, CT_SNORM, B8G8R8X8_SNORM );
	SET_UNPACKER( CL_BGRX, CS_8BIT, CT_UINT, B8G8R8X8_UINT );
	SET_UNPACKER( CL_BGRX, CS_8BIT, CT_SINT, B8G8R8X8_SINT );
	SET_UNPACKER( CL_BGRX, CS_8BIT, CT_FLOAT, VOID );
	SET_UNPACKER( CL_BGRX, CS_16BIT, CT_UNORM, B16G16R16X16_UNORM );
	SET_UNPACKER( CL_BGRX, CS_16BIT, CT_SNORM, B16G16R16X16_SNORM );
	SET_UNPACKER( CL_BGRX, CS_16BIT, CT_UINT, B16G16R16X16_UINT );
	SET_UNPACKER( CL_BGRX, CS_16BIT, CT_SINT, B16G16R16X16_SINT );
	SET_UNPACKER( CL_BGRX, CS_16BIT, CT_FLOAT, B16G16R16X16_FLOAT );
	SET_UNPACKER( CL_BGRX, CS_32BIT, CT_UNORM, B32G32R32X32_UNORM );
	SET_UNPACKER( CL_BGRX, CS_32BIT, CT_SNORM, B32G32R32X32_SNORM );
	SET_UNPACKER( CL_BGRX, CS_32BIT, CT_UINT, B32G32R32X32_UINT );
	SET_UNPACKER( CL_BGRX, CS_32BIT, CT_SINT, B32G32R32X32_SINT );
	SET_UNPACKER( CL_BGRX, CS_32BIT, CT_FLOAT, B32G32R32X32_FLOAT );

	SET_UNPACKER( CL_RGBA, CS_8BIT, CT_UNORM, R8G8B8A8_UNORM );
	SET_UNPACKER( CL_RGBA, CS_8BIT, CT_SNORM, R8G8B8A8_SNORM );
	SET_UNPACKER( CL_RGBA, CS_8BIT, CT_UINT, R8G8B8A8_UINT );
	SET_UNPACKER( CL_RGBA, CS_8BIT, CT_SINT, R8G8B8A8_SINT );
	SET_UNPACKER( CL_RGBA, CS_8BIT, CT_FLOAT, VOID );
	SET_UNPACKER( CL_RGBA, CS_16BIT, CT_UNORM, R16G16B16A16_UNORM );
	SET_UNPACKER( CL_RGBA, CS_16BIT, CT_SNORM, R16G16B16A16_SNORM );
	SET_UNPACKER( CL_RGBA, CS_16BIT, CT_UINT, R16G16B16A16_UINT );
	SET_UNPACKER( CL_RGBA, CS_16BIT, CT_SINT, R16G16B16A16_SINT );
	SET_UNPACKER( CL_RGBA, CS_16BIT, CT_FLOAT, R16G16B16A16_FLOAT );
	SET_UNPACKER( CL_RGBA, CS_32BIT, CT_UNORM, R32G32B32A32_UNORM );
	SET_UNPACKER( CL_RGBA, CS_32BIT, CT_SNORM, R32G32B32A32_SNORM );
	SET_UNPACKER( CL_RGBA, CS_32BIT, CT_UINT, R32G32B32A32_UINT );
	SET_UNPACKER( CL_RGBA, CS_32BIT, CT_SINT, R32G32B32A32_SINT );
	SET_UNPACKER( CL_RGBA, CS_32BIT, CT_FLOAT, R32G32B32A32_FLOAT );

	SET_UNPACKER( CL_BGRA, CS_8BIT, CT_UNORM, B8G8R8A8_UNORM );
	SET_UNPACKER( CL_BGRA, CS_8BIT, CT_SNORM, B8G8R8A8_SNORM );
	SET_UNPACKER( CL_BGRA, CS_8BIT, CT_UINT, B8G8R8A8_UINT );
	SET_UNPACKER( CL_BGRA, CS_8BIT, CT_SINT, B8G8R8A8_SINT );
	SET_UNPACKER( CL_BGRA, CS_8BIT, CT_FLOAT, VOID );
	SET_UNPACKER( CL_BGRA, CS_16BIT, CT_UNORM, B16G16R16A16_UNORM );
	SET_UNPACKER( CL_BGRA, CS_16BIT, CT_SNORM, B16G16R16A16_SNORM );
	SET_UNPACKER( CL_BGRA, CS_16BIT, CT_UINT, B16G16R16A16_UINT );
	SET_UNPACKER( CL_BGRA, CS_16BIT, CT_SINT, B16G16R16A16_SINT );
	SET_UNPACKER( CL_BGRA, CS_16BIT, CT_FLOAT, B16G16R16A16_FLOAT );
	SET_UNPACKER( CL_BGRA, CS_32BIT, CT_UNORM, B32G32R32A32_UNORM );
	SET_UNPACKER( CL_BGRA, CS_32BIT, CT_SNORM, B32G32R32A32_SNORM );
	SET_UNPACKER( CL_BGRA, CS_32BIT, CT_UINT, B32G32R32A32_UINT );
	SET_UNPACKER( CL_BGRA, CS_32BIT, CT_SINT, B32G32R32A32_SINT );
	SET_UNPACKER( CL_BGRA, CS_32BIT, CT_FLOAT, B32G32R32A32_FLOAT );
	/*
#ifdef _DEBUG
	for ( int iLayout = 0; iLayout < NUM_CHANNEL_LAYOUTS; iLayout++ )
	{
		for ( int iSize = 0; iSize < NUM_COMPONENT_SIZES; iSize++ )
		{
			for ( int iType = 0; iType < NUM_COMPONENT_TYPES; iType++ )
			{
				// enough to check only one here
				if ( g_apfnUnpackPixels[iLayout][iSize][iType] == NULL )
					__asm int 3;
			}
		}
	}
#endif
	*/

	SET_PF_UNPACKER( PF_B8G8R8X8_UNORM, B8G8R8X8_UNORM );
	SET_PF_UNPACKER( PF_B8G8R8A8_UNORM, B8G8R8A8_UNORM );
	SET_PF_UNPACKER( PF_R9G9B9E5_FLOAT, R9G9B9E5_FLOAT );
#ifdef _DEBUG
	for ( int i = 0; i < NUM_PIXEL_FORMATS; i++ )
	{
		if ( g_apfnPF_UnpackPixels[i] == NULL )
			__asm int 3;
	}
#endif

	g_aiChannelLayoutPixelSize[CL_R] = 1;
	g_aiChannelLayoutPixelSize[CL_G] = 1;
	g_aiChannelLayoutPixelSize[CL_B] = 1;
	g_aiChannelLayoutPixelSize[CL_A] = 1;
	g_aiChannelLayoutPixelSize[CL_L] = 1;
	g_aiChannelLayoutPixelSize[CL_LA] = 2;
	g_aiChannelLayoutPixelSize[CL_P] = 1;
	g_aiChannelLayoutPixelSize[CL_I] = 1;
	g_aiChannelLayoutPixelSize[CL_RG] = 2;
	g_aiChannelLayoutPixelSize[CL_GR] = 2;
	g_aiChannelLayoutPixelSize[CL_RGB] = 3;
	g_aiChannelLayoutPixelSize[CL_BGR] = 3;
	g_aiChannelLayoutPixelSize[CL_RGBX] = 4;
	g_aiChannelLayoutPixelSize[CL_BGRX] = 4;
	g_aiChannelLayoutPixelSize[CL_RGBA] = 4;
	g_aiChannelLayoutPixelSize[CL_BGRA] = 4;
#ifdef _DEBUG
	for ( int i = 0; i < NUM_CHANNEL_LAYOUTS; i++ )
	{
		if ( g_aiChannelLayoutPixelSize[i] == 0 )
			__asm int 3;
	}
#endif


	//SET_PF_INFO( PF_B8G8R8X8_UNORM, 4, RGBA_MASK( 1, 1, 1, 0 ) );
	//SET_PF_INFO( PF_B8G8R8A8_UNORM, 4, RGBA_MASK( 1, 1, 1, 1 ) );
	//SET_PF_INFO( PF_R9G9B9E5_FLOAT, 4, RGBA_MASK( 1, 1, 1, 0 ) );
	g_aiPixelFormatPixelSize[PF_B8G8R8X8_UNORM] = 4;
	g_aiPixelFormatPixelSize[PF_B8G8R8A8_UNORM] = 4;
	g_aiPixelFormatPixelSize[PF_R9G9B9E5_FLOAT] = 4;
#ifdef _DEBUG
	for ( int i = 0; i < NUM_PIXEL_FORMATS; i++ )
	{
		if ( g_aiPixelFormatPixelSize[i] == 0 )
			__asm int 3;
	}
#endif

}


PFNUNPACKPIXELS PLIB_GetUnpacker( PixelFormat_t* ppf, int bOverlay )
{
	if ( ppf->ePixelFormat != -1 )
	{
		if ( !bOverlay )
		{
			return g_apfnPF_UnpackPixels[ppf->ePixelFormat];
		}
		else
		{
			return g_apfnPF_UnpackPixelsO[ppf->ePixelFormat];
		}
	}
	else
	{
		if ( !bOverlay )
		{
			return g_apfnUnpackPixels[ppf->eChannelLayout][ppf->iComponentSize][ppf->eDataFormat];
		}
		else
		{
			return g_apfnUnpackPixelsO[ppf->eChannelLayout][ppf->iComponentSize][ppf->eDataFormat];
		}
	}
}
 
  
PFNUNPACKPIXELS PLIB_GetUnpacker_Float( PixelFormat_t* ppf, int bOverlay )
{
	if ( ppf->ePixelFormat != -1 )
	{
		if ( !bOverlay )
		{
			return g_apfnPF_UnpackPixelsF[ppf->ePixelFormat];
		}
		else
		{
			return g_apfnPF_UnpackPixelsFO[ppf->ePixelFormat];
		}
	}
	else
	{
		if ( !bOverlay )
		{
			return g_apfnUnpackPixelsF[ppf->eChannelLayout][ppf->iComponentSize][ppf->eDataFormat];
		}
		else
		{
			return g_apfnUnpackPixelsFO[ppf->eChannelLayout][ppf->iComponentSize][ppf->eDataFormat];
		}
	}
}


int PLIB_GetPixelSize( PixelFormat_t* ppf )
{
	if ( ppf->ePixelFormat != -1 )
	{
		return g_aiPixelFormatPixelSize[ppf->ePixelFormat];
	}
	else
	{
		return g_aiChannelLayoutPixelSize[ppf->eChannelLayout] * ppf->iComponentSize;
	}
}

/*
unsigned int PLIB_GetValidChannelMask( PixelFormat_t* ppf )
{
	if ( ppf->ePixelFormat != -1 )
	{
		return g_aiPixelFormatPixelSize[ppf->ePixelFormat];
	}
	else
	{
		return g_aiChannelLayoutPixelSize[ppf->eChannelLayout] * ppf->iComponentSize;
	}
}
*/

void PLIB_GetChannelInfo( PixelFormat_t* ppf, ChannelInfo_t* aci )
{
	if ( ppf->ePixelFormat != -1 )
	{
		switch ( ppf->ePixelFormat )
		{
		case PF_B8G8R8A8_UNORM:
			aci[CH_A].flags |= CHANNEL_VALID;
			aci[CH_A].eSize = CS_8BIT;
			aci[CH_A].eType = CT_UNORM;
			// continue
		case PF_B8G8R8X8_UNORM:
			aci[CH_R].flags |= CHANNEL_VALID;
			aci[CH_R].eSize = CS_8BIT;
			aci[CH_R].eType = CT_UNORM;
			aci[CH_G].flags |= CHANNEL_VALID;
			aci[CH_G].eSize = CS_8BIT;
			aci[CH_G].eType = CT_UNORM;
			aci[CH_B].flags |= CHANNEL_VALID;
			aci[CH_B].eSize = CS_8BIT;
			aci[CH_B].eType = CT_UNORM;
			break;
		case PF_R9G9B9E5_FLOAT:
			aci[CH_R].flags |= CHANNEL_VALID;
			aci[CH_R].eSize = CS_32BIT;
			aci[CH_R].eType = CT_FLOAT;
			aci[CH_G].flags |= CHANNEL_VALID;
			aci[CH_G].eSize = CS_32BIT;
			aci[CH_G].eType = CT_FLOAT;
			aci[CH_B].flags |= CHANNEL_VALID;
			aci[CH_B].eSize = CS_32BIT;
			aci[CH_B].eType = CT_FLOAT;
			break;
		default:
			__asm int 3;
		}
	}
	else
	{
		union
		{
			unsigned int dw;
			unsigned char ac[NUM_CHANNELS];
		} mask;

		switch ( ppf->eChannelLayout )
		{
		case CL_R:
			mask.dw = 0x000000FF;
			break;
		case CL_G:
			mask.dw = 0x0000FF00;
			break;
		case CL_B:
			mask.dw = 0x00FF0000;
			break;
		case CL_A:
			mask.dw = 0xFF000000;
			break;
		case CL_L:
			mask.dw = 0x00FFFFFF;
			break;
		case CL_LA:
			mask.dw = 0xFFFFFFFF;
			break;
		case CL_P:
			mask.dw = 0x00000000; // XXX:
			break;
		case CL_I:
			mask.dw = 0xFFFFFFFF;
			break;
		case CL_RG:
		case CL_GR:
			mask.dw = 0x0000FFFF;
			break;
		case CL_RGB:
		case CL_BGR:
		case CL_RGBX:
		case CL_BGRX:
			mask.dw = 0x00FFFFFF;
			break;
		case CL_RGBA:
		case CL_BGRA:
			mask.dw = 0xFFFFFFFF;
			break;
		default:
			__asm int 3;
		}

		for ( int i = 0; i < NUM_CHANNELS; i++ )
		{
			if ( mask.ac[i] )
			{
				aci[i].flags |= CHANNEL_VALID;
				aci[i].eSize = ppf->iComponentSize;
				aci[i].eType = ppf->eDataFormat;
			}
		}
	}
}


extern "C" void PLIB_ExpandRange( void* pIn, int iCount, float flRangeMin, float flRangeMax );
extern "C" void PLIB_Scale( void* pIn, int iCount, float* afl );
extern "C" void PLIB_Exponentiate( void* pIn, int iCount, float fl );
extern "C" void PLIB_AnalyzeResponse( void* buffer, Rect_t* prectIn, Rect_t* prectAnalysis, float* aflResponseMin, float* aflResponseMax, float flTopLimit, float flBottomLimit );
extern "C" void PLIB_ShuffleChannelsFloat( void* pIn, int iCount, unsigned int afMask );
extern "C" void PLIB_ShuffleChannels( void* pIn, int iCount, unsigned int afMask );
extern "C" void PLIB_DecodeGamma( void* pIn, int iCount, Gamma_t* pcs );
extern "C" void PLIB_EncodeGamma( void* pIn, int iCount, Gamma_t* pcs );
extern "C" void PLIB_ConvertGamma( void* pIn, int iCount, Gamma_t* pcs );
extern "C" void PLIB_ConvertOutputFloat( void* pOut, void* pIn, int iWidth, int iHeight, Rect_t* prect, int* aiChannelMask );


PLibFuncs_t g_funcs =
{
	PLIB_GetUnpacker,
	PLIB_GetUnpacker_Float,
	PLIB_GetPixelSize,
	PLIB_GetChannelInfo,
	PLIB_ExpandRange,
	PLIB_Scale,
	PLIB_Exponentiate,
	PLIB_AnalyzeResponse,
	PLIB_ShuffleChannelsFloat,
	PLIB_ShuffleChannels,
	PLIB_DecodeGamma, 
	PLIB_EncodeGamma,
	PLIB_ConvertGamma,
	PLIB_ConvertOutputFloat, // assembly implemented function
};


