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
#include <math.h>

#include "../shared/plibnative.h"

#ifdef _WIN32
 #define _ALIGNED(v) ((((unsigned int)(v)) + 3) & (-4))
#endif
#ifdef _WIN64
 #define _ALIGNED(v) ((((unsigned int)(v)) + 7) & (-8))
#endif

// NOTE: changing this will require to check the entire code for overflows
#define C_MUL 0x100

#define NUM_CHANNELS 4

//typedef enum { false, true } bool;

extern "C" const unsigned short g_aiLinear2Linear[];
extern "C" const unsigned short g_aiSRGB2Linear[];
extern "C" const unsigned char g_aiLinear2SRGB_8bit[];
extern "C" const unsigned char g_aiLinear2SRGB[];

typedef void (*PFNSHARPENPIXELS)(byte_t* dst, uint_t** apaiScanTriple, int iDstWidth, int iBlurControl);


// linear (no sharpening), just save the center pixel
void SharpenPixels_Linear(byte_t* dst, uint_t** apaiScanTriple, int iWidth, int iBlurControl)
{
	for (int i = 0; i < iWidth; i++)
	{
		uint_t* paiCenterSum = &apaiScanTriple[1][(i+1)*NUM_CHANNELS];
		byte_t* dstPixel = &dst[i * NUM_CHANNELS];

		for (int j = 0; j < NUM_CHANNELS; j++)
		{
			word_t k = paiCenterSum[j] / C_MUL;
			dstPixel[j] = g_aiLinear2SRGB[k];
		}
	}
}


// the center pixel is sharpened by the following model:
//    *
//  * c *
//    *
// where * are the corresponding side pixels weightened by the blur factor
// TODO: the blur factor is still a subject to further tests...
void SharpenPixels_X4(byte_t* dst, uint_t** apaiScanTriple, int iWidth, int iBlurControl)
{
	int iBlur = 4 << iBlurControl;
	int iDiv = iBlur * C_MUL;

	for (int i = 0; i < iWidth; i++)
	{
		uint_t* paiCenter = &apaiScanTriple[1][(i+1)*NUM_CHANNELS]; // center
		uint_t* paiSide[4];
		paiSide[0] = &apaiScanTriple[1][(i+0)*NUM_CHANNELS]; // left
		paiSide[1] = &apaiScanTriple[1][(i+2)*NUM_CHANNELS]; // right
		paiSide[2] = &apaiScanTriple[0][(i+1)*NUM_CHANNELS]; // top
		paiSide[3] = &apaiScanTriple[2][(i+1)*NUM_CHANNELS]; // bottom

		byte_t* dstPixel = &dst[i * NUM_CHANNELS];

		for (int j = 0; j < NUM_CHANNELS; j++)
		{
			int v = paiCenter[j] * (iBlur + 4);
			for (int iSide = 0; iSide < 4; iSide++)
			{
				v -= paiSide[iSide][j];
			}
			
			//v /= iBlur;
			//v /= C_MUL;
			v /= iDiv;
			
			v = max(0, min(0xFFFF, v));

			dstPixel[j] = g_aiLinear2SRGB[v];
		}
	}
}


// as in the above
// the model has been changed to use 8 pixels:
//  * * *
//  * c *
//  * * *
void SharpenPixels_X8(byte_t* dst, uint_t** apaiScanTriple, int iWidth, int iBlurControl)
{
	int iBlur = 8 << iBlurControl;
	int iDiv = iBlur * C_MUL;

	for (int i = 0; i < iWidth; i++)
	{
		uint_t* paiCenter = &apaiScanTriple[1][(i+1)*NUM_CHANNELS]; // center
		uint_t* paiSide[8];
		paiSide[0] = &apaiScanTriple[1][(i+0)*NUM_CHANNELS]; // left
		paiSide[1] = &apaiScanTriple[1][(i+2)*NUM_CHANNELS]; // right
		paiSide[2] = &apaiScanTriple[0][(i+1)*NUM_CHANNELS]; // top
		paiSide[3] = &apaiScanTriple[2][(i+1)*NUM_CHANNELS]; // bottom
		paiSide[4] = &apaiScanTriple[0][(i+0)*NUM_CHANNELS]; // top left
		paiSide[5] = &apaiScanTriple[0][(i+2)*NUM_CHANNELS]; // top right
		paiSide[6] = &apaiScanTriple[2][(i+0)*NUM_CHANNELS]; // bottom left
		paiSide[7] = &apaiScanTriple[2][(i+2)*NUM_CHANNELS]; // bottom right

		byte_t* dstPixel = &dst[i * NUM_CHANNELS];

		for (int j = 0; j < NUM_CHANNELS; j++)
		{
			int v = paiCenter[j] * (iBlur + 8);
			for (int iSide = 0; iSide < 8; iSide++)
			{
				v -= paiSide[iSide][j];
			}

			v /= iDiv;
			v = max(0, min(0xFFFF, v));

			dstPixel[j] = g_aiLinear2SRGB[v];
		}
	}
}


