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

// BC6-7 block compression decompressor

#include <windows.h>
#include <stdio.h>
#include <math.h>


extern "C" {
unsigned int PLIB_DecodeBC6H_UF16( void* pOut, void* pIn, int iWidth, int iHeight, int iPitch, void* pPal );
unsigned int PLIB_DecodeBC6H_SF16( void* pOut, void* pIn, int iWidth, int iHeight, int iPitch, void* pPal );
unsigned int PLIB_DecodeBC7( void* pOut, void* pIn, int iWidth, int iHeight, int iPitch, void* pPal );
}
// debug
FILE* gstream;


typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef __int64 int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

#pragma pack ( push, 1 )

typedef struct RGBQuad
{
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t a;
} RGBQuad_t, rgb_quad_t;

typedef struct HFRGBTriple
{
	uint16_t r;
	uint16_t g;
	uint16_t b;
} HFRGBTriple_t;

#pragma pack ( pop )



typedef struct BitBuffer_s
{
	int iCurrentByte;
	int iCurrentBit;
	int iSize;
	uint8_t* data;
} BitBuffer_t;


unsigned int ReadBits( BitBuffer_t* pbuf, int nBits )
{
	unsigned int v = 0;

	int nBitsRemaining = nBits;

	while ( nBitsRemaining > 0 )
	{
		if ( pbuf->iCurrentByte >= pbuf->iSize )
		{
			__asm int 3;
		}

		int nBitsAvail = 8 - pbuf->iCurrentBit;
		int nBitsToRead = min( nBitsAvail, nBitsRemaining );

		unsigned char c = pbuf->data[pbuf->iCurrentByte];
		int nExcessBits = nBitsAvail - nBitsToRead;
		c <<= nExcessBits; // ! erases the 'excess' top bits (8-bit clipping) !
		c >>= ( nExcessBits + pbuf->iCurrentBit );
		unsigned int a = c;
		a <<= ( nBits - nBitsRemaining );
		v |= a;

		pbuf->iCurrentBit += nBitsToRead;
		if ( pbuf->iCurrentBit >= 8 )
		{
			pbuf->iCurrentBit = 0;
			pbuf->iCurrentByte++;
		}

		nBitsRemaining -= nBitsToRead;
	}

	return v;
}


float HalfFloatToFloat( uint16_t hf )
{
	uint32_t fl;

	uint32_t sign = ( hf & 0x8000 ) << 16;
	uint32_t exp = ( hf & 0x7C00 ) >> 10;
	uint32_t man = hf & 0x3FF;

	if ( exp == 0 )
	{
		if ( man == 0 ) // zero
		{
			fl = 0;
		}
		else // denormal
		{
			while ( !( man & 0x400 ) )
			{
				man <<= 1;
				exp++;
			}
			man &= 0x3FF;
			fl = ( ( 127 - 15 - exp ) << 23 ) | ( man << 13 );
		}
	}
	else if ( ( exp & 0x1F ) == 0x1F ) // Inf or NaN
	{
		fl = ( 0xFF << 23 ) | ( man << 13 );
	}
	else // the regular case
	{
		fl = ( ( 127 - 15 + exp ) << 23 ) | ( man << 13 );
	}

	fl |= sign;

	return *( float* )&fl;
}

/*
float HalfFloatToFloat( uint16_t hf )
{
	uint32_t fl = 0;

	uint32_t sign = hf & 0x8000;
	uint32_t exp = hf & 0x7C00;
	uint32_t man = hf & 0x3FF;

	if ( exp == 0 && man == 0 )
		fl = (sign<<16);
	else if ( exp == 0 )
		fl = -1;
	else if ( ( exp & 0x7C00 ) == 0x7C00 )
		__asm int 3;
	else
		fl = (sign<<16) | ((exp+0x1C000)<<13) | (man<<13);

	return *( float* )&fl;
}
*/

#define BPTC_BLOCK_SIZE 16

#define INTERPOLATE_BASE_64( c1, c2, w ) ( ( ( (uint32_t)(c2)*(w) ) + ( (uint32_t)(c1)*(64-(w)) ) ) / 64 )
#define SIGNED_INTERPOLATE_BASE_64( c1, c2, w ) ( ( ( (int32_t)(c2)*(w) ) + ( (int32_t)(c1)*(64-(w)) ) ) / 64 )



#define ENDPOINT_SIZE 3
#define END ( -1 )
#define R0 ( ( 0 * ( ENDPOINT_SIZE ) ) + 0 )
#define R1 ( ( 1 * ( ENDPOINT_SIZE ) ) + 0 )
#define R2 ( ( 2 * ( ENDPOINT_SIZE ) ) + 0 )
#define R3 ( ( 3 * ( ENDPOINT_SIZE ) ) + 0 )
#define G0 ( ( 0 * ( ENDPOINT_SIZE ) ) + 1 )
#define G1 ( ( 1 * ( ENDPOINT_SIZE ) ) + 1 )
#define G2 ( ( 2 * ( ENDPOINT_SIZE ) ) + 1 )
#define G3 ( ( 3 * ( ENDPOINT_SIZE ) ) + 1 )
#define B0 ( ( 0 * ( ENDPOINT_SIZE ) ) + 2 )
#define B1 ( ( 1 * ( ENDPOINT_SIZE ) ) + 2 )
#define B2 ( ( 2 * ( ENDPOINT_SIZE ) ) + 2 )
#define B3 ( ( 3 * ( ENDPOINT_SIZE ) ) + 2 )

typedef struct BC6Command
{
	int8_t comp;
	int8_t a;
	int8_t b;
	int8_t x; // unused
} BC6Command_t;

