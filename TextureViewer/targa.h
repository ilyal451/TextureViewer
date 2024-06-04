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

#define TGA_NONE			0
#define TGA_PAL				1
#define TGA_RGB				2
#define TGA_BW				3
#define TGA_RLE_PAL			9
#define TGA_RLE_RGB			10
#define TGA_RLE_BW			11
//#define TGA_XZ			32
//#define TGA_XZ1			33

//#define TGAORIGIN_LEFT

#define TGA_INVERSELEFTRIGHT 0x10
#define TGA_INVERSETOPBOTTOM 0x20

#include <pshpack1.h>

typedef struct tga_file_header_s
{
	unsigned char iIDLen;
	unsigned char eColorMapType;
	unsigned char eImageType;
	struct { //color map specification
		unsigned short iStart;
		unsigned short iLength;
		unsigned char iEntrySize;
	} cm;
	struct { //image specification
		unsigned short xOrigin;
		unsigned short yOrigin;
		unsigned short iWidth;
		unsigned short iHeight;
		unsigned char iPixelSize;
		unsigned char flags;
	} img;
} tga_file_header_t;

#include <poppack.h>