// this model uses 8 pixels too, but the diagonal ones are weighted twice as less
// we use this version as the actual one (the above X8 is a dummy)
//  ! * !
//  * c *
//  ! * !
void SharpenPixels_X8A(byte_t* dst, uint_t** apaiScanTriple, int iWidth, int iBlurControl)
{
	int iBlur = 6 << iBlurControl;
	int iDiv = iBlur * C_MUL;

	for (int i = 0; i < iWidth; i++)
	{
		uint_t* paiCenter = &apaiScanTriple[1][(i+1)*NUM_CHANNELS]; // center
		uint_t* paiSide[8];
		paiSide[0] = &apaiScanTriple[1][(i+0)*NUM_CHANNELS]; // left
		paiSide[1] = &apaiScanTriple[1][(i+2)*NUM_CHANNELS]; // right
		paiSide[2] = &apaiScanTriple[0][(i+1)*NUM_CHANNELS]; // top
		paiSide[3] = &apaiScanTriple[2][(i+1)*NUM_CHANNELS]; // bottom
		paiSide[4] = &apaiScanTriple[0][(i+0)*NUM_CHANNELS]; // top left
		paiSide[5] = &apaiScanTriple[0][(i+2)*NUM_CHANNELS]; // top right
		paiSide[6] = &apaiScanTriple[2][(i+0)*NUM_CHANNELS]; // bottom left
		paiSide[7] = &apaiScanTriple[2][(i+2)*NUM_CHANNELS]; // bottom right

		byte_t* dstPixel = &dst[i * NUM_CHANNELS];

		for (int j = 0; j < NUM_CHANNELS; j++)
		{
			int v = paiCenter[j] * (iBlur + 6);
			for (int iSide = 0; iSide < 4; iSide++)
			{
				v -= paiSide[iSide][j];
			}
			for (int iSide = 4; iSide < 8; iSide++)
			{
				v -= paiSide[iSide][j] / 2;
			}

			v /= iDiv;
			v = max(0, min(0xFFFF, v));

			dstPixel[j] = g_aiLinear2SRGB[v];
		}
	}
}


// (out) pai - the weight of the src pixel in the current dst pixel
// (out) pab - determines the end of the current dst pixel, the next cycle will start a new pixel

int _InitWeightTable(word_t* pai, bool_t* pab, int iDstWidth, int iSrcWidth)
{
	int nPixelsOut = 0;
	int i;

	for (i = 0; i < iSrcWidth; i++)
	{
		int x1Int = (int)((((__int64)iDstWidth * C_MUL) * i) / iSrcWidth);
		int x2Int = (int)((((__int64)iDstWidth * C_MUL) * (i + 1)) / iSrcWidth);
		int x1Pixel = x1Int / C_MUL;
		int x2Pixel = x2Int / C_MUL;
		uint_t v;

		if (x2Pixel > x1Pixel)// new line
		{
			x2Int = (x2Int / C_MUL) * C_MUL;
			pab[i] = true;
			nPixelsOut++;
		}
		else
		{
			pab[i] = false;
		}

		v = x2Int - x1Int;

		pai[i] = (word_t)v;
	}

	if (!pab[i-1])
	{
		pab[i-1] = true;
		nPixelsOut++;
	}

	if (nPixelsOut != iDstWidth)
	{
		// XXX: we've cut this away but note that it gets triggered sometimes
		// primarily on uneven images like iWidth = 1251
		// UPD: seemingly solved... leave this here anyways
		//__asm int 3;
	}

	//return (iWidth * C_MUL) / iSrcWidth;
	return pai[0]; // the zero element is guaranteed to be of the full weight even if there's only one pixel
}


