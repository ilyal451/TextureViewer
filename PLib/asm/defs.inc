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

APIENTRY MACRO name, args:VARARG
align 16
name PROC args
ENDM

ENDENTRY MACRO name
name ENDP
ENDM

RETURN MACRO cb
	ret	cb * 4
ENDM

ALIGNDATA equ align 16
ALIGNCODE equ align 16

CH_B equ 00
CH_G equ 01
CH_R equ 02
CH_A equ 03

CH_0 equ 00
CH_1 equ 01
CH_2 equ 02
CH_3 equ 03

DEFAULT_COLOR equ 0
DEFAULT_ALPHA equ 255

DF_UNORM equ 0
DF_SNORM equ 1
DF_UINT equ 2
DF_SINT equ 3
DF_FLOAT equ 4

CS_8BIT equ 1
CS_16BIT equ 2
CS_32BIT equ 4

DPO_INT_NORM		equ 01H
DPO_SIGNED_RANGE	equ 02H
DPO_SWAP_XY		equ 10H
DPO_SWAP_LEFT_RIGHT	equ 20H
DPO_SWAP_TOP_BOTTOM	equ 40H


RECT STRUCT
	left DWORD ?
	top DWORD ?
	right DWORD ?
	bottom DWORD ?
RECT ENDS

GM_LINEAR equ 0
GM_SRGB equ 1
GM_SPECIFY equ 2

GAMMA STRUCT
	eGamma DWORD ?
	flGamma DWORD ?
GAMMA ENDS

; fpu control word flags (for fcom)
FPU_CONTROL_WORD	equ 0100011100000000b
FPU_GREATER		equ 0000000000000000b
FPU_LESS		equ 0000000100000000b
FPU_EQUAL		equ 0100000000000000b
FPU_CW_C0		equ 0000000100000000b
FPU_CW_C1		equ 0000001000000000b
FPU_CW_C2		equ 0000010000000000b
FPU_CW_C3		equ 0100000000000000b