BC6Command_t g_acmdMode_00[] = { { G2, 4, 4 }, { B2, 4, 4 }, { B3, 4, 4 }, { R0, 9, 0 }, { G0, 9, 0 }, { B0, 9, 0 }, { R1, 4, 0 }, { G3, 4, 4 }, { G2, 3, 0 }, { G1, 4, 0 }, { B3, 0, 0 }, { G3, 3, 0 }, { B1, 4, 0 }, { B3, 1, 1 }, { B2, 3, 0 }, { R2, 4, 0 }, { B3, 2, 2 }, { R3, 4, 0 }, { B3, 3, 3 }, { END } };
BC6Command_t g_acmdMode_01[] = { { G2, 5, 5 }, { G3, 4, 4 }, { G3, 5, 5 }, { R0, 6, 0 }, { B3, 0, 0 }, { B3, 1, 1 }, { B2, 4, 4 }, { G0, 6, 0 }, { B2, 5, 5 }, { B3, 2, 2 }, { G2, 4, 4 }, { B0, 6, 0 }, { B3, 3, 3 }, { B3, 5, 5 }, { B3, 4, 4 }, { R1, 5, 0 }, { G2, 3, 0 }, { G1, 5, 0 }, { G3, 3, 0 }, { B1, 5, 0 }, { B2, 3, 0 }, { R2, 5, 0 }, { R3, 5, 0 }, { END } };
BC6Command_t g_acmdMode_02[] = { { R0, 9, 0 }, { G0, 9, 0 }, { B0, 9, 0 }, { R1, 4, 0 }, { R0, 10, 10 }, { G2, 3, 0 }, { G1, 3, 0 }, { G0, 10, 10 }, { B3, 0, 0 }, { G3, 3, 0 }, { B1, 3, 0 }, { B0, 10, 10 }, { B3, 1, 1 }, { B2, 3, 0 }, { R2, 4, 0 }, { B3, 2, 2 }, { R3, 4, 0 }, { B3, 3, 3 }, { END } };
BC6Command_t g_acmdMode_06[] = { { R0, 9, 0 }, { G0, 9, 0 }, { B0, 9, 0 }, { R1, 3, 0 }, { R0, 10, 10 }, { G3, 4, 4 }, { G2, 3, 0 }, { G1, 4, 0 }, { G0, 10, 10 }, { G3, 3, 0 }, { B1, 3, 0 }, { B0, 10, 10 }, { B3, 1, 1 }, { B2, 3, 0 }, { R2, 3, 0 }, { B3, 0, 0 }, { B3, 2, 2 }, { R3, 3, 0 }, { G2, 4, 4 }, { B3, 3, 3 }, { END } };
BC6Command_t g_acmdMode_10[] = { { R0, 9, 0 }, { G0, 9, 0 }, { B0, 9, 0 }, { R1, 3, 0 }, { R0, 10, 10 }, { B2, 4, 4 }, { G2, 3, 0 }, { G1, 3, 0 }, { G0, 10, 10 }, { B3, 0, 0 }, { G3, 3, 0 }, { B1, 4, 0 }, { B0, 10, 10 }, { B2, 3, 0 }, { R2, 3, 0 }, { B3, 1, 1 }, { B3, 2, 2 }, { R3, 3, 0 }, { B3, 4, 4 }, { B3, 3, 3 }, { END } };
BC6Command_t g_acmdMode_14[] = { { R0, 8, 0 }, { B2, 4, 4 }, { G0, 8, 0 }, { G2, 4, 4 }, { B0, 8, 0 }, { B3, 4, 4 }, { R1, 4, 0 }, { G3, 4, 4 }, { G2, 3, 0 }, { G1, 4, 0 }, { B3, 0, 0 }, { G3, 3, 0 }, { B1, 4, 0 }, { B3, 1, 1 }, { B2, 3, 0 }, { R2, 4, 0 }, { B3, 2, 2 }, { R3, 4, 0 }, { B3, 3, 3 }, { END } };

BC6Command_t g_acmdMode_18[] = { 
	{ R0, 7, 0 }, { G3, 4, 4 }, { B2, 4, 4 }, 
	{ G0, 7, 0 }, { B3, 2, 2 }, { G2, 4, 4 }, 
	{ B0, 7, 0 }, { B3, 3, 3 }, { B3, 4, 4 }, 
	{ R1, 5, 0 }, { G2, 3, 0 }, { G1, 4, 0 }, 
	{ B3, 0, 0 }, { G3, 3, 0 }, { B1, 4, 0 }, 
	{ B3, 1, 1 }, { B2, 3, 0 }, { R2, 5, 0 }, 
	{ R3, 5, 0 }, { END } };

BC6Command_t g_acmdMode_22[] = { { R0, 7, 0 }, { B3, 0, 0 }, { B2, 4, 4 }, { G0, 7, 0 }, { G2, 5, 5 }, { G2, 4, 4 }, { B0, 7, 0 }, { G3, 5, 5 }, { B3, 4, 4 }, { R1, 4, 0 }, { G3, 4, 4 }, { G2, 3, 0 }, { G1, 5, 0 }, { G3, 3, 0 }, { B1, 4, 0 }, { B3, 1, 1 }, { B2, 3, 0 }, { R2, 4, 0 }, { B3, 2, 2 }, { R3, 4, 0 }, { B3, 3, 3 }, { END } };
BC6Command_t g_acmdMode_26[] = { { R0, 7, 0 }, { B3, 1, 1 }, { B2, 4, 4 }, { G0, 7, 0 }, { B2, 5, 5 }, { G2, 4, 4 }, { B0, 7, 0 }, { B3, 5, 5 }, { B3, 4, 4 }, { R1, 4, 0 }, { G3, 4, 4 }, { G2, 3, 0 }, { G1, 4, 0 }, { B3, 0, 0 }, { G3, 3, 0 }, { B1, 5, 0 }, { B2, 3, 0 }, { R2, 4, 0 }, { B3, 2, 2 }, { R3, 4, 0 }, { B3, 3, 3 }, { END } };
BC6Command_t g_acmdMode_30[] = { { R0, 5, 0 }, { G3, 4, 4 }, { B3, 0, 0 }, { B3, 1, 1 }, { B2, 4, 4 }, { G0, 5, 0 }, { G2, 5, 5 }, { B2, 5, 5 }, { B3, 2, 2 }, { G2, 4, 4 }, { B0, 5, 0 }, { G3, 5, 5 }, { B3, 3, 3 }, { B3, 5, 5 }, { B3, 4, 4 }, { R1, 5, 0 }, { G2, 3, 0 }, { G1, 5, 0 }, { G3, 3, 0 }, { B1, 5, 0 }, { B2, 3, 0 }, { R2, 5, 0 }, { R3, 5, 0 }, { END } };
BC6Command_t g_acmdMode_03[] = { { R0, 9, 0 }, { G0, 9, 0 }, { B0, 9, 0 }, { R1, 9, 0 }, { G1, 9, 0 }, { B1, 9, 0 }, { END } };
BC6Command_t g_acmdMode_07[] = { { R0, 9, 0 }, { G0, 9, 0 }, { B0, 9, 0 }, { R1, 8, 0 }, { R0, 10, 10 }, { G1, 8, 0 }, { G0, 10, 10 }, { B1, 8, 0 }, { B0, 10, 10 }, { END } };
BC6Command_t g_acmdMode_11[] = { { R0, 9, 0 }, { G0, 9, 0 }, { B0, 9, 0 }, { R1, 7, 0 }, { R0, 10, 11 }, { G1, 7, 0 }, { G0, 10, 11 }, { B1, 7, 0 }, { B0, 10, 11 }, { END } };
BC6Command_t g_acmdMode_15[] = { { R0, 9, 0 }, { G0, 9, 0 }, { B0, 9, 0 }, { R1, 3, 0 }, { R0, 10, 15 }, { G1, 3, 0 }, { G0, 10, 15 }, { B1, 3, 0 }, { B0, 10, 15 }, { END } };