// linear interpolator (uses some sharpening techniques)
void PLIB_ShrinkPixelsLinear(byte_t* dst, int iDstWidth, int iDstHeight, byte_t* src, int iSrcWidth, int iSrcHeight, int eSharpeningType, int iBlurControl)
{
	// this may break something i believe...
	// TODO: anyways check if we can remove it now
	if ((iSrcWidth >= 0x8000) || (iSrcHeight >= 0x8000))
	{
		return;
	}

	// XXX: may be this should be AND? should we allow one-dimensional shrinking?
	// we don't allow the enlarging in this function
	if ((iDstWidth >= iSrcWidth) || (iDstHeight >= iSrcHeight))
	{
		return;
	}

	// this is a legacy thing, but i guess it does something...
	if (iBlurControl > 4)
	{
		return;
	}

	// sharpening method (if any)
	PFNSHARPENPIXELS pfnSharpenPixels;
	switch (eSharpeningType)
	{
	case SHARPEN_NONE:
		pfnSharpenPixels = SharpenPixels_Linear;
		break;
	case SHARPEN_X4:
		pfnSharpenPixels = SharpenPixels_X4;
		break;
	case SHARPEN_X8:
		pfnSharpenPixels = SharpenPixels_X8;
		break;
	case SHARPEN_X8A:
		pfnSharpenPixels = SharpenPixels_X8A;
		break;
	default:
		return;
	}

	// here we alloc one large buffer for everything, later we will divide it into smaller chunks
	int iDstPitch = iDstWidth * NUM_CHANNELS;
	int iSrcPitch = iSrcWidth * NUM_CHANNELS;
	int iDstXBufferSize = _ALIGNED(iDstPitch) * sizeof(word_t);
	int iDstScanLen = iDstWidth + 2;
	int iDstScanSize = _ALIGNED(iDstScanLen * NUM_CHANNELS * sizeof(uint_t));
	int iSrcXWeightTableSize = _ALIGNED(iSrcWidth * sizeof(word_t));
	int iSrcYWeightTableSize = _ALIGNED(iSrcHeight * sizeof(word_t));
	int iSrcXBreakTableSize = _ALIGNED(iSrcWidth * sizeof(bool_t));
	int iSrcYBreakTableSize = _ALIGNED(iSrcHeight * sizeof(bool_t));
	int iSizeOfBuffers = iDstXBufferSize + (iDstScanSize * 3) + iSrcXWeightTableSize + iSrcYWeightTableSize + iSrcXBreakTableSize + iSrcYBreakTableSize;
	byte_t* buffer = (byte_t*)malloc(iSizeOfBuffers);
	if ( buffer == NULL )
	{
		return;
	}
	// give all the buffers their offsets inside the main buffer
	int iBufferPos = 0;
	word_t* aiDstXBuffer = (word_t*)&buffer[iBufferPos]; // XBuffer is an accumulator for the first pass wich filters only the width, the filtered height then goes to the scan triple
	iBufferPos += iDstXBufferSize;
	uint_t* apaiDstScanTriple[3]; // the sharpening methods all use a tri-scanline interpolation, while the simple linear method uses only the center (current) scanline sum
	for (int i = 0; i < 3; i++)
	{
		apaiDstScanTriple[i] = (uint_t*)&buffer[iBufferPos];
		iBufferPos += iDstScanSize;
	}
	word_t* paiSrcXWeightTable = (word_t*)&buffer[iBufferPos];
	iBufferPos += iSrcXWeightTableSize;
	word_t* paiSrcYWeightTable = (word_t*)&buffer[iBufferPos];
	iBufferPos += iSrcYWeightTableSize;
	bool_t* pabSrcXBreakTable = (bool_t*)&buffer[iBufferPos];
	iBufferPos += iSrcXBreakTableSize;
	bool_t* pabSrcYBreakTable = (bool_t*)&buffer[iBufferPos];
	iBufferPos += iSrcYBreakTableSize;
	if ( iBufferPos != iSizeOfBuffers )
	{
		__asm int 3;
	}

	// init the tables
	// the weight tables are used to determine an src pixel weight in the dst pixel
	// the break tables are used to indicate where the current dst pixel completes and the next one begins
	uint_t xFullWeight = _InitWeightTable(paiSrcXWeightTable, pabSrcXBreakTable, iDstWidth, iSrcWidth);
	uint_t yFullWeight = _InitWeightTable(paiSrcYWeightTable, pabSrcYBreakTable, iDstHeight, iSrcHeight);

	// reset the scanline buffers
	/*
	for (int i = 0; i < iDstScanLen; i++)
	{
		for (j = 0; j < NUM_CHANNELS; j++)
		{
			int iOffset = i * NUM_CHANNELS + j;
			apaiDstScanTriple[0][iOffset] = 0;
			apaiDstScanTriple[1][iOffset] = 0;
			apaiDstScanTriple[2][iOffset] = 0;
		}
	}
	*/
	for ( int i = 0; i < 3; i++ )
	{
		memset( apaiDstScanTriple[i], 0, iDstScanSize );
	}

	// begin processing

	// go thru the scanlines
	int yDst = -1; // will increase it later
	for (int ySrc = 0; ySrc < iSrcHeight; ySrc++)
	{
		byte_t* srcScan = &src[ySrc * iSrcPitch];

		// the current dst pixel accumulator
		int xDst = 0;
		uint_t av[NUM_CHANNELS];
		for (int i = 0; i < NUM_CHANNELS; i++)
		{
			av[i] = 0;
		}
		
		// shrink the width
		for (int xSrc = 0; xSrc < iSrcWidth; xSrc++)
		{
			byte_t* srcPixel = &srcScan[xSrc * NUM_CHANNELS];
			word_t* dstPixel = &aiDstXBuffer[xDst * NUM_CHANNELS];
			uint_t xWeight = paiSrcXWeightTable[xSrc];

			// accumulate pixels
			for (int i = 0; i < NUM_CHANNELS; i++)
			{
				uint_t k = g_aiSRGB2Linear[srcPixel[i]];
				av[i] += (k * xWeight);
			}
				
			if (pabSrcXBreakTable[xSrc]) // indicates the end of the current dst pixel
			{
				xWeight = xFullWeight - xWeight;
				for (int i = 0; i < NUM_CHANNELS; i++)
				{
					dstPixel[i] = av[i] / C_MUL;
					// init with the next pixel's weight minus the part we've already processed
					uint_t k = g_aiSRGB2Linear[srcPixel[i]];
					av[i] = (k * xWeight);
				}
				xDst++;
			}
		}

		// add the obtained intermediate values to the scanline sum
		// (i.e. shrink the height, as opposed to the width in above)
		uint_t* dstScan = apaiDstScanTriple[2];
		uint_t yWeight = paiSrcYWeightTable[ySrc];
		for (int i = 0; i < iDstWidth; i++)
		{
			word_t* srcPixel = &aiDstXBuffer[i * NUM_CHANNELS];
			uint_t* dstPixel = &dstScan[(1 + i) * NUM_CHANNELS]; // XXX: why do we use 1 pixel offset here?
			for (int j = 0; j < NUM_CHANNELS; j++)
			{
				dstPixel[j] += ((uint_t)srcPixel[j] * yWeight);
			}
		}

		// if finish this pixel
		if (pabSrcYBreakTable[ySrc])
		{
			// copy the left- and the rightmost pixels
			// the sharpening algorithms require this, as they operate on 3x3 pixel blocks (with the center as the current pixel)
			// it's not quite correct, but it's cheaper than doing the checks inside the sharpener
			for (int i = 0; i < NUM_CHANNELS; i++)
			{
				dstScan[NUM_CHANNELS*0+i] = dstScan[NUM_CHANNELS*1+i];
				dstScan[NUM_CHANNELS*(iDstWidth+1)+i] = dstScan[NUM_CHANNELS*(iDstWidth)+i];
			}

			// do the stuff
			do
			{
				if (yDst != -1) // make sure we have processed at least two scanlines
				{
					// same as with the left- and the rightmost pixels in the above
					if (yDst == 0) // copy the top most
					{
						memcpy(apaiDstScanTriple[0], apaiDstScanTriple[1], iDstScanSize);
					}
					else if (yDst == iDstHeight -1) // the bottom most
					{
						memcpy(apaiDstScanTriple[2], apaiDstScanTriple[1], iDstScanSize);
					}

					// ready to fill the destination
					pfnSharpenPixels(&dst[yDst * iDstPitch], apaiDstScanTriple, iDstWidth, iBlurControl);
				}

				yDst++;

				// shift the scans upwards
				apaiDstScanTriple[2] = apaiDstScanTriple[0];
				apaiDstScanTriple[0] = apaiDstScanTriple[1];
				apaiDstScanTriple[1] = dstScan; // the current scanline (is the number 2)
				dstScan = apaiDstScanTriple[2];

			} while (yDst == (iDstHeight - 1)); // so, do it twice when yDst == height-1 (the last scanline), this is the same as skipping of the very first scanline in the beginning of the loop
			//} while (false);

			// init the start of the next dst scan
			yWeight = yFullWeight - yWeight;
			for (int i = 0; i < iDstWidth; i++)
			{
				word_t* srcPixel = &aiDstXBuffer[i * NUM_CHANNELS];
				uint_t* dstPixel = &dstScan[(1 + i) * NUM_CHANNELS];
				for (int j = 0; j < NUM_CHANNELS; j++)
				{
					dstPixel[j] = ((uint_t)srcPixel[j] * yWeight);
				}
			}
		}
	}

	// cleanup
	free(buffer);
}


