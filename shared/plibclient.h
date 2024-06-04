

#pragma once

// pixel library client (used by the plugins mostly)
// generally this will convert some fancy format to a more general form (which then can be fed as an input to the viewer)
// general formats are described by the PixelFormat_t struct (types.h)

#include "../types.h"

// note that the notation for the channel order for the explicitly defined formats 
// starts from the most significant bit:
// PF_A8R8G8B8
// MSB     LSB
// A8 R8 G8 B8
// the above notation will represent the following structure:
// struct
// {
// 	unsigned char B;
// 	unsigned char G;
// 	unsigned char R;
// 	unsigned char A;
// };


/*
some components may have predefined data types, so the type for them is not stated explicitly:
R,G,B,A - a type should follow, if none then the type is typeless and can hold any type of data
U,V,W,Q - SNORM
L - UNORM (luminance)
P - UINT (palette index)
E - SINT (exponent, used with the shared exponent format only)
D - UNORM (depth)
S - UINT (stencil)
otherwise the type follows after the definition (eg _FLOAT)
NOTE: the encoded formats (BC,YUV,etc.) may not follow this convention
*/

// the notation is from the LSB to the MSB (DXGI byte order)
typedef enum InPixelFormat
{
	// definition					input						output
	//								layout			bytes		layout			bytes	type

	IN_NONE = -1,

	// generic formats
	// these typically do swizzle or no conversion at all...
	IN_R8,						//	R8					1		R8					1
	IN_R8G8,					//	R8:G8				2		R8:G8				2
	IN_G8R8,					//	G8:R8				2		R8:G8				2
	IN_R8G8B8,					//	R8:G8:B8			3		R8:G8:B8			3
	IN_B8G8R8,					//	B8:G8:R8			3		R8:G8:B8			3
	IN_R8G8B8X8,				//	R8:G8:B8:X8			4		R8:G8:B8			3
	IN_B8G8R8X8,				//	B8:G8:R8:X8			4		R8:G8:B8			3
	IN_X8X8X8A8,				//	X8:X8:X8:A8			4		A8					1
	IN_R8G8B8A8,				//	R8:G8:B8:A8			4		R8:G8:B8:A8			4
	IN_B8G8R8A8,				//	B8:G8:R8:A8			4		R8:G8:B8:A8			4
	IN_R16,						//	R16					2		R16					2
	IN_R16G16,					//	R16:G16				4		R16:G16				4
	IN_G16R16,					//	G16:R16				4		R16:G16				4
	IN_R16G16B16,				//	R16:G16:B16			6		R16:G16:B16			6
	IN_B16G16R16,				//	B16:G16:R16			6		R16:G16:B16			6
	IN_R16G16B16X16,			//	R16:G16:B16:X16		8		R16:G16:B16			6
	IN_B16G16R16X16,			//	B16:G16:R16:X16		8		R16:G16:B16			6
	IN_X16X16X16A16,			//	X16:X16:X16:A16		8		A16					2
	IN_R16G16B16A16,			//	R16:G16:B16:A16		8		R16:G16:B16:A16		8
	IN_B16G16R16A16,			//	B16:G16:R16:A16		8		R16:G16:B16:A16		8
	IN_R32,						//	R32					4		R32					4
	IN_R32G32,					//	R32:G32				8		R32:G32				8
	IN_G32R32,					//	G32:R32				8		R32:G32				8
	IN_R32G32B32,				//	R32:G32:B32			12		R32:G32:B32			12
	IN_B32G32R32,				//	B32:G32:R32			12		R32:G32:B32			12
	IN_R32G32B32X32,			//	R32:G32:B32:X32		16		R32:G32:B32			12
	IN_B32G32R32X32,			//	B32:G32:R32:X32		16		R32:G32:B32			12
	IN_X32X32X32A32,			//	X32:X32:X32:A32		16		A32					4
	IN_R32G32B32A32,			//	R32:G32:B32:A32		16		R32:G32:B32:A32		16
	IN_B32G32R32A32,			//	B32:G32:R32:A32		16		R32:G32:B32:A32		16

	// regular image formats (usually faster)
	IN_R8G8B8_UNORM,			//	R8:G8:B8			3		PF_B8G8R8X8_UNORM
	IN_B8G8R8_UNORM,			//	B8:G8:R8			3		PF_B8G8R8X8_UNORM
	IN_R8G8B8X8_UNORM,			//	R8:G8:B8:X8			4		PF_B8G8R8X8_UNORM
	IN_B8G8R8X8_UNORM,			//	B8:G8:R8:X8			4		PF_B8G8R8X8_UNORM
	IN_R8G8B8A8_UNORM,			//	R8:G8:B8:A8			4		PF_B8G8R8A8_UNORM
	IN_B8G8R8A8_UNORM,			//	B8:G8:R8:A8			4		PF_B8G8R8A8_UNORM

	// packed unsigned normalized integer formats
	//IN_R2G2B2A2_UNORM,			//	R2:G2:B2:A2			1		PF_B8G8R8A8_UNORM
	//IN_B2G2R2A2_UNORM,			//	B2:G2:R2:A2			1		PF_B8G8R8A8_UNORM
	IN_R3G3B2_UNORM,			//	R3:G3:B2			1		PF_B8G8R8X8_UNORM
	IN_B2G3R3_UNORM,			//	B2:G3:R3			1		PF_B8G8R8X8_UNORM
	IN_R3G3B2A8_UNORM,			//	R3:G3:B2:A8			2		PF_B8G8R8A8_UNORM
	IN_B2G3R3A8_UNORM,			//	B2:G3:R3:A8			2		PF_B8G8R8A8_UNORM
	IN_R4G4B4X4_UNORM,			//	R4:G4:B4:X4			2		PF_B8G8R8X8_UNORM
	IN_B4G4R4X4_UNORM,			//	B4:G4:R4:X4			2		PF_B8G8R8X8_UNORM
	IN_R4G4B4A4_UNORM,			//	R4:G4:B4:A4			2		PF_B8G8R8A8_UNORM
	IN_B4G4R4A4_UNORM,			//	B4:G4:R4:A4			2		PF_B8G8R8A8_UNORM
	IN_R5G6B5_UNORM,			//	R5:G6:B5			2		PF_B8G8R8X8_UNORM
	IN_B5G6R5_UNORM,			//	B5:G6:R5			2		PF_B8G8R8X8_UNORM
	IN_R5G5B5X1_UNORM,			//	R5:G5:B5:X1			2		PF_B8G8R8X8_UNORM
	IN_B5G5R5X1_UNORM,			//	B5:G5:R5:X1			2		PF_B8G8R8X8_UNORM
	IN_R5G5B5A1_UNORM,			//	R5:G5:B5:A1			2		PF_B8G8R8A8_UNORM
	IN_B5G5R5A1_UNORM,			//	B5:G5:R5:A1			2		PF_B8G8R8A8_UNORM
	IN_R10G10B10X2_UNORM,		//	R10:G10:B10:X2		4		R16:G16:B16			6	UNORM
	IN_B10G10R10X2_UNORM,		//	B10:G10:R10:X2		4		R16:G16:B16			6	UNORM
	IN_X10X10X10A2_UNORM,		//	X10:X10:X10:A2		4		A8					1	UNORM
	IN_R10G10B10A2_UNORM,		//	R10:G10:B10:A2		4		R16:G16:B16:A16		8	UNORM
	IN_B10G10R10A2_UNORM,		//	B10:G10:R10:A2		4		R16:G16:B16:A16		8	UNORM
	//IN_R12G12B12A12_UNORM,	//	R12:G12:B12:A12		6		R16:G16:B16:A16		8	UNORM
	// XXX: there's also a swizzled rgb10a2 format
	//  probably should leave it up to the application...

	// packed signed normalized integer formats
	// the alpha is discarded as it's not signed, use a different stream to store it
	IN_U10V10W10X2,				//	R10:G10:B10:X2		4		R16:G16:B16			6	SNORM
	IN_W10V10U10X2,				//	B10:G10:R10:X2		4		R16:G16:B16			6	SNORM

	IN_R10G10B10A2_UINT,	// XXX:
	IN_R10G10B10X2_XR_BIAS,

	// packed floating point formats
	IN_R9G9B9E5_FLOAT,			//	R9:G9:B9:E5			4		R32:G32:B32			12	FLOAT	NOTE: use PF_R9G9B9E5_FLOAT to store this instead
	IN_R11G11B10_FLOAT,			//	R11:G11:B10			4		R16:G16:B16			6	FLOAT
	IN_B10G11R11_FLOAT,			//	B10:G11:R11			4		R16:G16:B16			6	FLOAT	D3DFMT_W11V11U10 (check this as it may be an SNORM format)

	// legacy directx formats D3DFMT_
	IN_U8V8CX,		// CxV8U8
	IN_U5V5X6,		// L6V5U5
	IN_X5X5L6,		// L6V5U5
	IN_U8V8X8X8,	// X8L8V8U8
	IN_X8X8L8X8,	// X8L8V8U8

	// depth/stencil buffer formats (needed?)
	IN_D24X8,
	IN_X24S8,
	IN_D15X1,
	IN_X15S1,
	IN_D32X8X24,
	IN_X32S8X24,

	// packed luminance
	// M = MSB -> LSB (the Microsoft's bit order for packed bitmaps, used in the BMP format)
	// L = LSB -> MSB
	IN_L1M,						//	L1					1/8		L8					1	UNORM
	IN_L2M,						//	L2					2/8		L8					1	UNORM
	IN_L4M,						//	L4					4/8		L8					1	UNORM
	IN_L1L,						//	L1					1/8		L8					1	UNORM
	IN_L2L,						//	L2					2/8		L8					1	UNORM
	IN_L4L,						//	L4					4/8		L8					1	UNORM

	// packed palette indices, the order is like in the above
	IN_P1M,						//	P1					1/8		P8					1	UINT
	IN_P2M,						//	P2					2/8		P8					1	UINT
	IN_P4M,						//	P4					4/8		P8					1	UINT
	IN_P1L,						//	P1					1/8		P8					1	UINT
	IN_P2L,						//	P2					2/8		P8					1	UINT
	IN_P4L,						//	P4					4/8		P8					1	UINT

	// packed palette/luminance with alpha
	IN_L4A4,
	IN_P4X4,
	IN_X4A4,
	//IN_L6A2,
	IN_L8A8,
	IN_P8X8,
	IN_X8A8,
	//IN_L12A4,
	IN_L16A16,

	// single palette/luminance
	IN_L8,
	IN_L16,
	IN_L32,
	IN_P8,
	IN_P16,
	IN_P32,

	// single alpha
	IN_A8,
	IN_A16,
	IN_A32,

	// CMYK
	//IN_CMYK,
	//IN_CMYK_ADOBE, // to handle the adobe cmyk jpegs (whose are swizzled)

	// interleaved formats (DX9)
	IN_RGBG,
	IN_GRGB,
	IN_YUY2,
	IN_UYVY,

	// compressed formats
	// - do not support negative widths/heights
	// - require (?) the width/height to be multiple of 4
	IN_BC1,						//	-					-		PF_B8G8R8A8_UNORM
	IN_BC2,						//	-					-		PF_B8G8R8A8_UNORM
	IN_BC3,						//	-					-		PF_B8G8R8A8_UNORM
	IN_BC4_UNORM,				//	-					-		R8					1	UNORM
	IN_BC4_SNORM,				//	-					-		R8					1	SNORM
	IN_BC5_UNORM,				//	-					-		R8:G8				2	UNORM
	IN_BC5_SNORM,				//	-					-		R8:G8				2	SNORM
	IN_BC6H_UF16,				//	-					-		R16:G16:B16			6	FLOAT
	IN_BC6H_SF16,				//	-					-		R16:G16:B16			6	FLOAT
	IN_BC7,						//	-					-		PF_B8G8R8A8_UNORM
	//IN_RGTC,
	
	NUM_INPUT_PIXEL_FORMATS,

} InPixelFormat_t;