typedef struct BC6ModeStruct
{
	int iModeNumber;
	int bTransform;
	int nSubsets;
	int nPartitionBits;
	int nIndexBits;
	int nEndpointBits;
	int nDeltaBits[3];
	BC6Command_t* pcmd;
} BC6ModeStruct_t;

BC6ModeStruct_t g_asBC6Mode1Bit[] =
{
	{ 0,  1,  2,  5,  3,  10, { 5,  5,  5  }, g_acmdMode_00 },
	{ 1,  1,  2,  5,  3,  7,  { 6,  6,  6  }, g_acmdMode_01 },
};

BC6ModeStruct_t g_asBC6Mode3Bits[] =
{
	{ 2,  1,  2,  5,  3,  11, { 5,  4,  4  }, g_acmdMode_02 },
	{ 6,  1,  2,  5,  3,  11, { 4,  5,  4  }, g_acmdMode_06 },
	{ 10, 1,  2,  5,  3,  11, { 4,  4,  5  }, g_acmdMode_10 },
	{ 14, 1,  2,  5,  3,  9,  { 5,  5,  5  }, g_acmdMode_14 },
	{ 18, 1,  2,  5,  3,  8,  { 6,  5,  5  }, g_acmdMode_18 },
	{ 22, 1,  2,  5,  3,  8,  { 5,  6,  5  }, g_acmdMode_22 },
	{ 26, 1,  2,  5,  3,  8,  { 5,  5,  6  }, g_acmdMode_26 },
	{ 30, 0,  2,  5,  3,  6,  { 6,  6,  6  }, g_acmdMode_30 },
};

BC6ModeStruct_t g_asBC6Mode2Bits[] =
{
	{ 3,  0,  1,  0,  4,  10, { 10, 10, 10 }, g_acmdMode_03 },
	{ 7,  1,  1,  0,  4,  11, { 9,  9,  9  }, g_acmdMode_07 },
	{ 11, 1,  1,  0,  4,  12, { 8,  8,  8  }, g_acmdMode_11 },
	{ 15, 1,  1,  0,  4,  16, { 4,  4,  4  }, g_acmdMode_15 },
};


// BC7
typedef struct ModeStruct
{
	int nSubsets;
	int nPartitionBits;
	int nRotationBits;
	int nIndexSelectionBits;
	int nColorBits;
	int nAlphaBits;
	int bUniquePBits;
	int bSharedPBits;
	int nIndexBits;
	int nSecondaryIndexBits;
} ModeStruct_t;

ModeStruct_t g_asMode[8] =
{
	{ 3,   4, 0, 0,   4, 0,   1, 0,   3, 0 },
	{ 2,   6, 0, 0,   6, 0,   0, 1,   3, 0 },
	{ 3,   6, 0, 0,   5, 0,   0, 0,   2, 0 },
	{ 2,   6, 0, 0,   7, 0,   1, 0,   2, 0 },
	{ 1,   0, 2, 1,   5, 6,   0, 0,   2, 3 },
	{ 1,   0, 2, 0,   7, 8,   0, 0,   2, 2 },
	{ 1,   0, 0, 0,   7, 7,   1, 0,   4, 0 },
	{ 2,   6, 0, 0,   5, 5,   1, 0,   2, 0 }
};

unsigned char g_aiPattern2[64*16] =
{
    0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,
    0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,
    0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,
    0,0,0,1,0,0,1,1,0,0,1,1,0,1,1,1,
    0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,1,
    0,0,1,1,0,1,1,1,0,1,1,1,1,1,1,1,
    0,0,0,1,0,0,1,1,0,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,1,0,0,1,1,0,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,
    0,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1,
    0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,
    0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,
    0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,
    0,0,0,0,1,0,0,0,1,1,1,0,1,1,1,1,
    0,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,1,0,0,0,1,1,1,0,
    0,1,1,1,0,0,1,1,0,0,0,1,0,0,0,0,
    0,0,1,1,0,0,0,1,0,0,0,0,0,0,0,0,
    0,0,0,0,1,0,0,0,1,1,0,0,1,1,1,0,
    0,0,0,0,0,0,0,0,1,0,0,0,1,1,0,0,
    0,1,1,1,0,0,1,1,0,0,1,1,0,0,0,1,
    0,0,1,1,0,0,0,1,0,0,0,1,0,0,0,0,
    0,0,0,0,1,0,0,0,1,0,0,0,1,1,0,0,
    0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,
    0,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0,
    0,0,0,1,0,1,1,1,1,1,1,0,1,0,0,0,
    0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
    0,1,1,1,0,0,0,1,1,0,0,0,1,1,1,0,
    0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,
    0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,
    0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,
    0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,
    0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0,
    0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,
    0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,
    0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,
    0,1,0,1,1,0,1,0,1,0,1,0,0,1,0,1,
    0,1,1,1,0,0,1,1,1,1,0,0,1,1,1,0,
    0,0,0,1,0,0,1,1,1,1,0,0,1,0,0,0,
    0,0,1,1,0,0,1,0,0,1,0,0,1,1,0,0,
    0,0,1,1,1,0,1,1,1,1,0,1,1,1,0,0,
    0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
    0,0,1,1,1,1,0,0,1,1,0,0,0,0,1,1,
    0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,
    0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,
    0,1,0,0,1,1,1,0,0,1,0,0,0,0,0,0,
    0,0,1,0,0,1,1,1,0,0,1,0,0,0,0,0,
    0,0,0,0,0,0,1,0,0,1,1,1,0,0,1,0,
    0,0,0,0,0,1,0,0,1,1,1,0,0,1,0,0,
    0,1,1,0,1,1,0,0,1,0,0,1,0,0,1,1,
    0,0,1,1,0,1,1,0,1,1,0,0,1,0,0,1,
    0,1,1,0,0,0,1,1,1,0,0,1,1,1,0,0,
    0,0,1,1,1,0,0,1,1,1,0,0,0,1,1,0,
    0,1,1,0,1,1,0,0,1,1,0,0,1,0,0,1,
    0,1,1,0,0,0,1,1,0,0,1,1,1,0,0,1,
    0,1,1,1,1,1,1,0,1,0,0,0,0,0,0,1,
    0,0,0,1,1,0,0,0,1,1,1,0,0,1,1,1,
    0,0,0,0,1,1,1,1,0,0,1,1,0,0,1,1,
    0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,0,
    0,0,1,0,0,0,1,0,1,1,1,0,1,1,1,0,
    0,1,0,0,0,1,0,0,0,1,1,1,0,1,1,1
};