// point interpolator (i.e. no interpolation at all)
void PLIB_ShrinkPixelsPoint(byte_t* dst, int iDstWidth, int iDstHeight, byte_t* src, int iSrcWidth, int iSrcHeight)
{
	// alloc several buffers at once
	// this reduces the count of times we will have to call to malloc and free
	// TODO: think of static buffers
	int iWidthLookupTableSize = sizeof(int) * iDstWidth;
	int iHeightLookupTableSize = sizeof(int) * iDstHeight;
	int iSizeOfBuffers = iWidthLookupTableSize + iHeightLookupTableSize;
	byte_t* buffer = (byte_t*)malloc(iSizeOfBuffers);
	if (buffer == NULL)
	{
		return;
	}
	// just init the tables with the offsets into the main buffer
	int iBufferPos = 0;
	int* aiWidthLookupTable = (int*)&buffer[iBufferPos];
	iBufferPos += iWidthLookupTableSize;
	int* aiHeightLookupTable = (int*)&buffer[iBufferPos];
	iBufferPos += iHeightLookupTableSize;
	if ( iBufferPos != iSizeOfBuffers )
	{
		__asm int 3;
	}

	// init the tables
	for (int i = 0; i < iDstWidth; i++)
	{
		aiWidthLookupTable[i] = (int)((((((__int64)iSrcWidth * C_MUL) * i) / iDstWidth)) / C_MUL);
	}
	for (int i = 0; i < iDstHeight; i++)
	{
		aiHeightLookupTable[i] = (int)((((((__int64)iSrcHeight * C_MUL) * i) / iDstHeight)) / C_MUL);
	}

	// copy pixels using the generated lookup tables
	int iDstPitch = iDstWidth * NUM_CHANNELS;
	int iSrcPitch = iSrcWidth * NUM_CHANNELS;
	for (int yDst = 0; yDst < iDstHeight; yDst++)
	{
		int ySrc = aiHeightLookupTable[yDst];
		byte_t* srcLine = &src[ySrc * iSrcPitch];
		byte_t* dstLine = &dst[yDst * iDstPitch];
		
		for (int xDst = 0; xDst < iDstWidth; xDst++)
		{
			int xSrc = aiWidthLookupTable[xDst];
			byte_t* srcPixel = &srcLine[xSrc * NUM_CHANNELS];
			byte_t* dstPixel = &dstLine[xDst * NUM_CHANNELS];

			*(uint_t*)dstPixel = *(uint_t*)srcPixel;
		}
	}

	// cleanup
	free(buffer);
}


