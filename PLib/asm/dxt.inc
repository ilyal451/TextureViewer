comment *

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

*

RGBA_COLOR STRUCT
	blue BYTE ?
	green BYTE ?
	red BYTE ?
	alpha BYTE ?
RGBA_COLOR ENDS

DXT_COLOR_BLOCK STRUCT
	colors WORD 2 dup(?)
	pixels BYTE 4 dup(?)
DXT_COLOR_BLOCK ENDS

DXT1_BLOCK STRUCT
	color DXT_COLOR_BLOCK <>
DXT1_BLOCK ENDS

DXT3_BLOCK STRUCT
	alpha WORD 4 dup(?)
	color DXT_COLOR_BLOCK <>
DXT3_BLOCK ENDS

DXT5_BLOCK STRUCT
	alpha BYTE 2 dup(?)
	adata BYTE 6 dup(?)
	color DXT_COLOR_BLOCK <>
DXT5_BLOCK ENDS

DXT5_ALPHA_BLOCK STRUCT
	alpha BYTE 2 dup(?)
	adata BYTE 6 dup(?)
DXT5_ALPHA_BLOCK ENDS

BC4_BLOCK STRUCT
	red DXT5_ALPHA_BLOCK <>
BC4_BLOCK ENDS

BC5_BLOCK STRUCT
	red DXT5_ALPHA_BLOCK <>
	green DXT5_ALPHA_BLOCK <>
BC5_BLOCK ENDS

DDS_PIXELFORMAT STRUCT
	dwSize DWORD ?
	dwFlags DWORD ?
	dwFourCC DWORD ?
	dwRGBBitCount DWORD ?
	dwRBitMask DWORD ?
	dwGBitMask DWORD ?
	dwBBitMask DWORD ?
	dwRGBAlphaBitMask DWORD ?
DDS_PIXELFORMAT ENDS