unsigned char g_aiPattern3[64*16] =
{
    0,0,1,1,0,0,1,1,0,2,2,1,2,2,2,2,
    0,0,0,1,0,0,1,1,2,2,1,1,2,2,2,1,
    0,0,0,0,2,0,0,1,2,2,1,1,2,2,1,1,
    0,2,2,2,0,0,2,2,0,0,1,1,0,1,1,1,
    0,0,0,0,0,0,0,0,1,1,2,2,1,1,2,2,
    0,0,1,1,0,0,1,1,0,0,2,2,0,0,2,2,
    0,0,2,2,0,0,2,2,1,1,1,1,1,1,1,1,
    0,0,1,1,0,0,1,1,2,2,1,1,2,2,1,1,
    0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,
    0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,
    0,0,0,0,1,1,1,1,2,2,2,2,2,2,2,2,
    0,0,1,2,0,0,1,2,0,0,1,2,0,0,1,2,
    0,1,1,2,0,1,1,2,0,1,1,2,0,1,1,2,
    0,1,2,2,0,1,2,2,0,1,2,2,0,1,2,2,
    0,0,1,1,0,1,1,2,1,1,2,2,1,2,2,2,
    0,0,1,1,2,0,0,1,2,2,0,0,2,2,2,0,
    0,0,0,1,0,0,1,1,0,1,1,2,1,1,2,2,
    0,1,1,1,0,0,1,1,2,0,0,1,2,2,0,0,
    0,0,0,0,1,1,2,2,1,1,2,2,1,1,2,2,
    0,0,2,2,0,0,2,2,0,0,2,2,1,1,1,1,
    0,1,1,1,0,1,1,1,0,2,2,2,0,2,2,2,
    0,0,0,1,0,0,0,1,2,2,2,1,2,2,2,1,
    0,0,0,0,0,0,1,1,0,1,2,2,0,1,2,2,
    0,0,0,0,1,1,0,0,2,2,1,0,2,2,1,0,
    0,1,2,2,0,1,2,2,0,0,1,1,0,0,0,0,
    0,0,1,2,0,0,1,2,1,1,2,2,2,2,2,2,
    0,1,1,0,1,2,2,1,1,2,2,1,0,1,1,0,
    0,0,0,0,0,1,1,0,1,2,2,1,1,2,2,1,
    0,0,2,2,1,1,0,2,1,1,0,2,0,0,2,2,
    0,1,1,0,0,1,1,0,2,0,0,2,2,2,2,2,
    0,0,1,1,0,1,2,2,0,1,2,2,0,0,1,1,
    0,0,0,0,2,0,0,0,2,2,1,1,2,2,2,1,
    0,0,0,0,0,0,0,2,1,1,2,2,1,2,2,2,
    0,2,2,2,0,0,2,2,0,0,1,2,0,0,1,1,
    0,0,1,1,0,0,1,2,0,0,2,2,0,2,2,2,
    0,1,2,0,0,1,2,0,0,1,2,0,0,1,2,0,
    0,0,0,0,1,1,1,1,2,2,2,2,0,0,0,0,
    0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,
    0,1,2,0,2,0,1,2,1,2,0,1,0,1,2,0,
    0,0,1,1,2,2,0,0,1,1,2,2,0,0,1,1,
    0,0,1,1,1,1,2,2,2,2,0,0,0,0,1,1,
    0,1,0,1,0,1,0,1,2,2,2,2,2,2,2,2,
    0,0,0,0,0,0,0,0,2,1,2,1,2,1,2,1,
    0,0,2,2,1,1,2,2,0,0,2,2,1,1,2,2,
    0,0,2,2,0,0,1,1,0,0,2,2,0,0,1,1,
    0,2,2,0,1,2,2,1,0,2,2,0,1,2,2,1,
    0,1,0,1,2,2,2,2,2,2,2,2,0,1,0,1,
    0,0,0,0,2,1,2,1,2,1,2,1,2,1,2,1,
    0,1,0,1,0,1,0,1,0,1,0,1,2,2,2,2,
    0,2,2,2,0,1,1,1,0,2,2,2,0,1,1,1,
    0,0,0,2,1,1,1,2,0,0,0,2,1,1,1,2,
    0,0,0,0,2,1,1,2,2,1,1,2,2,1,1,2,
    0,2,2,2,0,1,1,1,0,1,1,1,0,2,2,2,
    0,0,0,2,1,1,1,2,1,1,1,2,0,0,0,2,
    0,1,1,0,0,1,1,0,0,1,1,0,2,2,2,2,
    0,0,0,0,0,0,0,0,2,1,1,2,2,1,1,2,
    0,1,1,0,0,1,1,0,2,2,2,2,2,2,2,2,
    0,0,2,2,0,0,1,1,0,0,1,1,0,0,2,2,
    0,0,2,2,1,1,2,2,1,1,2,2,0,0,2,2,
    0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,2,
    0,0,0,2,0,0,0,1,0,0,0,2,0,0,0,1,
    0,2,2,2,1,2,2,2,0,2,2,2,1,2,2,2,
    0,1,0,1,2,2,2,2,2,2,2,2,2,2,2,2,
    0,1,1,1,2,0,1,1,2,2,0,1,2,2,2,0
};

inline int GetSubset( int nSubsets, int iPartition, int iPixel )
{
	int iSubset;
	if ( nSubsets == 1 )
	{
		iSubset = 0;
	}
	else if ( nSubsets == 2 )
	{
		iSubset = g_aiPattern2[iPartition*16+iPixel];
	}
	else
	{
		iSubset = g_aiPattern3[iPartition*16+iPixel];
	}
	return iSubset;
}


unsigned char g_aiAnchorIndex21[64] =
{
    15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,
    15, 2, 8, 2, 2, 8, 8,15,
     2, 8, 2, 2, 8, 8, 2, 2,
    15,15, 6, 8, 2, 8,15,15,
     2, 8, 2, 2, 2,15,15, 6,
     6, 2, 6, 8,15,15, 2, 2,
    15,15,15,15,15, 2, 2,15,
};

