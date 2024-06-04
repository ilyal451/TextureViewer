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

#ifndef _TYPES_H
#define _TYPES_H

// shared type definitions

enum PixelFormat_e
{
	PF_NONE = -1,
	
	// these are usually faster than the layout ones
	PF_B8G8R8X8_UNORM,	// GDI (native) format
	PF_B8G8R8A8_UNORM,	// GDI (native) format
	
	PF_R9G9B9E5_FLOAT,	// shared exponent floating point format
	
	//PF_R11G11B10_FLOAT,
	//PF_B10G11R11_FLOAT,
	//PF_R10G10B10X2_UNORM,
	//PF_R10G10B10X2_SNORM,
	NUM_PIXEL_FORMATS
};

enum ChannelLayout_e
{
					//	input			output				notes
					//	LSB   MSB		LSB   MSB
	CL_R = 0,		//	R				R:X:X:X
	CL_G,			//	G				X:G:X:X
	CL_B,			//	B				X:X:B:X
	CL_A,			//  A				X:X:X:A
	CL_L,			//	L				L:L:L:X				luminance
	CL_LA,			//	L:A				L:L:L:A				luminance + alpha
	CL_P,			//	P				P:P:P:X				a palette should be present
	CL_I,			//	I				I:I:I:I				XXX: intensity?
	CL_RG,			//	R:G				R:G:X:X
	CL_GR,			//	G:R				R:G:X:X
	CL_RGB,			//	R:G:B			R:G:B:X
	CL_BGR,			//	B:G:R			R:G:B:X
	CL_RGBX,		//	R:G:B:X			R:G:B:X
	CL_BGRX,		//	B:G:R:X			R:G:B:X
	CL_RGBA,		//	R:G:B:A			R:G:B:A
	CL_BGRA,		//	B:G:R:A			R:G:B:A

	NUM_CHANNEL_LAYOUTS

};

// this enum matches the actual component size in bytes and is here just for conveniece
enum ComponentSize_e
{
	CS_8BIT = 1,			// 1-byte component
	CS_16BIT = 2,			// 2-byte component
	CS_32BIT = 4,			// 4-byte component
	
	NUM_COMPONENT_SIZES

};

enum ComponentType_e
{
	CT_TYPELESS = -1,		// the type is invalid, you should specify it manually (PLIB_DecodeXX may return this)
	CT_UNORM,				// unsigned normalized integer (0.0/+1.0)*
	CT_SNORM,				// signed normalized integer (-1.0/+1.0)*
	CT_UINT,				// unsigned integer
	CT_SINT,				// signed integer (this is not supported currently)
	CT_FLOAT,				// IEEE 754 signed floating point format (-INF/+INF)
	// * the actual integer range depends on the component size
	//  the regular RGB images should use the UNORM type
	//  the SNORM and FLOAT types may be used for normal maps
	//  the FLOAT type may be used for HDR images
	//  the UINT type is used for palette lookup
	NUM_COMPONENT_TYPES

};

typedef struct PixelFormat_s
{
	int ePixelFormat; // if this is -1 then the channel layout, component size, and the data format members are valid, otherwise this is one of the PixelFormat definitions
	int eChannelLayout;
	int iComponentSize;
	int eDataFormat;
} PixelFormat_t;

enum Gamma_e
{
	GM_LINEAR = 0,
	GM_SRGB,
	GM_SPECIFY, // specify the gamma value directly (XXX: maybe unsupported later)
	NUM_GAMMA_OPTIONS
};

typedef struct Gamma_s
{
	int eGamma;
	float flGamma;	// usually 2.2; only valid if eGamma is GM_SPECIFY
} Gamma_t;

// currently only this flag is supported, all other bits should be 0
#define CHANNEL_VALID 0x01

typedef struct ChannelInfo_s
{
	unsigned char flags;
	unsigned char eSize;
	unsigned char eType;
	unsigned char iOriginalCapacity; // the original bit depth; if this is 0 (means the plugin has failed to fill this properly) then the eSize member is used to determine the original bit capacity (in bytes)
} ChannelInfo_t;

enum Channel_e
{
	CH_R,
	CH_G,
	CH_B,
	CH_A,
	NUM_CHANNELS
};

typedef struct Rect_s
{
	int left, top, right, bottom;
} Rect_t;

typedef int Bool_t;


#endif // _TYPES_H