void PLIB_CopyPixels(uint_t* dst, int iDstWidth, int iDstHeight, rect_t* prectDst, uint_t* src, int iSrcWidth, int iSrcHeight, rect_t* prectSrc)
{
	int xDstStart = max(0, prectDst->left);
	int xDstEnd = min(iDstWidth, prectDst->right);
	int xDstRange = xDstEnd - xDstStart;
	int yDstStart = max(0, prectDst->top);
	int yDstEnd = min(iDstHeight, prectDst->bottom);
	int yDstRange = yDstEnd - yDstStart;

	int xOffset = xDstStart - prectDst->left;
	int yOffset = yDstStart - prectDst->top;

	int xSrcStart = max(0, prectSrc->left);
	int ySrcStart = max(0, prectSrc->top);

	for (int y = 0; y < yDstRange; y++)
	{
		int yDst = y + yDstStart;
		int ySrc = y + ySrcStart + yOffset;
		uint_t* dstLine = &dst[yDst * iDstWidth];
		uint_t* srcLine = &src[ySrc * iSrcWidth];

		for (int x = 0; x < xDstRange; x++)
		{
			int xDst = x + xDstStart;
			int xSrc = x + xSrcStart + xOffset;
			uint_t* dstPixel = &dstLine[xDst];
			uint_t* srcPixel = &srcLine[xSrc];

			*dstPixel = *srcPixel;
		}
	}
}