// the width and height can be negative thus indicating that the order should be reversed; the default order is top to bottom, left to right
// if the pitch value is greater than 0, then it's considered to be the size (in bytes) of a single scanline
//  if it's 0, then there is no gap between the scanlines
//  if it's less that 0, then the pitch is aligned by the amount of bytes equal to ( 1 << -pitch )
//   an example may be the BMP format which scanlines are aligned by 4 bytes, in this case the pitch should be -2 (shifts two bits)
// the return value is the amount of bytes has been read from the input
// the return value is 0 on error
typedef unsigned int ( *PFNDECODEPIXELS )( void* pOut, void* pIn, int iWidth, int iHeight, int iPitch, void* pPal );

// the input size can be calculated by calling DecodePixels with the buffer
// pointers set to NULL, or you can also use GetInputSize for this purpose

typedef struct PLibClientFuncs
{
	unsigned int ( *pfnDecodePixels )( int eInPixelFormat, void* pOut, void* pIn, int iWidth, int iHeight, int iPitch );

	// input/output size prediction
	// the return value is the size in bytes expected/required for the buffer to hold the described image
	unsigned int ( *pfnGetInputSize )( int eInPixelFormat, int iWidth, int iHeight, int iPitch );

	// returns the size (in bytes) of a single pixel for the given format
	int ( *pfnGetPixelSize )( PixelFormat_t* ppf );

	// some formats may require the number of pixels width and height to be of a certain granularity, mostly the DXTn formats
	// the return value is the granularity in pixels (NOT bytes!)
	int ( *pfnGetWidthGranularity )( int eInPixelFormat );
	int ( *pfnGetHeightGranularity )( int eInPixelFormat );

	int ( *pfnGetOutputFormat )( int eInPixelFormat, PixelFormat_t* ppf );

} PLibClientFuncs_t;


typedef PLibClientFuncs_t* ( *PFNPLIBGETCLIENTFUNCS )( void );