unsigned char g_aiAnchorIndex31[64] =
{
     3, 3,15,15, 8, 3,15,15,
     8, 8, 6, 6, 6, 5, 3, 3,
     3, 3, 8,15, 3, 3, 6,10,
     5, 8, 8, 6, 8, 5,15,15,
     8,15, 3, 5, 6,10, 8,15,
    15, 3,15, 5,15,15,15,15,
     3,15, 5, 5, 5, 8, 5,10,
     5,10, 8,13,15,12, 3, 3,
};

unsigned char g_aiAnchorIndex32[64] =
{
    15, 8, 8, 3,15,15, 3, 8,
    15,15,15,15,15,15,15, 8,
    15, 8,15, 3,15, 8,15, 8,
     3,15, 6,10,15,15,10, 8,
    15, 3,15,10,10, 8, 9,10,
     6,15, 8,15, 3, 6, 6, 8,
    15, 3,15,15,15,15,15,15,
    15,15,15,15, 3,15,15, 8,
};

unsigned char g_aiWeights2[4] = { 0, 21, 43, 64 };
unsigned char g_aiWeights3[8] = { 0, 9, 18, 27, 37, 46, 55, 64 };
unsigned char g_aiWeights4[16] = { 0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64 };

inline uint8_t GetWeight( int index, int nIndexBits )
{
	uint8_t iWeight;
	if ( nIndexBits == 2 )
	{
		iWeight = g_aiWeights2[index];
	}
	else if ( nIndexBits == 3 )
	{
		iWeight = g_aiWeights3[index];
	}
	else // 4 bits
	{
		iWeight = g_aiWeights4[index];
	}
	return iWeight;
}