// 
void PLIB_StretchPixelsPoint(uint_t* dst, int iDstWidth, int iDstHeight, rect_t* prectDst, uint_t* src, int iSrcWidth, int iSrcHeight, rect_t* prectSrc)
{
	int xDstStart = max(0, prectDst->left);
	int xDstEnd = min(iDstWidth, prectDst->right);
	int xDstRange = xDstEnd - xDstStart;
	int yDstStart = max(0, prectDst->top);
	int yDstEnd = min(iDstHeight, prectDst->bottom);
	int yDstRange = yDstEnd - yDstStart;

	int xOffset = xDstStart - prectDst->left;
	int yOffset = yDstStart - prectDst->top;
	int xRect = prectDst->right - prectDst->left;
	int yRect = prectDst->bottom - prectDst->top;

	// TODO: make no rect (disallow, only full image for src)
	int xSrcStart = max(0, prectSrc->left);
	int xSrcEnd = min(iSrcWidth, prectSrc->right);
	int xSrcRange = xSrcEnd - xSrcStart;
	int ySrcStart = max(0, prectSrc->top);
	int ySrcEnd = min(iSrcHeight, prectSrc->bottom);
	int ySrcRange = ySrcEnd - ySrcStart;

	for (int y = 0; y < yDstRange; y++)
	{
		int yDst = y + yDstStart;
		int yRPos = y + yOffset;
		int ySrc = ySrcStart + (float)(((float)ySrcRange / (float)yRect) * (float)yRPos);
		uint_t* dstLine = &dst[yDst * iDstWidth];
		uint_t* srcLine = &src[ySrc * iSrcWidth];

		for (int x = 0; x < xDstRange; x++)
		{
			int xDst = x + xDstStart;
			int xRPos = x + xOffset;
			int xSrc = xSrcStart + (float)(((float)xSrcRange / (float)xRect) * (float)xRPos);
			uint_t* dstPixel = &dstLine[xDst];
			uint_t* srcPixel = &srcLine[xSrc];

			*dstPixel = *srcPixel;
		}
	}
}


extern "C" {
void PLIB_ExtractRed( void* dst, int iCount );
void PLIB_ExtractGreen( void* dst, int iCount );
void PLIB_ExtractBlue( void* dst, int iCount );
void PLIB_ExtractAlpha( void* dst, int iCount );
void PLIB_OverlayAlpha( void* dst, int iCount, uint_t clrAlpha, int iOpacity );
}

 
PLibNativeFuncs_t g_nativeFuncs =
{
	PLIB_ShrinkPixelsPoint,
	PLIB_ShrinkPixelsLinear,
	PLIB_CopyPixels,
	PLIB_StretchPixelsPoint,

	PLIB_ExtractRed,
	PLIB_ExtractGreen,
	PLIB_ExtractBlue,
	PLIB_ExtractAlpha,
	PLIB_OverlayAlpha,

};