unsigned int _PLIB_DecodeBC6H( void* pOut, void* pIn, int iWidth, int iHeight, int iPitch, void* pPal, bool bSigned )
{
	//gstream = fopen("dump.txt", "wt");

	BitBuffer_t buf;
	int32_t endpoints[4][3];
	uint8_t indices[16];
	uint16_t block_pixels[16][4];
	//uint8_t rgba[4];

	// do the plib stuff
	if ( iWidth == 0 )
	{
		// width granularity
		return 4; // 4 pixels
	}
	else if ( iHeight == 0 )
	{
		// height granularity
		return 4; // 4 pixels
	}
	else if ( ( iWidth < 0 ) || ( iHeight < 0 ) )
	{
		// not supported here
		return 0;
	}

	int nBlocksWidth = ( iWidth + 3 ) / 4;
	int nBlocksHeight = ( iHeight + 3 ) / 4;

	// do the plib stuff
	if ( pIn == NULL )
	{
		// input buffer size
		return nBlocksWidth * nBlocksHeight * BPTC_BLOCK_SIZE;
	}
	else if ( pOut == NULL )
	{
		// output buffer size
		return iWidth * iHeight * sizeof( HFRGBTriple_t );
	}

	uint8_t* block = ( uint8_t* )pIn;
	HFRGBTriple_t* pixels = ( HFRGBTriple_t* )pOut;

	for ( int yBlock = 0; yBlock < nBlocksHeight; yBlock++ )
	{
		//RGBQuad_t* apScanline[4];
		for ( int xBlock = 0; xBlock < nBlocksWidth; xBlock++ )
		{
			//if ( xBlock == 8 )
			//	__asm int 3;

			// a new block
			buf.data = block;
#ifdef _DEBUG
			buf.iSize = BPTC_BLOCK_SIZE + 1;
#else
			buf.iSize = BPTC_BLOCK_SIZE;
#endif
			buf.iCurrentByte = 0;
			buf.iCurrentBit = 0;

			BC6ModeStruct* psMode;

			if ( true )
			{
				// read the mode
				//BC6ModeStruct* psMode;
				uint32_t eMode = ReadBits( &buf, 2 );
				if ( !( eMode & 0x2 ) )
				{
					psMode = &g_asBC6Mode1Bit[eMode];
				}
				else
				{
					uint32_t eMode2 = ReadBits( &buf, 3 );
					if ( !( eMode & 0x1 ) )
					{
						psMode = &g_asBC6Mode3Bits[eMode2];
					}
					else
					{
						if ( !( eMode2 & 0x4 ) )
						{
							//if ( eMode2 == 2 )
							//	__asm int 3;
							psMode = &g_asBC6Mode2Bits[eMode2];
						}
						else
						{
							// invalid block
							goto err;
						}
					}
					//eMode |= eMode2 << 2;
				}
				//if ( psMode->iModeNumber != eMode )
				//	__asm int 3;

				// reset the endpoints
				int nEndpoints = psMode->nSubsets * 2; 
				for ( int i = 0; i < nEndpoints; i++ )
				{
					for ( int j = 0; j < 3; j++ )
					{
						endpoints[i][j] = 0;
					}
				}

				// track the instruction list within the mode
				BC6Command_t* pcmd = psMode->pcmd;
				while ( pcmd->comp != END )
				{
					// read the bits first
					uint32_t bits = 0;
					int nBits = pcmd->a - pcmd->b;
					if ( nBits < 0 )
					{
						// reverse the order
						nBits = -nBits + 1;
						uint32_t rawbits = ReadBits( &buf, nBits );
						int iBit = pcmd->b;
						for ( int j = 0; j < nBits; j++ )
						{
							if ( iBit < 0 )
								__asm int 3;
							bits |= ( rawbits & 1 ) << iBit;
							rawbits >>= 1;
							iBit--;
						}
					}
					else
					{
						nBits++; // one bit more for this mode
						uint32_t rawbits = ReadBits( &buf, nBits );
						bits |= ( rawbits << pcmd->b );
					}

					// if we have any bits, do the instruction
					if ( bits )
					{
						// the instructions are now simple offsets to the endpoint components
						// instantinate as a 1D array
						int32_t* ae = ( int32_t* )endpoints;
						ae[pcmd->comp] |= bits;
					}

					pcmd++;
				}

				// sign extend and transform
				if ( bSigned || psMode->bTransform )
				{
					if ( !psMode->bTransform ) // signed only
					{
						int nBits = psMode->nEndpointBits;
						uint32_t iSignBit = 1 << ( nBits - 1 );
						uint32_t signExtendMask = ~( iSignBit - 1 );
						for ( int i = 0; i < nEndpoints; i++ )
						{
							for ( int j = 0; j < 3; j++ )
							{
								if ( endpoints[i][j] & iSignBit )
								{
									endpoints[i][j] |= signExtendMask;
								}
							}
						}
					}
					else // transfrom
					{
						// sign extend the first endpoint
						if ( bSigned )
						{
							int nBits = psMode->nEndpointBits;
							uint32_t iSignBit = 1 << ( nBits - 1 );
							uint32_t signExtendMask = ~( iSignBit - 1 );
							for ( int j = 0; j < 3; j++ )
							{
								if ( endpoints[0][j] & iSignBit )
								{
									endpoints[0][j] |= signExtendMask;
								}
							}
						}
						// add deltas
						for ( int j = 0; j < 3; j++ )
						{
							int nBits = psMode->nDeltaBits[j];
							uint32_t iSignBit = 1 << ( nBits - 1 );
							uint32_t signExtendMask = ~( iSignBit - 1 );
							uint32_t wrapMask = ( 1 << nBits ) - 1;
							for ( int i = 1; i < nEndpoints; i++ )
							{
								if ( endpoints[i][j] & iSignBit )
								{
									endpoints[i][j] |= signExtendMask;
								}

								endpoints[i][j] = endpoints[0][j] + endpoints[i][j];
								/*
								endpoints[i][j] &= wrapMask;
								if ( bSigned )
								{
									if ( endpoints[i][j] & iSignBit )
									{
										endpoints[i][j] |= signExtendMask;
									}
								}
								*/
								/*
								// clamp
								// XXX: a proper clamping should include the carry and borrow flags
								int32_t v = endpoints[i][j];
								if ( bSigned )
								{
									if ( v > 32767 )
									{
										v = 32767;
									}
									else if ( v < -32767 )
									{
										v = -32767;
									}
								}
								else
								{
									if ( v > 65535 )
									{
										v = 65535;
									}
								}
								endpoints[i][j] = v;
								*/
							}
						}
					}
				}

				// unquantize
				if ( true )
				{
					int nEndpointBits = psMode->nEndpointBits;
					int nBitsShift = 16 - nEndpointBits;
					//assert( nBitsShift >= 0 );
					//if ( nBitsShift )
					{
						uint32_t rounding = 0x5555 >> nEndpointBits;
						for ( int i = 0; i < nEndpoints; i++ )
						{
							for ( int j = 0; j < 3; j++ )
							{
								endpoints[i][j] = endpoints[i][j] << nBitsShift;
								//uint16_t l = *( uint16_t* )&endpoints[i][j];
								//uint32_t l = endpoints[i][j] & 0xFFFFFF;
								//endpoints[i][j] |= ( l >> nEndpointBits );
								endpoints[i][j] |= rounding;
							}
						}
					}
				}
				/*
				for ( int i = 0; i < nEndpoints; i++ )
				{
					fwrite( &endpoints[i][0], sizeof(int), 1,gstream);
					fwrite( &endpoints[i][1], sizeof(int), 1,gstream);
					fwrite( &endpoints[i][2], sizeof(int), 1,gstream);
				}
				*/

				// partition
				uint32_t iPartition = ReadBits( &buf, psMode->nPartitionBits );

				// read the indices
				int nIndexBits = psMode->nIndexBits;
				if ( psMode->nSubsets == 1 )
				{
					for ( int i = 0; i < 16; i++ )
					{
						bool bFixup = ( i == 0 );
						int nBits = bFixup ? nIndexBits - 1 : nIndexBits;
						indices[i] = ( uint8_t )ReadBits( &buf, nBits );
					}
				}
				else if ( psMode->nSubsets == 2 )
				{
					for ( int i = 0; i < 16; i++ )
					{
						bool bFixup = ( i == 0 ) || ( i == g_aiAnchorIndex21[iPartition] );
						int nBits = bFixup ? nIndexBits - 1 : nIndexBits;
						indices[i] = ( uint8_t )ReadBits( &buf, nBits );
					}
				}

				// lookup and interpolate the final colors
				if ( bSigned )
				{
					for ( int i = 0; i < 16; i++ )
					{
						int iSubset = GetSubset( psMode->nSubsets, iPartition, i );
						int32_t* color1 = endpoints[iSubset*2];
						int32_t* color2 = endpoints[iSubset*2+1];
						uint32_t iWeight = GetWeight( indices[i], nIndexBits );

						for ( int j = 0; j < 3; j++ )
						{
							int32_t c = SIGNED_INTERPOLATE_BASE_64( color1[j], color2[j], iWeight );

							// compose a half-float
							bool bSign = false;
							if ( c < 0 )
							{
								bSign = true;
								c = -c;
							}
							c = ( c * 31 ) / 32;
							uint16_t hf = ( uint16_t )c;
							if ( bSign )
							{
								hf |= 0x8000;
							}

							block_pixels[i][j] = hf;

							/*
							// XXX: debug only, will handle the half-float natively later
							// convert to full float
							float fl = HalfFloatToFloat( hf );
							// cut it down to a 8-bit integer
							fl*=4;
							//fl/=512;
							if ( fl > 1.0 )
								fl = 1.0;
							//fl = pow(fl, 0.2f);
							block_pixels[i][j] = ( uint8_t )( fl * 255.0 );
							*/
						}
						
						//block_pixels[i][3] = 255;
					}
				}
				else
				{
					for ( int i = 0; i < 16; i++ )
					{
						int iSubset = GetSubset( psMode->nSubsets, iPartition, i );
						int32_t* color1 = endpoints[iSubset*2];
						int32_t* color2 = endpoints[iSubset*2+1];
						uint32_t iWeight = GetWeight( indices[i], nIndexBits );

						for ( int j = 0; j < 3; j++ )
						{
							int32_t c = SIGNED_INTERPOLATE_BASE_64( color1[j], color2[j], iWeight );
							// compose a half-float
							c = ( c * 31 ) / 64;
							uint16_t hf = ( uint16_t )c;

							block_pixels[i][j] = hf;
							/*
							// convert to full float
							float fl = HalfFloatToFloat( hf );
							// cut it down to a 8-bit integer
							fl*=4;
							//fl/=512;
							if ( fl > 1.0 )
								fl = 1.0;
							//fl = pow(fl, 0.2f);
							block_pixels[i][j] = ( uint8_t )( fl * 255.0 );
							*/
						}
						
						//block_pixels[i][3] = 255;
					}
				}
			}

#ifdef _DEBUG
			if ( buf.iCurrentByte != BPTC_BLOCK_SIZE || buf.iCurrentBit != 0 )
			{
				__asm int 3;
			}
#endif
			goto jmp_over;

err:
			// fill with the error colors
			for ( int i = 0; i < 16; i++ )
			{
				for ( int j = 0; j < 3; j++ )
				{
					block_pixels[i][j] = 0;
				}
				//block_pixels[i][3] = 255;
			}

jmp_over:
			// copy the block colors to the destination
			for ( int y = 0; y < 4; y++ )
			{
				int yImage = ( yBlock * 4 ) + y;
				if ( yImage < iHeight )
				{
					for ( int x = 0; x < 4; x++ )
					{
						int xImage = ( xBlock * 4 ) + x;
						if ( xImage < iWidth )
						{
							int iImage = ( yImage * iWidth ) + xImage;
							int i = ( y * 4 ) + x;
							pixels[iImage].b = block_pixels[i][2];
							pixels[iImage].g = block_pixels[i][1];
							pixels[iImage].r = block_pixels[i][0];
						}
					}
				}
			}

			// point to the next block
			block = &block[BPTC_BLOCK_SIZE];
		}
	}

	//fclose(gstream);

	// return the count of bytes read
	return nBlocksWidth * nBlocksHeight * BPTC_BLOCK_SIZE;
}


unsigned int PLIB_DecodeBC6H_UF16( void* pOut, void* pIn, int iWidth, int iHeight, int iPitch, void* pPal )
{
	return _PLIB_DecodeBC6H( pOut, pIn, iWidth, iHeight, iPitch, pPal, false );
}


unsigned int PLIB_DecodeBC6H_SF16( void* pOut, void* pIn, int iWidth, int iHeight, int iPitch, void* pPal )
{
	return _PLIB_DecodeBC6H( pOut, pIn, iWidth, iHeight, iPitch, pPal, true );
}


unsigned int PLIB_DecodeBC7( void* pOut, void* pIn, int iWidth, int iHeight, int iPitch, void* pPal )
{
	//gstream = fopen("dump.bin", "wb");

	BitBuffer_t buf;
	uint8_t colors[6][4];
	uint8_t pbits[6];
	uint8_t indices1[16];
	uint8_t indices2[16];
	uint8_t block_pixels[16][4];

	// do the plib stuff
	if ( iWidth == 0 )
	{
		// width granularity
		return 4; // 4 pixels
	}
	else if ( iHeight == 0 )
	{
		// height granularity
		return 4; // 4 pixels
	}
	else if ( ( iWidth < 0 ) || ( iHeight < 0 ) )
	{
		// not supported here
		return 0;
	}

	int nBlocksWidth = ( iWidth + 3 ) / 4;
	int nBlocksHeight = ( iHeight + 3 ) / 4;

	// do the plib stuff
	if ( pIn == NULL )
	{
		// input buffer size
		return nBlocksWidth * nBlocksHeight * BPTC_BLOCK_SIZE;
	}
	else if ( pOut == NULL )
	{
		// output buffer size
		return iWidth * iHeight * sizeof( RGBQuad_t );
	}

	uint8_t* block = ( uint8_t* )pIn;
	RGBQuad_t* pixels = ( RGBQuad_t* )pOut;

	for ( int yBlock = 0; yBlock < nBlocksHeight; yBlock++ )
	{
		//RGBQuad_t* apScanline[4];
		for ( int xBlock = 0; xBlock < nBlocksWidth; xBlock++ )
		{
			// a new block
			buf.data = block;
			buf.iSize = BPTC_BLOCK_SIZE + 1; // for debug
			buf.iCurrentByte = 0;
			buf.iCurrentBit = 0;

			if ( true )
			{
				// read the mode
				int eMode;
				uint32_t c;
				for ( eMode = 0; eMode < 8; eMode++ )
				{
					c = ReadBits( &buf, 1 );
					if ( c )
						break;
				}
				if ( !c )
				{
					// invalid block
					goto err;
				}

				ModeStruct_t* psMode = &g_asMode[eMode];

				// partition (pattern)
				uint32_t iPartition = ReadBits( &buf, psMode->nPartitionBits );

				// rotation (rgb>alpha swap mode)
				uint32_t iRotation = ReadBits( &buf, psMode->nRotationBits );

				// index selection bit (primary>secondary index swap)
				uint32_t fIndexSelectionBit = ReadBits( &buf, psMode->nIndexSelectionBits );

				// read the colors (endpoints)
				int nEndpoints = psMode->nSubsets * 2; // there are 2 endpoints per a subset
				int nColorBits = psMode->nColorBits;
				for ( int j = 0; j < 3; j++ )
				{
					for ( int i = 0; i < nEndpoints; i++ )
					{
						colors[i][j] = ( uint8_t )ReadBits( &buf, nColorBits );
					}
				}
				int nAlphaBits = psMode->nAlphaBits;
				if ( nAlphaBits )
				{
					for ( int i = 0; i < nEndpoints; i++ )
					{
						colors[i][3] = ( uint8_t )ReadBits( &buf, nAlphaBits );
					}
				}

				// P-bits
				if ( psMode->bUniquePBits || psMode->bSharedPBits )
				{
					int nPBits = psMode->bUniquePBits ? nEndpoints : nEndpoints / 2;
					for ( int i = 0; i < nPBits; i++ )
					{
						pbits[i] = ( uint8_t )ReadBits( &buf, 1 );
					}

					// combine with the color
					for ( int i = 0; i < nEndpoints; i++ )
					{
						int iPBit = psMode->bUniquePBits ? i : i / 2;
						for ( int j = 0; j < 3; j++ )
						{
							colors[i][j] = ( colors[i][j] << 1 ) | pbits[iPBit];
						}
						if ( nAlphaBits )
						{
							colors[i][3] = ( colors[i][3] << 1 ) | pbits[iPBit];
						}
					}

					nColorBits++;
					if ( nAlphaBits )
					{
						nAlphaBits++;
					}
				}

				// unquantize the color and alpha
				int nColorBitsShift = 8 - nColorBits;
				if ( nColorBitsShift )
				{
					for ( int i = 0; i < nEndpoints; i++ )
					{
						for ( int j = 0; j < 3; j++ )
						{
							colors[i][j] = colors[i][j] << nColorBitsShift;
							colors[i][j] |= ( colors[i][j] >> nColorBits );
						}
					}
				}
				if ( nAlphaBits )
				{
					int nAlphaBitsShift = 8 - nAlphaBits;
					if ( nAlphaBitsShift )
					{
						for ( int i = 0; i < nEndpoints; i++ )
						{
							colors[i][3] = colors[i][3] << nAlphaBitsShift;
							colors[i][3] |= ( colors[i][3] >> nAlphaBits );
						}
					}
				}

				// read the indices
				int nIndexBits1 = psMode->nIndexBits;
				if ( psMode->nSubsets == 1 )
				{
					for ( int i = 0; i < 16; i++ )
					{
						bool bFixup = ( i == 0 );
						int nBits = bFixup ? nIndexBits1 - 1 : nIndexBits1;
						indices1[i] = ( uint8_t )ReadBits( &buf, nBits );
					}
				}
				else if ( psMode->nSubsets == 2 )
				{
					for ( int i = 0; i < 16; i++ )
					{
						bool bFixup = ( i == 0 ) || ( i == g_aiAnchorIndex21[iPartition] );
						int nBits = bFixup ? nIndexBits1 - 1 : nIndexBits1;
						indices1[i] = ( uint8_t )ReadBits( &buf, nBits );
					}
				}
				else // 3 subsets
				{
					for ( int i = 0; i < 16; i++ )
					{
						bool bFixup = ( i == 0 ) || ( i == g_aiAnchorIndex31[iPartition] ) || ( i == g_aiAnchorIndex32[iPartition] );
						int nBits = bFixup ? nIndexBits1 - 1 : nIndexBits1;
						indices1[i] = ( uint8_t )ReadBits( &buf, nBits );
					}
				}
				int nIndexBits2 = psMode->nSecondaryIndexBits;
				if ( nIndexBits2 )
				{
					// this mode assumes a single set (according to the documentation)
					for ( int i = 0; i < 16; i++ )
					{
						bool bFixup = ( i == 0 );
						int nBits = bFixup ? nIndexBits2 - 1 : nIndexBits2;
						indices2[i] = ( uint8_t )ReadBits( &buf, nBits );
					}
				}

				// lookup and interpolate the final colors
				if ( !nIndexBits2 )
				{
					// single index set
					for ( int i = 0; i < 16; i++ )
					{
						int iSubset = GetSubset( psMode->nSubsets, iPartition, i );
						uint8_t* color1 = colors[iSubset*2];
						uint8_t* color2 = colors[iSubset*2+1];
						uint32_t iWeight1 = GetWeight( indices1[i], nIndexBits1 );

						for ( int j = 0; j < 3; j++ )
						{
							block_pixels[i][j] = INTERPOLATE_BASE_64( color1[j], color2[j], iWeight1 );
						}
						if ( nAlphaBits )
						{
							block_pixels[i][3] = INTERPOLATE_BASE_64( color1[3], color2[3], iWeight1 );
						}
						else
						{
							block_pixels[i][3] = 255;
						}
					}
				}
				else
				{
					// two sets of indices
					for ( int i = 0; i < 16; i++ )
					{
						int iSubset = GetSubset( psMode->nSubsets, iPartition, i );
						uint8_t* color1 = colors[iSubset*2];
						uint8_t* color2 = colors[iSubset*2+1];
						uint32_t iWeight1 = GetWeight( indices1[i], nIndexBits1 );
						uint32_t iWeight2 = GetWeight( indices2[i], nIndexBits2 );

						if ( fIndexSelectionBit )
						{
							for ( int j = 0; j < 3; j++ )
							{
								block_pixels[i][j] = INTERPOLATE_BASE_64( color1[j], color2[j], iWeight2 );
							}
							if ( nAlphaBits )
							{
								block_pixels[i][3] = INTERPOLATE_BASE_64( color1[3], color2[3], iWeight1 );
							}
							else
							{
								block_pixels[i][3] = 255;
							}
						}
						else
						{
							for ( int j = 0; j < 3; j++ )
							{
								block_pixels[i][j] = INTERPOLATE_BASE_64( color1[j], color2[j], iWeight1 );
							}
							if ( nAlphaBits )
							{
								block_pixels[i][3] = INTERPOLATE_BASE_64( color1[3], color2[3], iWeight2 );
							}
							else
							{
								block_pixels[i][3] = 255;
							}
						}
					}
				}

				// swap channels if required
				if ( iRotation == 1 )
				{
					// red <-> alpha
					for ( int i = 0; i < 16; i++ )
					{
						uint8_t v = block_pixels[i][0];
						block_pixels[i][0] = block_pixels[i][3];
						block_pixels[i][3] = v;
					}
				}
				else if ( iRotation == 2 )
				{
					// green <-> alpha
					for ( int i = 0; i < 16; i++ )
					{
						uint8_t v = block_pixels[i][1];
						block_pixels[i][1] = block_pixels[i][3];
						block_pixels[i][3] = v;
					}
				}
				else if ( iRotation == 3 )
				{
					// blue <-> alpha
					for ( int i = 0; i < 16; i++ )
					{
						uint8_t v = block_pixels[i][2];
						block_pixels[i][2] = block_pixels[i][3];
						block_pixels[i][3] = v;
					}
				}
				/*
				for ( int i = 0; i < 16; i++ )
				{
					fwrite( &block_pixels[i][0], sizeof(char),1,gstream);
					fwrite( &block_pixels[i][1], sizeof(char),1,gstream);
					fwrite( &block_pixels[i][2], sizeof(char),1,gstream);
				}
				*/
			}

			goto jmp_over;

err:
			// fill with error colors
			for ( int i = 0; i < 16; i++ )
			{
				for ( int j = 0; j < 3; j++ )
				{
					block_pixels[i][j] = 0;
				}
				block_pixels[i][3] = 255;
			}

jmp_over:
			// debug
			if ( buf.iCurrentByte != BPTC_BLOCK_SIZE || buf.iCurrentBit != 0 )
			{
				__asm int 3;
			}

			// copy the block colors to the destination (also needs r and b swapping)
			for ( int y = 0; y < 4; y++ )
			{
				int yImage = ( yBlock * 4 ) + y;
				if ( yImage < iHeight )
				{
					for ( int x = 0; x < 4; x++ )
					{
						int xImage = ( xBlock * 4 ) + x;
						if ( xImage < iWidth )
						{
							int iImage = ( yImage * iWidth ) + xImage;
							int i = ( y * 4 ) + x;
							pixels[iImage].b = block_pixels[i][2];
							pixels[iImage].g = block_pixels[i][1];
							pixels[iImage].r = block_pixels[i][0];
							pixels[iImage].a = block_pixels[i][3];
						}
					}
				}
			}

			// point to the next block
			block = &block[BPTC_BLOCK_SIZE];
		}
	}

	//fclose(gstream);

	// return the count of bytes read
	return nBlocksWidth * nBlocksHeight * BPTC_BLOCK_SIZE;
}


