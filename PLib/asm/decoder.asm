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

;//////////////////////////////////////////////////////////////////////

INCLUDE compile.inc

;//////////////////////////////////////////////////////////////////////

INCLUDE defs.inc
INCLUDE dxt.inc

;//////////////////////////////////////////////////////////////////////

.const

ALIGNDATA
g_c255 dd 255
g_c127 dd 127
ALIGNDATA
g_c1n127 dq 0.0078740157480314960 ;3F80204081020408

ALIGNDATA
g_color8_2	db 0000h, 00FFh
ALIGNDATA
g_color8_4	db 0000h, 0055h, 00AAh, 00FFh
ALIGNDATA
g_color8_8	db 0000h, 0024h, 0049h, 006Dh, 0092h, 00B6h, 00DBh, 00FFh
ALIGNDATA
g_color8_16	db 0000h, 0011h, 0022h, 0033h, 0044h, 0055h, 0066h, 0077h, 0088h, 0099h, 00AAh, 00BBh, 00CCh, 00DDh, 00EEh, 00FFh

ALIGNDATA
g_color16_2	dw 000000h, 00FFFFh
ALIGNDATA
g_color16_4	dw 000000h, 005555h, 00AAAAh, 00FFFFh
ALIGNDATA
g_color16_8	dw 000000h, 002492h, 004924h, 006DB6h, 009248h, 00B6DBh, 00DB6Dh, 00FFFFh
ALIGNDATA
g_color16_16	dw 000000h, 001111h, 002222h, 003333h, 004444h, 005555h, 006666h, 007777h, 008888h, 009999h, 00AAAAh, 00BBBBh, 00CCCCh, 00DDDDh, 00EEEEh, 00FFFFh


;//////////////////////////////////////////////////////////////////////

.code


INLINE_ABS MACRO dst, src

	mov	eax, src
	mov	edx, eax
	sar	edx, 31
	add	eax, edx
	xor	eax, edx
	mov	dst, eax

ENDM


;INLINE_ABS_old MACRO _dst, _src
;LOCAL @pos
;
;	mov	eax, _src
;	test	eax, eax
;	jns	@pos
;	neg	eax
;@pos:
;	mov	_dst, eax
;
;ENDM


; a generic 8-bit version (accepts any bit count)
; the input bits are in edx
; expands (or squeezes) everything to 8 bits
INLINE_DECODE_CHANNEL_8 MACRO _dst:REQ, _nBits:REQ, _bShift:REQ

	mov	eax, edx
 IF _bShift
	shr	edx, _nBits
 ENDIF
 IF (_nBits EQ 1)
	and	al, 1
	sub	al, 1
	not	al
 ELSEIF (_nBits EQ 2)
	and	eax, 3
	mov	al, BYTE PTR [g_color8_4+eax]
 ELSEIF (_nBits EQ 3)
	and	eax, 7
	mov	al, BYTE PTR [g_color8_8+eax]
 ELSEIF (_nBits EQ 4)
	and	eax, 15
	mov	al, BYTE PTR [g_color8_16+eax]
 ELSEIF (_nBits LT 8)
	shl	al, 8 - _nBits
	mov	ah, al
	shr	ah, _nBits
	or	al, ah
 ELSEIF (_nBits GT 8)
	; simply truncate
	shr	eax, _nBits - 8
 ENDIF
	mov	BYTE PTR _dst, al

ENDM


; NOTE: uses EBX! (i'm just out of registers)
; accepts any bit count
; everything -> 16 bits
; the input bits are in edx
INLINE_DECODE_CHANNEL_16 MACRO _dst, _nBits, _bShift:REQ

	mov	eax, edx
 IF _bShift
	shr	edx, _nBits
 ENDIF
 IF (_nBits EQ 1)
	and	ax, 1
	sub	ax, 1
	not	ax
 ELSEIF (_nBits EQ 2)
	and	eax, 3
	mov	ax, WORD PTR [g_color16_4+eax*2]
 ELSEIF (_nBits EQ 3)
	and	eax, 7
	mov	ax, WORD PTR [g_color16_8+eax*2]
 ELSEIF (_nBits EQ 4)
	and	eax, 15
	mov	ax, WORD PTR [g_color16_16+eax*2]
 ELSEIF (_nBits LT 8)
	shl	ax, 16 - _nBits
	mov	ebx, eax
	shr	bx, _nBits
	or	eax, ebx
	shr	bx, _nBits
	or	eax, ebx
 ELSEIF (_nBits LT 16)
	shl	ax, 16 - _nBits
	mov	ebx, eax
	shr	bx, _nBits
	or	eax, ebx
 ELSEIF (_nBits GT 16)
	shr	eax, _nBits - 16
 ENDIF
	mov	WORD PTR _dst, ax

ENDM


; just copies the channel
INLINE_COPY_CHANNEL_8 MACRO _dst, _src

	movzx	eax, BYTE PTR _src
	mov	BYTE PTR _dst, al

ENDM

INLINE_COPY_CHANNEL_16 MACRO _dst, _src

	movzx	eax, WORD PTR _src
	mov	WORD PTR _dst, ax

ENDM

INLINE_COPY_CHANNEL_32 MACRO _dst, _src

	mov	eax, DWORD PTR _src
	mov	DWORD PTR _dst, eax

ENDM


; XXX: should be used only for DXT
INLINE_DECODE_565COLOR MACRO _dst, _src, _bFillAlpha:REQ

	movzx	edx, WORD PTR _src
	INLINE_DECODE_CHANNEL_8 [_dst+CH_B], 5, 1
	INLINE_DECODE_CHANNEL_8 [_dst+CH_G], 6, 1
	INLINE_DECODE_CHANNEL_8 [_dst+CH_R], 5, 0
 IF (_bFillAlpha EQ 1)
	mov	BYTE PTR [_dst+CH_A], DEFAULT_ALPHA
 ENDIF

ENDM


; a macro for fast multiplication
INLINE_MULTIPLY MACRO _iWeight

 IF (_iWeight GT 1)
  IF (_iWeight EQ 2)
	add	eax, eax
  ELSEIF (_iWeight EQ 3)
	lea	eax, [eax+eax*2]
  ELSEIF (_iWeight EQ 4)
	shl	eax, 2
  ELSEIF (_iWeight EQ 5)
	lea	eax, [eax+eax*4]
  ELSEIF (_iWeight EQ 6)
	lea	eax, [eax+eax*2]
	add	eax, eax
  ELSE
	; just use regular multiply
	imul	eax, _iWeight
  ENDIF
 ENDIF

ENDM


; interpolates by weight (7 total)
; uses ecx
INLINE_INTERPOLATE_7 MACRO _dst, _c1, _c2, _w1, _bSigned

	movzx	eax, BYTE PTR _c1
 IF (_bSigned EQ 1)
	add	al, 80H
 ENDIF

	INLINE_MULTIPLY _w1

	mov	ecx, eax

	movzx	eax, BYTE PTR _c2
 IF (_bSigned EQ 1)
	add	al, 80H
 ENDIF

	INLINE_MULTIPLY 7 - _w1

	add	eax, ecx
	;add	eax, 3

	; optimized division (R > 0.5, F is rounded up)
	; divide by 7
	mov	edx, 24924925H
	mul	edx

 IF (_bSigned EQ 1)
	sub	dl, 80H
 ENDIF
	mov	BYTE PTR _dst, dl

ENDM


; interpolates by weight (5 total)
; uses ecx
INLINE_INTERPOLATE_5 MACRO _dst, _c1, _c2, _w1, _bSigned

	movzx	eax, BYTE PTR _c1
 IF (_bSigned EQ 1)
	add	al, 80H
 ENDIF

	INLINE_MULTIPLY _w1

	mov	ecx, eax

	movzx	eax, BYTE PTR _c2
 IF (_bSigned EQ 1)
	add	al, 80H
 ENDIF

	INLINE_MULTIPLY 5 - _w1

	add	eax, ecx
	;add	eax, 2

	; optimized division (R < 0.5)
	; divide by 5
	add	eax, 1
	mov	edx, 33333333H
	mul	edx

 IF (_bSigned EQ 1)
	sub	dl, 80H
 ENDIF
	mov	BYTE PTR _dst, dl

ENDM


; interpolates as 2:1
INLINE_INTERPOLATE_21 MACRO _dst, _c1, _c2

	movzx	eax, BYTE PTR _c1
	movzx	edx, BYTE PTR _c2
	lea	eax, [edx+eax*2]

	; optimized division (R < 0.5)
	add	eax, 1
	mov	edx, 55555555H
	mul	edx
	;

	mov	BYTE PTR _dst, dl

ENDM

; interpolates as 1:1
INLINE_INTERPOLATE_11 MACRO _dst, _c1, _c2

	movzx	eax, BYTE PTR _c1
	movzx	edx, BYTE PTR _c2
	add	eax, edx
	shr	eax, 1
	mov	BYTE PTR _dst, al

ENDM

; 2:1 interpolation
INLINE_INTERPOLATE_21_RGBA MACRO _dst, _c1, _c2

	INLINE_INTERPOLATE_21 _dst+CH_B, _c1+CH_B, _c2+CH_B
	INLINE_INTERPOLATE_21 _dst+CH_G, _c1+CH_G, _c2+CH_G
	INLINE_INTERPOLATE_21 _dst+CH_R, _c1+CH_R, _c2+CH_R
	mov	BYTE PTR _dst+CH_A, DEFAULT_ALPHA

ENDM

; 1:1 interpolation
INLINE_INTERPOLATE_11_RGBA MACRO _dst, _c1, _c2

	INLINE_INTERPOLATE_11 _dst+CH_B, _c1+CH_B, _c2+CH_B
	INLINE_INTERPOLATE_11 _dst+CH_G, _c1+CH_G, _c2+CH_G
	INLINE_INTERPOLATE_11 _dst+CH_R, _c1+CH_R, _c2+CH_R
	mov	BYTE PTR _dst+CH_A, DEFAULT_ALPHA

ENDM


; an optimized version (use it for dxt1 rather than dxtn)
INLINE_DECODE_DXT1_BLOCK MACRO _pBlock, _nBytesOut
 LOCAL @1BitAlpha, @X, @Y

	; decode the tables

	INLINE_DECODE_565COLOR colorlt[0*4], _pBlock.color.colors[0*2], 1
	INLINE_DECODE_565COLOR colorlt[1*4], _pBlock.color.colors[1*2], 1

	movzx	eax, _pBlock.color.colors[0*2]
	cmp	ax, _pBlock.color.colors[1*2]
	jbe	@1BitAlpha
	INLINE_INTERPOLATE_21_RGBA colorlt[2*4], colorlt[0*4], colorlt[1*4]
	INLINE_INTERPOLATE_21_RGBA colorlt[3*4], colorlt[1*4], colorlt[0*4]
	jmp	@F
@1BitAlpha:
	xor	eax, eax
	xor	edx, edx
	INLINE_INTERPOLATE_11_RGBA colorlt[2*4], colorlt[0*4], colorlt[1*4]
	xor	eax, eax
	mov	colorlt[3*4], eax ; a full zero color with zero alpha
@@:

	; process the pixels

	push	rdi

	xor	ebx, ebx	; iYY (row counter)
@Y:
	; a 2 bits per pixel encoding gives 4 pixels in a byte
	movzx	edx, BYTE PTR _pBlock.color.pixels[rbx]
	xor	ecx, ecx
@X:
	mov	eax, edx
	and	eax, 3
	mov	eax, colorlt[rax*4]
	mov	[rdi], eax	; 4 bytes out

	add	rdi, 4
	shr	edx, 2	; 2 bits per pixel
	add	ecx, 1
	cmp	ecx, iCellWidth
	jb	@X
@@:
	shl	ecx, 2 ; ecx is iCellWidth here, mul by the number of bytes out
	sub	rdi, rcx	; return the pointer to the row start
	mov	eax, iOutPitch	; offset to the next scanline
	add	rdi, rax
	add	ebx, 1
	cmp	ebx, iCellHeight
	jb	@Y

	pop rdi

ENDM


INLINE_PREPARE_DXT5_ALPHA_TABLE MACRO _table, _pBlock, _bSigned
 LOCAL @5Alpha

	movzx	eax, BYTE PTR _pBlock.alpha[0]
	movzx	edx, BYTE PTR _pBlock.alpha[1]

	mov	_table[0], al
	mov	_table[1], dl
	cmp	al, dl
 IF (_bSigned EQ 0)
	jbe	@5Alpha
 ELSE
	jle	@5Alpha
 ENDIF

	INLINE_INTERPOLATE_7 _table[2], _table[0], _table[1], 6, _bSigned
	INLINE_INTERPOLATE_7 _table[3], _table[0], _table[1], 5, _bSigned
	INLINE_INTERPOLATE_7 _table[4], _table[0], _table[1], 4, _bSigned
	INLINE_INTERPOLATE_7 _table[5], _table[0], _table[1], 3, _bSigned
	INLINE_INTERPOLATE_7 _table[6], _table[0], _table[1], 2, _bSigned
	INLINE_INTERPOLATE_7 _table[7], _table[0], _table[1], 1, _bSigned
	jmp	@F
@5Alpha:
	INLINE_INTERPOLATE_5 _table[2], _table[0], _table[1], 4, _bSigned
	INLINE_INTERPOLATE_5 _table[3], _table[0], _table[1], 3, _bSigned
	INLINE_INTERPOLATE_5 _table[4], _table[0], _table[1], 2, _bSigned
	INLINE_INTERPOLATE_5 _table[5], _table[0], _table[1], 1, _bSigned
	mov	BYTE PTR _table[6], 0
 IF (_bSigned EQ 0)
	mov	BYTE PTR _table[7], -1
 ELSE
	mov	BYTE PTR _table[7], 7FH
 ENDIF
@@:

ENDM


DXT1 equ 1
DXT3 equ 3
DXT5 equ 5

; the output is 4 bytes per pixel always, for BC5-6 use a different macro
INLINE_DECODE_DXTN_BLOCK MACRO _iDXTn:REQ, _pBlock:REQ, _nBytesOut
 LOCAL @1BitAlpha, @5Alpha, @X, @Y

	; decode the tables first

 IF (_iDXTn EQ DXT5)
	INLINE_PREPARE_DXT5_ALPHA_TABLE alphalt, _pBlock, 0
 ENDIF

 IF (_iDXTn EQ DXT1)
	INLINE_DECODE_565COLOR colorlt[0*4], _pBlock.color.colors[0*2], 1
	INLINE_DECODE_565COLOR colorlt[1*4], _pBlock.color.colors[1*2], 1

	mov	ax, _pBlock.color.colors[0*2]
	cmp	ax, _pBlock.color.colors[1*2]
	jbe	@1BitAlpha
 ELSE
	INLINE_DECODE_565COLOR colorlt[0*4], _pBlock.color.colors[0*2], 0
	INLINE_DECODE_565COLOR colorlt[1*4], _pBlock.color.colors[1*2], 0
 ENDIF

	INLINE_INTERPOLATE_21_RGBA colorlt[2*4], colorlt[0*4], colorlt[1*4]
	INLINE_INTERPOLATE_21_RGBA colorlt[3*4], colorlt[1*4], colorlt[0*4]

 IF (_iDXTn EQ DXT1)
	jmp	@F
@1BitAlpha:
	INLINE_INTERPOLATE_11_RGBA colorlt[2*4], colorlt[0*4], colorlt[1*4]
	xor	eax, eax
	mov	colorlt[3*4], eax ; a full zero color with zero alpha
@@:
 ENDIF

	; now process the pixels

	push	rdi

	xor	eax, eax
	mov	iYY, eax
@Y:
	movzx	edx, BYTE PTR _pBlock.color.pixels[rax]

 IF (_iDXTn EQ DXT3)
	movzx	ebx, WORD PTR _pBlock.alpha[rax*2]
 ELSEIF (_iDXTn EQ DXT5)
	test	eax, 1 ; eax is iYY here, skip if not even
	jnz	@F
	shr	eax, 1 ; there are two 24-bit values, iYY 0 and 1 will select the 1-st, 2 and 3 the 2-nd
	lea	eax, [eax+eax*2]
	mov	ebx, DWORD PTR _pBlock.adata[rax]
@@:
 ENDIF
	xor	ecx, ecx
@X:
	; pixel color
	mov	eax, edx
	and	eax, 3
	mov	eax, colorlt[rax*4]
	mov	[rdi], eax	; 4 bytes always

	; pixel alpha
 IF (_iDXTn EQ DXT3)
	mov	eax, ebx
	and	eax, 0FH
	mov	ah, al
	shl	al, 4
	or	al, ah
	mov	BYTE PTR [rdi+CH_A], al
	shr	ebx, 4
 ELSEIF (_iDXTn EQ DXT5)
	mov	eax, ebx
	and	eax, 7
	movzx	eax, BYTE PTR alphalt[rax]
	mov	BYTE PTR [rdi+CH_A], al
	shr	ebx, 3
 ENDIF

	shr	edx, 2			; 2 bits per pixel encoding
	add	rdi, 4			; 4 bytes out
	add	ecx, 1
	cmp	ecx, iCellWidth		; don't allow to excede the image borders
	jb	@X
@@:
	shl	ecx, 2			; ecx is iCellWidth here, so we multiply it by 4
	sub	rdi, rcx		; to obtain the num bytes written, then offset 
	mov	eax, iOutPitch		; the ptr back to the start of the cell,
	add	rdi, rax		; and then make it point to the next scanline
	mov	eax, iYY
	add	eax, 1
	cmp	eax, iCellHeight	; don't allow to excede the image borders
	mov	iYY, eax
	jb	@Y
@@:
	pop	rdi

ENDM


; especially for bc4
INLINE_DECODE_BC4_BLOCK MACRO _bSigned, _pBlock, _nBytesOut
 LOCAL @X, @Y

	; same as DXT5, but may be signed
	INLINE_PREPARE_DXT5_ALPHA_TABLE redlt, _pBlock.red, _bSigned

	push	rdi

	xor	ebx, ebx
@Y:
	mov	eax, ebx
	test	eax, 1 ; eax is iYY here, if not even - skip
	jnz	@F
	shr	eax, 1 ; there are two 24-bit values, iYY 0 and 1 will select the 1-st, 2 and 3 the 2-nd
	lea	eax, [eax+eax*2]
	mov	edx, DWORD PTR _pBlock.red.adata[rax]
@@:
	xor	rcx, rcx
@X:
	mov	eax, edx
	and	eax, 7
	movzx	eax, BYTE PTR redlt[rax]
	mov	BYTE PTR [rdi+CH_0], al

	add	rdi, 1
	shr	edx, 3
	add	ecx, 1
	cmp	ecx, iCellWidth
	jb	@X
@@:
	sub	rdi, rcx
	mov	eax, iOutPitch
	add	rdi, rax
	add	ebx, 1
	cmp	ebx, iCellHeight
	jb	@Y
@@:
	pop rdi

ENDM


BC4 equ 4
BC5 equ 5

INLINE_DECODE_BC_BLOCK MACRO _iBCn, _bSigned, _pBlock, _nBytesOut
 LOCAL @1BitAlpha, @5Alpha, @X, @Y

	; this format stores its colors in exactly the same way as DXT5 stores its alpha (with the exception that this may be signed, and DXT5 is never so)
	INLINE_PREPARE_DXT5_ALPHA_TABLE redlt, _pBlock.red, _bSigned
 IF (_iBCn EQ BC5)
	INLINE_PREPARE_DXT5_ALPHA_TABLE greenlt, _pBlock.green, _bSigned
 ENDIF

	push	rdi

	xor	eax, eax
	mov	iYY, eax
@Y:
	test	eax, 1 ; eax should be iYY here, skip uneven
	jnz	@F

	shr	eax, 1 ; there are two 24-bit values, iYY 0 and 1 will select the 1-st one, 2 and 3 the 2-nd
	lea	eax, [eax+eax*2]
	mov	edx, DWORD PTR _pBlock.red.adata[rax]
 IF (_iBCn EQ BC5)
	mov	ebx, DWORD PTR _pBlock.green.adata[rax]
 ENDIF
@@:
	xor	ecx, ecx
@X:
	mov	eax, edx
	shr	edx, 3
	and	eax, 7
	movzx	eax, BYTE PTR redlt[rax]
	mov	BYTE PTR [rdi+CH_0], al
 IF (_iBCn EQ BC5)
	mov	eax, ebx
	shr	ebx, 3
	and	eax, 7
	movzx	eax, BYTE PTR greenlt[rax]
	mov	BYTE PTR [rdi+CH_1], al
 ENDIF

	add	rdi, _nBytesOut
	add	ecx, 1
	cmp	ecx, iCellWidth
	jb	@X
@@:
 IF (_nBytesOut EQ 2)
	shl	ecx, 1			; ecx is iCellWidth here, BC4 - 1 byte per pixel, BC5 - 2
 ENDIF
	sub	rdi, rcx		; go back at the number of bytes written
	mov	eax, iOutPitch
	add	rdi, rax		; point to the next scanline
	mov	eax, iYY
	add	eax, 1
	cmp	eax, iCellHeight
	mov	iYY, eax
	jb	@Y
@@:
	pop	rdi

ENDM


DXT_GRANULARITY EQU 4

TEMPLATE_PROC_DECODE_DXT MACRO c_aFuncName:REQ, c_iBlockSize:REQ, c_nBytesOut:REQ, c_fnDo:REQ
 APIENTRY PLIB_Decode&c_aFuncName, USES rsi rdi rbx, dst:PTR BYTE, src:PTR BYTE, iWidth:DWORD, iHeight:DWORD, iPitch:DWORD, pPal:PTR BYTE

	; NOTE: not all of these vars are used in all the derived functions
	%LOCAL iYY		:DWORD	; YY is used inside the block processing macros only
	%LOCAL iX		:DWORD	; X and Y are the current pos (in pixels)
	%LOCAL iY		:DWORD	;  they are increased by 4 since the blocks are 4x4 pixels
	%LOCAL iCellWidth	:DWORD	; cell width and height determine the actual block pixels
	%LOCAL iCellHeight	:DWORD	;  to fill the current block with
	%LOCAL iAbsWidth	:DWORD	; we just abs() the width and height here as
	%LOCAL iAbsHeight	:DWORD	;  the dxt functions do not support negative widths and heights
	%LOCAL iOutPitch	:DWORD	; helper const used to offset the pointer inside the blocks
	%LOCAL colorlt[4]	:DWORD	; for all DXT, the interpolated color lookup table
	%LOCAL alphalt[8]	:BYTE	; for DXT5, the interpolated alpha lookup table
	%LOCAL redlt[8]		:BYTE	; for BC
	%LOCAL greenlt[8]	:BYTE	; for BC5

	; compute the granularities if we should
	mov	eax, iWidth
	test	eax, eax
	jnz	@F
	mov	eax, DXT_GRANULARITY
	ret
@@:
	mov	eax, iHeight
	test	eax, eax
	jnz	@F
	mov	eax, DXT_GRANULARITY
	ret
@@:
	mov	rsi, src

	; no negative widths/heights here
	mov	eax, iWidth
	test	eax, eax
	js	@error
	mov	iAbsWidth, eax

	mov	eax, iHeight
	test	eax, eax
	js	@error
	mov	iAbsHeight, eax

	; compute the output pitch
	mov	eax, iAbsWidth
	xor	edx, edx
	imul	eax, c_nBytesOut
	mov	iOutPitch, eax

	; NOTE: the input pitch (iPitch value) is not used here 
	; because it would relate on the block input, which is 4x4 pixels anyway...

	; get the input size?
	mov	rax, src
	test	rax, rax
	jz	@getinsize

	; ???
	mov	rax, dst
	test	rax, rax
	jz	@getoutsize

	; start the loop
	xor	eax, eax
	mov	iY, eax
@Y:
	; scanline = dst + y * iOutPitch
	mov	eax, iOutPitch
	xor	edx, edx
	mul	iY
	add	rax, dst
	mov	rdi, rax

	xor	eax, eax
	mov	iX, eax
@X:
	; should support image sizes not multiple of 4
	; in this case we just adjust the cell width/height 
	; at the end of the scanline/image where needed
	mov	eax, iAbsHeight
	sub	eax, iY
	cmp	eax, 4
	jna	@F
	mov	eax, 4
@@:
	mov	iCellHeight, eax

	mov	eax, iAbsWidth
	sub	eax, iX
	cmp	eax, 4
	jna	@F
	mov	eax, 4
@@:
	mov	iCellWidth, eax

	; do the block
	;---------
	%c_fnDo, c_nBytesOut
	;---------

	add	rsi, c_iBlockSize
	add	rdi, 4*c_nBytesOut	; 4 pixels forward (note, not bytes)

	mov	eax, iX
	add	eax, 4
	cmp	eax, iAbsWidth
	mov	iX, eax
	jb	@X
@@:
	mov	eax, iY
	add	eax, 4
	cmp	eax, iAbsHeight
	mov	iY, eax
	jb	@Y
@@:
	; done

@getinsize:

	; iSize = ((iAbsWidth + 3) / 4) * ((iAbsHeight + 3) / 4) * iBlockSize
	mov	eax, iAbsWidth
	add	eax, 3
	shr	eax, 2
	mov	ecx, iAbsHeight
	add	ecx, 3
	shr	ecx, 2
	xor	edx, edx
	mul	ecx
	mov	ecx, c_iBlockSize
	mul	ecx

	ret

@getoutsize:

	; iSize = iOutPitch * iAbsHeight
	mov	eax, iAbsHeight
	mov	ecx, iOutPitch
	xor	edx, edx
	mul	ecx

	ret

@error:

	xor	eax, eax

	ret

 ENDENTRY PLIB_Decode&c_aFuncName
ENDM



; DXT formats

; XXX: ASSUME keyword doesn't work on ml64...

ASSUME rsi:PTR DXT1_BLOCK
;;;TEMPLATE_PROC_DECODE_DXT BC1, SIZEOF DXT1_BLOCK, 4, <INLINE_DECODE_DXTN_BLOCK DXT1, (PTR DXT1_BLOCK[rsi])>
TEMPLATE_PROC_DECODE_DXT BC1, SIZEOF DXT1_BLOCK, 4, <INLINE_DECODE_DXT1_BLOCK [rsi]>

ASSUME rsi:PTR DXT3_BLOCK
TEMPLATE_PROC_DECODE_DXT BC2, SIZEOF DXT3_BLOCK, 4, <INLINE_DECODE_DXTN_BLOCK DXT3, [rsi]>

ASSUME rsi:PTR DXT5_BLOCK
TEMPLATE_PROC_DECODE_DXT BC3, SIZEOF DXT5_BLOCK, 4, <INLINE_DECODE_DXTN_BLOCK DXT5, [rsi]>

ASSUME rsi:PTR BC4_BLOCK
TEMPLATE_PROC_DECODE_DXT BC4_UNORM, SIZEOF BC4_BLOCK, 1, <INLINE_DECODE_BC_BLOCK BC4, 0, [rsi]>
TEMPLATE_PROC_DECODE_DXT BC4_SNORM, SIZEOF BC4_BLOCK, 1, <INLINE_DECODE_BC_BLOCK BC4, 1, [rsi]>

ASSUME rsi:PTR BC5_BLOCK
TEMPLATE_PROC_DECODE_DXT BC5_UNORM, SIZEOF BC5_BLOCK, 2, <INLINE_DECODE_BC_BLOCK BC5, 0, [rsi]>
TEMPLATE_PROC_DECODE_DXT BC5_SNORM, SIZEOF BC5_BLOCK, 2, <INLINE_DECODE_BC_BLOCK BC5, 1, [rsi]>

;ASSUME rsi:NOTHING




;//////////////////////////////////////////////////////////////////////


;
; RGBG and YUYV interleaved formats
;


INLINE_DECODE_RGBG MACRO _CG, _CR, _CB

	movzx	eax, BYTE PTR [rsi+_CB]
	mov	BYTE PTR [rdi+CH_B], al
	movzx	eax, BYTE PTR [rsi+_CG]
	mov	BYTE PTR [rdi+CH_G], al
	movzx	eax, BYTE PTR [rsi+_CR]
	mov	BYTE PTR [rdi+CH_R], al

ENDM


; CY = Y - 16
; DU = U - 128
; EV = V - 128
; R = ((CY * 298) + (EV * 409) + 128) >> 8
; G = ((CY * 298) - (DU * 100) - (EV * 208) + 128) >> 8
; B = ((CY * 298) + (DU * 516) + 128) >> 8

CLIP_EAX MACRO
LOCAL @sign, @above, @quit

	test	eax, 0FFFFFF00H
	js	@sign
	jnz	@above
	jmp	@quit
@sign:
	xor	eax, eax
	jmp	@quit
@above:
	mov	eax, 255
@quit:

ENDM

g_flY dd 1.164383
g_flUB dd 2.017232
g_flUG dd 0.391762
g_flVR dd 1.596027
g_flVG dd 0.812968


INLINE_YUV_TO_RGB1 MACRO dst, CY, CU, CV
LOCAL @ns

	push rax

	movzx eax, BYTE PTR CY
	sub eax, 16
	mov [rsp], eax
	fild DWORD PTR [rsp]
	fmul g_flY

	movzx eax, BYTE PTR CU
	sub eax, 128
	mov [rsp], eax
	fild DWORD PTR [rsp]

	movzx eax, BYTE PTR CV
	sub eax, 128
	mov [rsp], eax
	fild DWORD PTR [rsp]

	; st(0) - v-128
	; st(1) - u-128
	; st(2) - y-16*1.164383

	fld st(0) ; v
	fmul g_flVR
	fld st(3) ; y
	fadd
	fistp DWORD PTR [rsp]
	mov eax, [rsp]
	CLIP_EAX
	mov BYTE PTR dst+CH_R, al

	fld st(0) ; v
	fmul g_flVG
	fld st(2) ; u
	fmul g_flUG
	fld st(4) ; y
	fsubr
	fsubr
	fistp DWORD PTR [rsp]
	mov eax, [rsp]
	CLIP_EAX
	mov BYTE PTR dst+CH_G, al

	fld st(1) ; u
	fmul g_flUB
	fld st(3) ; y
	fadd
	fistp DWORD PTR [rsp]
	mov eax, [rsp]
	CLIP_EAX
	mov BYTE PTR dst+CH_B, al

	fstp st(0)
	fstp st(0)
	fstp st(0)

	pop rax

ENDM


; this version is faster
INLINE_YUV_TO_RGB2 MACRO dst, CY, CU, CV

	movzx eax, BYTE PTR CY
	sub eax, 16
	imul eax, 298
	movzx edx, BYTE PTR CU
	sub edx, 128
	imul edx, 516
	add eax, edx
	add eax, 128
	sar eax, 8
	CLIP_EAX
	mov BYTE PTR dst+CH_B, al

	movzx eax, BYTE PTR CY
	sub eax, 16
	imul eax, 298
	movzx edx, BYTE PTR CU
	sub edx, 128
	imul edx, 100
	sub eax, edx
	movzx edx, BYTE PTR CV
	sub edx, 128
	imul edx, 208
	sub eax, edx
	add eax, 128
	sar eax, 8
	CLIP_EAX
	mov BYTE PTR dst+CH_G, al

	movzx eax, BYTE PTR CY
	sub eax, 16
	imul eax, 298
	movzx edx, BYTE PTR CV
	sub edx, 128
	imul edx, 409
	add eax, edx
	add eax, 128
	sar eax, 8
	CLIP_EAX
	mov BYTE PTR dst+CH_R, al

ENDM


INLINE_YUV_TO_RGB MACRO _CY, _CU, _CV

	movzx	ecx, BYTE PTR [rsi+_CY]
	sub	ecx, 16
	imul	ecx, 298

	mov	eax, ecx
	movzx	edx, BYTE PTR [rsi+_CU]
	sub	edx, 128
	imul	edx, 516
	add	eax, edx
	add	eax, 128
	sar	eax, 8
	CLIP_EAX
	mov	BYTE PTR [rdi+CH_B], al

	mov	eax, ecx
	movzx	edx, BYTE PTR [rsi+_CU]
	sub	edx, 128
	imul	edx, 100
	sub	eax, edx
	movzx	edx, BYTE PTR [rsi+_CV]
	sub	edx, 128
	imul	edx, 208
	sub	eax, edx
	add	eax, 128
	sar	eax, 8
	CLIP_EAX
	mov	BYTE PTR [rdi+CH_G], al

	mov	eax, ecx
	movzx	edx, BYTE PTR [rsi+_CV]
	sub	edx, 128
	imul	edx, 409
	add	eax, edx
	add	eax, 128
	sar	eax, 8
	CLIP_EAX
	mov	BYTE PTR [rdi+CH_R], al

ENDM


INLINE_DECODE_AYUV MACRO

	INLINE_YUV_TO_RGB 2, 1, 0
	movzx	eax, BYTE PTR [rsi+CH_3]
	mov	BYTE PTR [rdi+CH_A], al

ENDM


; this function should not support negative width
TEMPLATE_PROC_DECODE_UYVY MACRO _aFuncName:REQ, _fnDo:REQ, _iG0, _iG1, _iR0, _iB0
 APIENTRY PLIB_Decode&_aFuncName, USES rsi rdi rbx, dst:PTR BYTE, src:PTR BYTE, iWidth:DWORD, iHeight:DWORD, iPitch:DWORD, pPal:PTR BYTE

	%LOCAL iAddY		:PTR BYTE	; scanline to scanline offset
	%LOCAL iY		:DWORD		; counter
	%LOCAL iAbsWidth	:DWORD		; the negative widths are not allowed but anyhow it's here...
	%LOCAL iAbsHeight	:DWORD		; abs(iHeight)
	%LOCAL iActualPitch	:DWORD		; iPitch or ((iAbsWidth + 1) / 2) * 4

	;int	3

	; compute the granularities if we should
	mov	eax, iWidth
	test	eax, eax
	jnz	@F
	mov	eax, 2	; 2 bytes per pixel
	ret
@@:
	mov	eax, iHeight
	test	eax, eax
	jnz	@F
	mov	eax, 1
	ret
@@:
	; source/dest
	mov	rdi, dst
	mov	rsi, src

	; width/height should be positive
	mov	eax, iWidth
	test	eax, eax
	js	@error
	mov	iAbsWidth, eax

	; get the input pitch
	mov	ebx, iPitch
	test	ebx, ebx
	jnz	@F
					; compute the pitch as
	mov	ebx, iAbsWidth		; ((iAbsWidth + 1) / 2) * 4
	add	ebx, 1
	shr	ebx, 1
	shl	ebx, 2
@@:
	mov	iActualPitch, ebx

	xor	rax, rax		; zero if the height is positive
	mov	iAddY, rax
	mov	ebx, iHeight
	test	ebx, ebx
	js	@error
	mov	iAbsHeight, ebx

	; the the input size if we should
	mov	rax, src
	test	rax, rax
	jz	@getsize

	; begin the loop
	mov	eax, iAbsHeight
	mov	iY, eax
@Y:
	mov	ebx, iAbsWidth

	sub	rsi, 4
align 16
@X:
	; read two pixels at once
	add	rsi, 4

	; first pixel
	%_fnDo _iG0, _iR0, _iB0
	add	rdi, 4
	sub	ebx, 1
	jz	@F

	; second pixel
	%_fnDo _iG1, _iR0, _iB0
	add	rdi, 4
	sub	ebx, 1
	jnz	@X
@@:
	add	rsi, 4
	add	rdi, iAddY
	sub	iY, 1
	jnz	@Y

@getsize:

	mov	eax, iActualPitch
	imul	eax, iAbsHeight

	ret

@error:

	xor	eax, eax

	ret

 ENDENTRY PLIB_Decode&_aFuncName
ENDM

TEMPLATE_PROC_DECODE_UYVY RGBG, <INLINE_DECODE_RGBG>, 1, 3, 0, 2
TEMPLATE_PROC_DECODE_UYVY GRGB, <INLINE_DECODE_RGBG>, 0, 2, 1, 3
TEMPLATE_PROC_DECODE_UYVY UYVY, <INLINE_YUV_TO_RGB>, 1, 3, 0, 2
TEMPLATE_PROC_DECODE_UYVY YUY2, <INLINE_YUV_TO_RGB>, 0, 2, 1, 3


;//////////////////////////////////////////////////////////////////////






;
; RGB
;



INLINE_DECODE_R8 MACRO

	movzx	eax, BYTE PTR [rsi]
	mov	BYTE PTR [rdi], al

ENDM


INLINE_DECODE_R8G8 MACRO _bSwap:REQ

 IF (_bSwap EQ 0)
	movzx	eax, WORD PTR [rsi]
	mov	WORD PTR [rdi], ax
 ELSE
	movzx	eax, BYTE PTR [rsi+CH_0]
	mov	BYTE PTR [rdi+CH_1], al
	movzx	eax, BYTE PTR [rsi+CH_1]
	mov	BYTE PTR [rdi+CH_0], al
 ENDIF

ENDM


INLINE_DECODE_R8G8B8 MACRO _bSwap:REQ, _bExpand:REQ

 IF (_bSwap EQ 0)
	movzx	eax, WORD PTR [rsi]
	mov	WORD PTR [rdi], ax
	movzx	eax, BYTE PTR [rsi+CH_2]
	mov	BYTE PTR [rdi+CH_2], al
 ELSE
	movzx	eax, BYTE PTR [rsi+CH_0]
	mov	BYTE PTR [rdi+CH_2], al
	movzx	eax, BYTE PTR [rsi+CH_1]
	mov	BYTE PTR [rdi+CH_1], al
	movzx	eax, BYTE PTR [rsi+CH_2]
	mov	BYTE PTR [rdi+CH_0], al
 ENDIF

ENDM


INLINE_DECODE_R8G8B8X8 MACRO _bSwap:REQ, _bTruncate:REQ

 IF (_bSwap EQ 0)
  IF (_bTruncate EQ 0)
	mov	eax, DWORD PTR [rsi]
	mov	DWORD PTR [rdi], eax
  ELSE
	movzx	eax, WORD PTR [rsi]
	mov	WORD PTR [rdi], ax
	movzx	eax, BYTE PTR [rsi+CH_2]
	mov	BYTE PTR [rdi+CH_2], al
  ENDIF
 ELSE
	movzx	eax, BYTE PTR [rsi+CH_0]
	mov	BYTE PTR [rdi+CH_2], al
	movzx	eax, BYTE PTR [rsi+CH_1]
	mov	BYTE PTR [rdi+CH_1], al
	movzx	eax, BYTE PTR [rsi+CH_2]
	mov	BYTE PTR [rdi+CH_0], al
 ENDIF

ENDM


INLINE_DECODE_R8G8B8A8 MACRO _bSwap:REQ

 IF (_bSwap EQ 0)
	mov	eax, DWORD PTR [rsi]
	mov	DWORD PTR [rdi], eax
 ELSE
	movzx	eax, BYTE PTR [rsi+CH_0]
	mov	BYTE PTR [rdi+CH_2], al
	movzx	eax, BYTE PTR [rsi+CH_1]
	mov	BYTE PTR [rdi+CH_1], al
	movzx	eax, BYTE PTR [rsi+CH_2]
	mov	BYTE PTR [rdi+CH_0], al
	movzx	eax, BYTE PTR [rsi+CH_3]
	mov	BYTE PTR [rdi+CH_3], al
 ENDIF

ENDM


INLINE_DECODE_X8X8X8A8 MACRO

	movzx	eax, BYTE PTR [rsi+CH_3]
	mov	BYTE PTR [rdi], al

ENDM


INLINE_DECODE_R16 MACRO

	movzx	eax, WORD PTR [rsi]
	mov	WORD PTR [rdi], ax

ENDM


INLINE_DECODE_R16G16 MACRO _bSwap:REQ

 IF (_bSwap EQ 0)
	mov	eax, DWORD PTR [rsi]
	mov	DWORD PTR [rdi], eax
 ELSE
	movzx	eax, WORD PTR [rsi+CH_0*2]
	mov	WORD PTR [rdi+CH_1*2], ax
	movzx	eax, WORD PTR [rsi+CH_1*2]
	mov	WORD PTR [rdi+CH_0*2], ax
 ENDIF

ENDM


INLINE_DECODE_R16G16B16 MACRO _bSwap:REQ

 IF (_bSwap EQ 0)
	mov	eax, DWORD PTR [rsi]
	mov	DWORD PTR [rdi], eax
	movzx	eax, WORD PTR [rsi+CH_2*2]
	mov	WORD PTR [rdi+CH_2*2], ax
 ELSE
	movzx	eax, WORD PTR [rsi+CH_0*2]
	mov	WORD PTR [rdi+CH_2*2], ax
	movzx	eax, WORD PTR [rsi+CH_1*2]
	mov	WORD PTR [rdi+CH_1*2], ax
	movzx	eax, WORD PTR [rsi+CH_2*2]
	mov	WORD PTR [rdi+CH_0*2], ax
 ENDIF

ENDM


INLINE_DECODE_R16G16B16X16 MACRO _bSwap:REQ

 IF (_bSwap EQ 0)
	mov	eax, DWORD PTR [rsi]
	mov	DWORD PTR [rdi], eax
	movzx	eax, WORD PTR [rsi+CH_2*2]
	mov	WORD PTR [rdi+CH_2*2], ax
 ELSE
	movzx	eax, WORD PTR [rsi+CH_0*2]
	mov	WORD PTR [rdi+CH_2*2], ax
	movzx	eax, WORD PTR [rsi+CH_1*2]
	mov	WORD PTR [rdi+CH_1*2], ax
	movzx	eax, WORD PTR [rsi+CH_2*2]
	mov	WORD PTR [rdi+CH_0*2], ax
 ENDIF

ENDM


INLINE_DECODE_R16G16B16A16 MACRO _bSwap:REQ

 IF (_bSwap EQ 0)
  IFDEF _X64
	mov	rax, QWORD PTR [rsi]
	mov	QWORD PTR [rdi], rax
  ELSE
	mov	eax, DWORD PTR [rsi]
	mov	DWORD PTR [rdi], eax
	mov	eax, DWORD PTR [rsi+CH_2*2]
	mov	DWORD PTR [rdi+CH_2*2], eax
  ENDIF
 ELSE
	movzx	eax, WORD PTR [rsi+CH_0*2]
	mov	WORD PTR [rdi+CH_2*2], ax
	movzx	eax, WORD PTR [rsi+CH_1*2]
	mov	WORD PTR [rdi+CH_1*2], ax
	movzx	eax, WORD PTR [rsi+CH_2*2]
	mov	WORD PTR [rdi+CH_0*2], ax
	movzx	eax, WORD PTR [rsi+CH_3*2]
	mov	WORD PTR [rdi+CH_3*2], ax
 ENDIF

ENDM


INLINE_DECODE_X16X16X16A16 MACRO

	movzx	eax, WORD PTR [rsi+CH_3*2]
	mov	WORD PTR [rdi], ax

ENDM


INLINE_DECODE_R32 MACRO

	mov	eax, DWORD PTR [rsi]
	mov	DWORD PTR [rdi], eax

ENDM


INLINE_DECODE_R32G32 MACRO _bSwap:REQ

 IF (_bSwap EQ 0)
  IFDEF _X64
	mov	rax, QWORD PTR [rsi]
	mov	QWORD PTR [rdi], rax
  ELSE
	mov	eax, DWORD PTR [rsi+CH_0*4]
	mov	DWORD PTR [rdi+CH_0*4], eax
	mov	eax, DWORD PTR [rsi+CH_1*4]
	mov	DWORD PTR [rdi+CH_1*4], eax
  ENDIF
 ELSE
	mov	eax, DWORD PTR [rsi+CH_0*4]
	mov	DWORD PTR [rdi+CH_1*4], eax
	mov	eax, DWORD PTR [rsi+CH_1*4]
	mov	DWORD PTR [rdi+CH_0*4], eax
 ENDIF

ENDM


INLINE_DECODE_R32G32B32 MACRO _bSwap:REQ

 IF (_bSwap EQ 0)
  IFDEF _X64
	mov	rax, QWORD PTR [rsi]
	mov	QWORD PTR [rdi], rax
	mov	eax, DWORD PTR [rsi+CH_2*4]
	mov	DWORD PTR [rdi+CH_2*4], eax
  ELSE
	mov	eax, DWORD PTR [rsi+CH_0*4]
	mov	DWORD PTR [rdi+CH_0*4], eax
	mov	eax, DWORD PTR [rsi+CH_1*4]
	mov	DWORD PTR [rdi+CH_1*4], eax
	mov	eax, DWORD PTR [rsi+CH_2*4]
	mov	DWORD PTR [rdi+CH_2*4], eax
  ENDIF
 ELSE
	mov	eax, DWORD PTR [rsi+CH_0*4]
	mov	DWORD PTR [rdi+CH_2*4], eax
	mov	eax, DWORD PTR [rsi+CH_1*4]
	mov	DWORD PTR [rdi+CH_1*4], eax
	mov	eax, DWORD PTR [rsi+CH_2*4]
	mov	DWORD PTR [rdi+CH_0*4], eax
 ENDIF

ENDM


INLINE_DECODE_R32G32B32X32 MACRO _bSwap:REQ

 IF (_bSwap EQ 0)
  IFDEF _X64
	mov	rax, QWORD PTR [rsi]
	mov	QWORD PTR [rdi], rax
	mov	eax, DWORD PTR [rsi+CH_2*4]
	mov	DWORD PTR [rdi+CH_2*4], eax
  ELSE
	mov	eax, DWORD PTR [rsi+CH_0*4]
	mov	DWORD PTR [rdi+CH_0*4], eax
	mov	eax, DWORD PTR [rsi+CH_1*4]
	mov	DWORD PTR [rdi+CH_1*4], eax
	mov	eax, DWORD PTR [rsi+CH_2*4]
	mov	DWORD PTR [rdi+CH_2*4], eax
  ENDIF
 ELSE
	mov	eax, DWORD PTR [rsi+CH_0*4]
	mov	DWORD PTR [rdi+CH_2*4], eax
	mov	eax, DWORD PTR [rsi+CH_1*4]
	mov	DWORD PTR [rdi+CH_1*4], eax
	mov	eax, DWORD PTR [rsi+CH_2*4]
	mov	DWORD PTR [rdi+CH_0*4], eax
 ENDIF

ENDM


INLINE_DECODE_R32G32B32A32 MACRO _bSwap:REQ

 IF (_bSwap EQ 0)
  IFDEF _X64
	mov	rax, QWORD PTR [rsi]
	mov	QWORD PTR [rdi], rax
	mov	rax, QWORD PTR [rsi+CH_2*4]
	mov	QWORD PTR [rdi+CH_2*4], rax
  ELSE
	mov	eax, DWORD PTR [rsi+CH_0*4]
	mov	DWORD PTR [rdi+CH_0*4], eax
	mov	eax, DWORD PTR [rsi+CH_1*4]
	mov	DWORD PTR [rdi+CH_1*4], eax
	mov	eax, DWORD PTR [rsi+CH_2*4]
	mov	DWORD PTR [rdi+CH_2*4], eax
	mov	eax, DWORD PTR [rsi+CH_3*4]
	mov	DWORD PTR [rdi+CH_3*4], eax
  ENDIF
 ELSE
	mov	eax, DWORD PTR [rsi+CH_0*4]
	mov	DWORD PTR [rdi+CH_2*4], eax
	mov	eax, DWORD PTR [rsi+CH_1*4]
	mov	DWORD PTR [rdi+CH_1*4], eax
	mov	eax, DWORD PTR [rsi+CH_2*4]
	mov	DWORD PTR [rdi+CH_0*4], eax
	mov	eax, DWORD PTR [rsi+CH_3*4]
	mov	DWORD PTR [rdi+CH_3*4], eax
 ENDIF

ENDM


INLINE_DECODE_X32X32X32A32 MACRO

	mov	eax, DWORD PTR [rsi+CH_3*4]
	mov	DWORD PTR [rdi], eax

ENDM






;
; packed integer formats
;


INLINE_DECODE_R3G3B2 MACRO _iCH_0, _iCH_1, _iCH_2

	movzx	edx, BYTE PTR [rsi]
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_0], 3, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_1], 3, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_2], 2, 0

ENDM


INLINE_DECODE_B2G3R3 MACRO _iCH_0, _iCH_1, _iCH_2

	movzx	edx, BYTE PTR [rsi]
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_0], 2, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_1], 3, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_2], 3, 0

ENDM


INLINE_DECODE_R3G3B2A8 MACRO _iCH_0, _iCH_1, _iCH_2

	movzx	edx, WORD PTR [rsi]
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_0], 3, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_1], 3, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_2], 2, 1
	mov	BYTE PTR [rdi+CH_3], dl

ENDM


INLINE_DECODE_B2G3R3A8 MACRO _iCH_0, _iCH_1, _iCH_2

	movzx	edx, WORD PTR [rsi]
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_0], 2, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_1], 3, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_2], 3, 1
	mov	BYTE PTR [rdi+CH_3], dl

ENDM


INLINE_DECODE_R2G2B2X2 MACRO _iCH_0, _iCH_1, _iCH_2

	movzx	edx, BYTE PTR [rsi]
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_0], 2, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_1], 2, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_2], 2, 0

ENDM


INLINE_DECODE_R2G2B2A2 MACRO _iCH_0, _iCH_1, _iCH_2

	movzx	edx, BYTE PTR [rsi]
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_0], 2, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_1], 2, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_2], 2, 1
	INLINE_DECODE_CHANNEL_8 [rdi+CH_3], 2, 0

ENDM


INLINE_DECODE_R4G4B4X4 MACRO _iCH_0, _iCH_1, _iCH_2

	movzx	edx, WORD PTR [rsi]
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_0], 4, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_1], 4, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_2], 4, 0

ENDM


INLINE_DECODE_R4G4B4A4 MACRO _iCH_0, _iCH_1, _iCH_2

	movzx	edx, WORD PTR [rsi]
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_0], 4, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_1], 4, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_2], 4, 1
	INLINE_DECODE_CHANNEL_8 [rdi+CH_3], 4, 0

ENDM


INLINE_DECODE_R5G6B5 MACRO _iCH_0, _iCH_1, _iCH_2

	movzx	edx, WORD PTR [rsi]
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_0], 5, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_1], 6, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_2], 5, 0

ENDM


INLINE_DECODE_R5G5B5X1 MACRO _iCH_0, _iCH_1, _iCH_2

	movzx	edx, WORD PTR [rsi]
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_0], 5, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_1], 5, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_2], 5, 0

ENDM


INLINE_DECODE_R5G5B5A1 MACRO _iCH_0, _iCH_1, _iCH_2

	movzx	edx, WORD PTR [rsi]
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_0], 5, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_1], 5, 1
	INLINE_DECODE_CHANNEL_8 [rdi+_iCH_2], 5, 1
	INLINE_DECODE_CHANNEL_8 [rdi+CH_3], 1, 0

ENDM


INLINE_DECODE_R10G10B10X2 MACRO _iCH_0, _iCH_1, _iCH_2

	mov	edx, DWORD PTR [rsi]
	INLINE_DECODE_CHANNEL_16 [rdi+_iCH_0*2], 10, 1
	INLINE_DECODE_CHANNEL_16 [rdi+_iCH_1*2], 10, 1
	INLINE_DECODE_CHANNEL_16 [rdi+_iCH_2*2], 10, 0

ENDM


INLINE_DECODE_R10G10B10A2 MACRO _iCH_0, _iCH_1, _iCH_2

	mov	edx, DWORD PTR [rsi]
	INLINE_DECODE_CHANNEL_16 [rdi+_iCH_0*2], 10, 1
	INLINE_DECODE_CHANNEL_16 [rdi+_iCH_1*2], 10, 1
	INLINE_DECODE_CHANNEL_16 [rdi+_iCH_2*2], 10, 1
	INLINE_DECODE_CHANNEL_16 [rdi+CH_3*2], 2, 0

ENDM


; uses EBX!
INLINE_DECODE_CH10_SNORM MACRO _dst, _bShift:REQ

	mov	eax, edx
 IF _bShift
	shr	edx, 10
 ENDIF
	shl	ax, 16 - 10
	add	ax, 8000H
	mov	ebx, eax
	shr	bx, 10
	or	eax, ebx
	sub	ax, 8000H

	mov	WORD PTR _dst, ax

ENDM


INLINE_DECODE_R10G10B10X2_SNORM MACRO _iCH_0, _iCH_1, _iCH_2

	mov	edx, DWORD PTR [rsi]
	INLINE_DECODE_CH10_SNORM [rdi+_iCH_0*2], 1
	INLINE_DECODE_CH10_SNORM [rdi+_iCH_1*2], 1
	INLINE_DECODE_CH10_SNORM [rdi+_iCH_2*2], 0

ENDM


INLINE_DECODE_X10X10X10A2 MACRO

	mov	edx, DWORD PTR [rsi]
	shr	edx, 30
	INLINE_DECODE_CHANNEL_8 [rdi], 2, 0

ENDM





INLINE_DECODE_CHANNEL_16_UINT MACRO _dst, _nBits, _bShift:REQ

	mov	eax, edx
 IF _bShift
	shr	edx, _nBits
 ENDIF
	; TODO: use a mask better
	shl	ax, 16 - _nBits
	shr	ax, 16 - _nBits

	mov	WORD PTR _dst, ax

ENDM

INLINE_DECODE_R10G10B10A2_UINT MACRO _iCH_0, _iCH_1, _iCH_2

	mov	edx, DWORD PTR [rsi]
	INLINE_DECODE_CHANNEL_16_UINT [rdi+_iCH_0*2], 10, 1
	INLINE_DECODE_CHANNEL_16_UINT [rdi+_iCH_1*2], 10, 1
	INLINE_DECODE_CHANNEL_16_UINT [rdi+_iCH_2*2], 10, 1
	INLINE_DECODE_CHANNEL_16_UINT [rdi+CH_3*2], 2, 0

ENDM


; TODO: 2.8 fixed point format (can be stored in a half-float but requires normalization)
INLINE_DECODE_CHANNEL_XR_BIAS MACRO _dst, _nBits, _bShift:REQ

	mov	WORD PTR _dst, ax

ENDM

INLINE_DECODE_R10G10B10X2_XR_BIAS MACRO _iCH_0, _iCH_1, _iCH_2

	mov	edx, DWORD PTR [rsi]
	xor	eax, eax
	INLINE_DECODE_CHANNEL_XR_BIAS [rdi+_iCH_0*2], 10, 1
	INLINE_DECODE_CHANNEL_XR_BIAS [rdi+_iCH_1*2], 10, 1
	INLINE_DECODE_CHANNEL_XR_BIAS [rdi+_iCH_2*2], 10, 0

ENDM





; the input bits in edx
INLINE_DECODE_SE_COMPONENT MACRO _dst, _nBits, _bShift

	mov	eax, edx
IF (_bShift EQ 1)
	shr	edx, _nBits
ENDIF
IF (_nBits EQ 9)
	and	eax, 1FFH
ELSE
	error
ENDIF
	mov	DWORD PTR _dst, eax
	fild	DWORD PTR _dst
	fscale
	fstp	DWORD PTR _dst

ENDM

; this macro expects some space under rsp/esp
; the output is a triple of 32-bit floats
INLINE_DECODE_R9G9B9E5_FLOAT MACRO _iCH_0, _iCH_1, _iCH_2

	mov	edx, DWORD PTR [rsi]
	mov	eax, edx
	shr	eax, 27	; extract the exponent
	sub	eax, 15 + 9 ; apply the bias
	; do it simple for now
	mov	DWORD PTR [rsp], eax
	fild	DWORD PTR [rsp]
	INLINE_DECODE_SE_COMPONENT [rdi+_iCH_0*4], 9, 1
	INLINE_DECODE_SE_COMPONENT [rdi+_iCH_1*4], 9, 1
	INLINE_DECODE_SE_COMPONENT [rdi+_iCH_2*4], 9, 0
	fstp	st(0)

ENDM

; this format is very similar to the half-float one, so the conversion is pretty simple
INLINE_DECODE_PF_COMPONENT MACRO _dst, _nBits, _bShift
LOCAL @quit

	mov	eax, edx
IF (_bShift EQ 1)
	shr	edx, _nBits
ENDIF
IF (_nBits EQ 11)
	and	eax, 0000011111111111b
	test	eax, 0000011111000000b	; test for denorm or zero
	jz	@quit
	shl	eax, 4
ELSEIF (_nBits EQ 10)
	and	eax, 0000001111111111b
	test	eax, 0000001111100000b	; test for denorm or zero
	jz	@quit
	shl	eax, 5
ELSE
	error
ENDIF
@quit:
	mov	WORD PTR _dst, ax

ENDM


INLINE_DECODE_R11G11B10_FLOAT MACRO _iCH_0, _iCH_1, _iCH_2

	mov	edx, DWORD PTR [rsi]
	INLINE_DECODE_PF_COMPONENT [rdi+_iCH_0*2], 11, 1
	INLINE_DECODE_PF_COMPONENT [rdi+_iCH_1*2], 11, 1
	INLINE_DECODE_PF_COMPONENT [rdi+_iCH_2*2], 10, 0

ENDM


INLINE_DECODE_B10G11R11_FLOAT MACRO _iCH_0, _iCH_1, _iCH_2

	mov	edx, DWORD PTR [rsi]
	INLINE_DECODE_PF_COMPONENT [rdi+_iCH_0*2], 10, 1
	INLINE_DECODE_PF_COMPONENT [rdi+_iCH_1*2], 11, 1
	INLINE_DECODE_PF_COMPONENT [rdi+_iCH_2*2], 11, 0

ENDM
















; packed alpha-luminance (D3DFMT_A4L4)
; output 16-bit 2 component L:A
INLINE_DECODE_L4A4 MACRO

	movzx	edx, BYTE PTR [rsi]
	INLINE_DECODE_CHANNEL_8 [rdi+CH_0], 4, 1
	INLINE_DECODE_CHANNEL_8 [rdi+CH_1], 4, 0

ENDM


INLINE_DECODE_P4X4 MACRO

	movzx	eax, BYTE PTR [rsi]
	and	al, 0FH
	mov	BYTE PTR [rdi], al

ENDM


INLINE_DECODE_X4A4 MACRO

	movzx	eax, BYTE PTR [rsi]
	shl	al, 4
	mov	dl, al
	shr	dl, 4
	or	al, dl
	mov	BYTE PTR [rdi], al

ENDM


INLINE_DECODE_L8A8 MACRO

	movzx	eax, WORD PTR [rsi]
	mov	WORD PTR [rdi], ax

ENDM

; P8A8 (D3DFMT_A8P8)
; output 8-bit palette indices
INLINE_DECODE_P8X8 MACRO

	movzx	eax, BYTE PTR [rsi]
	mov	BYTE PTR [rdi], al

ENDM

; output 8-bit alpha
INLINE_DECODE_X8A8 MACRO

	movzx	eax, BYTE PTR [rsi+1]
	mov	BYTE PTR [rdi], al

ENDM


INLINE_DECODE_L16A16 MACRO

	mov	eax, DWORD PTR [rsi]
	mov	DWORD PTR [rdi], eax

ENDM


;
; DirectX packed bump DuDv formats
;

; U8V8CX (D3DFMT_CxV8U8)
; input 16-bit, UV, signed
; the C component is calculated as sqrt(1.0-(U^2)-(V^2))
; output 3-byte, U>>R,V>>G,C>>B
; TODO: think about loading some constants onto the stack
INLINE_DECODE_U8V8CX1 MACRO

	movsx	eax, BYTE PTR [rsi]
	mov	BYTE PTR [rdi+CH_0], al
	mov	DWORD PTR [rsp], eax
	fild	DWORD PTR [rsp]
	fmul	QWORD PTR g_c1n127
	fmul	st(0), st(0)

	movsx	eax, BYTE PTR [rsi+1]
	mov	BYTE PTR [rdi+CH_1], al
	mov	DWORD PTR [rsp], eax
	fild	DWORD PTR [rsp]
	fmul	QWORD PTR g_c1n127
	fmul	st(0), st(0)

	fld1
	fsubr
	fsubr
	fsqrt

	fimul	DWORD PTR g_c127
	fistp	DWORD PTR [rsp]

	mov	eax, DWORD PTR [rsp]
	mov	BYTE PTR [rdi+CH_2], al

ENDM

; store it simple for now
INLINE_DECODE_U8V8CX MACRO

	movzx	eax, WORD PTR [rsi]
	mov	WORD PTR [rdi], ax

ENDM


; U5V5L6 (D3DFMT_L6V5U5)
; in - packed 16-bit, UV - signed, L - unsigned
; output 2x8-bit, U>>R, V>>G
INLINE_DECODE_U5V5X6 MACRO

	movzx	edx, WORD PTR [rsi]
	INLINE_DECODE_CHANNEL_8 [rdi+CH_1], 5, 1
	INLINE_DECODE_CHANNEL_8 [rdi+CH_0], 5, 0

ENDM

; output 8-bit, L>>R
INLINE_DECODE_X5X5L6 MACRO

	movzx	edx, WORD PTR [rsi]
	shr	edx, 10
	INLINE_DECODE_CHANNEL_8 [rdi+CH_0], 5, 0

ENDM


; U8V8L8X8 (D3DFMT_X8L8V8U8)
; in - 4x8-bit, UV - signed, L - usnigned, X - none
; output 3x8-bit, U>>R, V>>G
INLINE_DECODE_U8V8X8X8 MACRO

	mov	eax, DWORD PTR [rsi]
	mov	BYTE PTR [rdi+CH_1], al
	mov	BYTE PTR [rdi+CH_0], ah

ENDM

; output 8-bit, L>>R
INLINE_DECODE_X8X8L8X8 MACRO

	movzx	eax, BYTE PTR [rsi+02]
	mov	BYTE PTR [rdi+CH_0], al

ENDM



; depth/stencil

INLINE_DECODE_D24X8 MACRO

	mov	eax, DWORD PTR [rsi]
	shl	eax, 8
	mov	edx, eax
	shr	edx, 24
	or	eax, edx
	mov	DWORD PTR [rdi], eax

ENDM


INLINE_DECODE_X24S8 MACRO

	movzx	eax, BYTE PTR [rsi+3]
	mov	BYTE PTR [rdi], al

ENDM


INLINE_DECODE_D15X1 MACRO

	movzx	eax, WORD PTR [rsi]
	shl	eax, 1
	mov	edx, eax
	shr	dx, 15
	or	eax, edx
	mov	WORD PTR [rdi], ax

ENDM


INLINE_DECODE_X15S1 MACRO

	movzx	eax, WORD PTR [rsi]
	shr	eax, 15
	mov	BYTE PTR [rdi], al

ENDM


INLINE_DECODE_D32X8X24 MACRO

	mov	eax, DWORD PTR [rsi]
	mov	DWORD PTR [rdi], eax

ENDM


INLINE_DECODE_X32S8X24 MACRO

	movzx	eax, BYTE PTR [rsi+4]
	mov	BYTE PTR [rdi], al

ENDM









;//////////////////////////////////////////////////////////////////////



;
; TEMPLATE_PROC_DECODE_RGB
;


comment '

gap=pitch-(width*bpp)
case w+/h+:
  source=source
  dx=bpp
  dy=gap
case w+/h-:
  source=pitch*(height-1)
  dx=bpp
  dy=gap-2*pitch
case w-/h+:
  source=bpp*(width-1)
  dx=-bpp
  dy=2*pitch-gap
case w-/h-:
  source=pitch*(height-1)+bpp*(width-1)
  dx=-bpp
  dy=-gap

where:
 bpp = bytes per pixel
 w = the width parameter (signed); + for the positive; - for the negative
 h = the height parameter (signed); + for the positive; - for the negative
 width = width (absolute)
 height = height (absolute)
 pitch = the actual pitch value (absolute)
 dx = delta x (iAddX)
 dy = delta y (iAddY)
 gap = is the gap between the scanlines (often zero)
 source = the source ptr (rsi)

'

; to give you an idea what the function calculates:
; iAddY offsets rsi to the next scanline
; the value of iAddY will be:
;  w  h: iAddY = ~(0) forward
; -w  h: iAddY = ~(w*2) forward
;  w -h: iAddY = ~(w*2) backwards
; -w -h: iAddY = ~(0) backwards
; ~ includes the pitch difference (gap)
; the comment in the above may explain a bit more...

INLINE_PREPARE_ARGS MACRO c_nBytesIn

	movsxd	rbx, iWidth

	; iAddY = iWidth * c_nBytesIn
	; note it can be both negative and positive here
	; we will also modify it later...
	imul	rax, rbx, c_nBytesIn
	mov	iAddY, rax

	mov	rdx, c_nBytesIn

	; check if iWidth is negative
	test	rbx, rbx
	jns	@F

	; iAbsWidth = -iWidth
	; source += ( ( iAbsWidth * c_nBytesIn ) - c_nBytesIn )
	; iAddX = -c_nBytesIn
	neg	rbx
	mov	rax, rbx
	imul	rax, rdx
	add	rsi, rax
	neg	rdx
	add	rsi, rdx
@@:
	mov	iAbsWidth, ebx
	mov	iAddX, rdx

	; compute the pitch
	; iNaturalPitch = iAbsWidth * c_nBytesIn
	imul	rax, rbx, c_nBytesIn

	; check if we have the value of pitch directly specified
	; there are three options:
	; if iPitch is zero, iActualPitch = iNaturalPitch
	; if iPitch is > 0, iActualPitch = iPitch
	; if iPitch is < 0, this should mean an alignment
	movsxd	rbx, iPitch
	test	rbx, rbx
	jz	@F
	jns	@pos
	
	; iAlign = -iPitch
	; iActualPitch = ( ( iNaturalPitch + ( iAlign - 1 ) ) / iAlign ) * iAlign
	neg	rbx
	lea	rax, [rax+rbx-1]
	xor	rdx, rdx
	div	rbx
	mul	rbx
	jmp	@F
@pos:
	mov	rax, rbx
@@:
	mov	iActualPitch, eax

	; iAddY = iActualPitch - iAddY
	mov	rdx, rax
	sub	rax, iAddY
	mov	iAddY, rax

	; check if iHeight is negative
	movsxd	rbx, iHeight
	test	rbx, rbx
	jns	@F

	; iAbsHeight = -iHeight
	; iAddY -= ( iActualPitch * 2 )
	; source += (iActualPitch * iAbsHeight) - iActualPitch
	neg	rbx
	mov	rax, rdx
	shl	rax, 1
	sub	iAddY, rax
	mov	rax, rdx
	imul	rax, rbx
	sub	rax, rdx
	add	rsi, rax
@@:
	mov	iAbsHeight, ebx

ENDM


comment '

if (iWidth < 0)
{
	// point to the last pixel in the scanline
	dest += ( iWidth - 1 ) * c_nBytesIn
	iAddX = -c_nBytesIn
}
else
{
	dest = dest
	iAddX = c_nBytesIn
}

if (iHeight < 0)
{
	dest += ( iHeight - 1 ) * iAbsPitch
	iAddY = -Width * c_nBytesIn * 2
}
else
{
	dest = dest
	iAddY = 0
}

w+h+
  rdi=rdi
  ax=4
  ay=0
w+h-
  rdi=4*width*(height-1)
  ax=4
  ay=-4*width*2
w-h+
  rdi=4*(width-1)
  ax=-4
  ay=4*width*2
w-h-
  rdi=4*width*(height-1)+4*(width-1)
  ax=-4
  ay=0


//
// the code
//

iAddY = -iWidth * c_nBytesIn;

if (iWidth < 0)
{
	iAddX = -c_nBytesIn;
	iAbsWidth = -iWidth;

	// point to the last byte in the scanline
	dest += (c_nBytesIn * (iAbsWidth - 1));
}
else
{
	iAddX = c_nBytesIn;
	iAbsWidth = iWidth;
}

if (iHeight < 0)
{
	// here iAddY will be -2*iAbsPitch if iWidth is positive and 0 if negative
	iAddY -= iAbsWidth * c_nBytesIn;
	iAbsHeight = -iHeight;

	// point to the last scanline
	dest += ((c_nBytesIn * iAbsWidth) * (iAbsHeight - 1));
}
else
{
	// +2*iAbsPitch on the width is negative and 0 if positive
	iAddY += iAbsWidth * c_nBytesIn;
	iAbsHeight = iHeight;
}

'

; the special 'bits' version, TODO: make this version the base one (as it's seemingly newer than the above...)
; this variant is also better since it doesn't include the gap calculation, so the function is simplier in overall
INLINE_PREPARE_ARGS_BITS MACRO c_nBytesOut:REQ

	movsxd	rbx, iWidth
	mov	rdx, c_nBytesOut

	; iAddY = -iWidth * c_nBytesOut
	; we will modify it later also
	mov	rax, rbx
	imul	rax, rdx
	neg	rax
	mov	iAddY, rax

	; check if iWidth is negative
	test	rbx, rbx
	jns	@F

	; iAbsWidth = -iWidth
	; dest += (c_nBytesOut * (iAbsWidth - 1)) -- point to the last byte in the scanline
	; iAddX = -c_nBytesOut
	neg	rbx
	mov	rax, rbx
	sub	rax, 1
	imul	rax, rdx
	add	rdi, rax
	neg	rdx
@@:
	mov	iAbsWidth, ebx
	mov	iAddX, rdx

	; calculate the pitch
	; pitch = iAbsWidth * c_nBytesOut
	mov	rdx, rbx
	mov	rax, c_nBytesOut
	imul	rdx, rax

	; HACK: we add it now and then sub twice
	; iAddY += pitch
	add	iAddY, rdx

	movsxd	rbx, iHeight

	; check if iHeight is negative
	test	rbx, rbx
	jns	@F

	; iAbsHeight = -iHeight
	neg	rbx

	; iAddY -= pitch (sub twice to complement the HACK in the above)
	sub	iAddY, rdx
	sub	iAddY, rdx

	; dest += (pitch * (iAbsHeight - 1))
	mov	rax, rbx		
	sub	rax, 1
	imul	rax, rdx
	add	rdi, rax
@@:
	mov	iAbsHeight, ebx

ENDM


; this proc accepts only BPP >= 8 pixel formats, for < than 8 use decode bits
TEMPLATE_PROC_DECODE_RGB MACRO c_aFuncName:REQ, c_nBytesIn:REQ, c_nBytesOut:REQ, c_fnDo:REQ, c_usesEBX
 APIENTRY PLIB_Decode&c_aFuncName, USES rsi rdi ebx, dst:PTR BYTE, src:PTR BYTE, iWidth:DWORD, iHeight:DWORD, iPitch:DWORD, pPal:PTR BYTE

	%LOCAL	iAddY		:PTR BYTE	; flat offset change per Y step (may be negative)
	%LOCAL	iAddX		:PTR BYTE	; same for the X step
	%LOCAL	iAbsWidth	:DWORD	; abs(iWidth)
	%LOCAL	iAbsHeight	:DWORD	; abs(iHeight)
	%LOCAL	iActualPitch	:DWORD	; the actual pitch value (including the alignment if any)
	%LOCAL	iY		:DWORD	; Y pos (the X is stored in ebp), used only if a palette is in use (to deal with the register shortage)

	;int	3

	; compute the granularities if we should
	mov	eax, iWidth
	test	eax, eax
	jnz	@F
	mov	eax, 1
	ret
@@:
	mov	eax, iHeight
	test	eax, eax
	jnz	@F
	mov	eax, 1
	ret
@@:
	; init the source and dest
	mov	rdi, dst
	mov	rsi, src

	; do all the complex calculations of the x,y deltas
	INLINE_PREPARE_ARGS c_nBytesIn

	; check if we should just get the input size
	mov	rax, src
	test	rax, rax
	jz	getsize

	; assume the high-word rax/eax is zero
	xor	rax, rax
	xor	rdx, rdx

	; begin the loop

	; init the height
	; we store pPal in ebx if the palette is not blank
 IFNB <c_usesEBX>
	mov	eax, iAbsHeight
	mov	iY, eax
 ELSE
	mov	ebx, iAbsHeight
 ENDIF

	; get some space under rsp
	sub	rsp, 16
@Y:
	; init the width
	mov	ecx, iAbsWidth
@X:

	; do the function
	;----------
	%c_fnDo
	;----------


	; offset to the next byte (may be negative)
	add	rdi, c_nBytesOut
	add	rsi, iAddX		; XXX: should we change the code to deal with the dest insted?
	sub	ecx, 1
	jnz	@X
@@:
	; offset to the next scanline (may be negative)
	add	rsi, iAddY

 IFNB <c_usesEBX>
	sub	iY, 1
 ELSE
	sub	ebx, 1
 ENDIF

	jnz	@Y
@@:
	; restore the stack
	add	rsp, 16

getsize:

	; total size written
	movzxd	rax, iActualPitch
	movzxd	rcx, iAbsHeight
	imul	rax, rcx
	ret

@error:

	xor	rax, rax
	ret

 ENDENTRY PLIB_Decode&c_aFuncName
ENDM


; the 'bits' decoder (less than 1 byte)
; if c_order is 1 the most significant bit comes first (like in the Microsoft's BMP format)
TEMPLATE_PROC_DECODE_BITS MACRO c_aFuncName:REQ, c_nBitsIn:REQ, c_mask:REQ, c_nBytesOut:REQ, c_order:REQ, c_palette
 APIENTRY PLIB_Decode&c_aFuncName, USES rsi rdi ebx, dst:PTR BYTE, src:PTR BYTE, iWidth:DWORD, iHeight:DWORD, iPitch:DWORD, pPal:PTR BYTE

	%LOCAL	iAddY		:PTR BYTE	; see the above macro for parameter definitions
	%LOCAL	iAddX		:PTR BYTE
	%LOCAL	iSrcAddY	:PTR BYTE	; the gap if we have any
	%LOCAL	iAbsWidth	:DWORD
	%LOCAL	iAbsHeight	:DWORD
	%LOCAL	iActualPitch	:DWORD
	%LOCAL	iY		:DWORD

	; compute the granularities if we should
	mov	eax, iWidth
	test	eax, eax
	jnz	@F
	mov	eax, (8 / c_nBitsIn)
	ret
@@:
	mov	eax, iHeight
	test	eax, eax
	jnz	@F
	mov	eax, 1
	ret
@@:
	; source and dest
	mov	rdi, dst
	mov	rsi, src

	; do all the complex calculations of the x,y deltas
	INLINE_PREPARE_ARGS_BITS c_nBytesOut

	; just get the input size?
	mov	rax, src
	test	rax, rax
	jz	@getsize

	; iNaturalPitch = ((iAbsWidth * nBits) + 7) / 8;
	mov	edx, iAbsWidth		
	imul	eax, edx, c_nBitsIn
	add	eax, 7
	shr	eax, 3

	; check if we have the pitch value directly specified
	movsxd	rbx, iPitch
	test	rbx, rbx
	jz	@F

	; XXX: negative pitch?
	js	@error

	; calculate the gap amount
	; iSrcAddY = iPitch - iNaturalPitch
	; iActualPitch = iPitch
	mov	edx, eax
	mov	eax, ebx
	sub	ebx, edx	; may get broken here if iPitch > iNaturalPitch
	js	@error
@@:
	mov	iActualPitch, eax
	mov	iSrcAddY, rbx

	; begin the loop here
	; init height
	mov	eax, iAbsHeight
	mov	iY, eax
@Y:
	; init width
	mov	ebx, iAbsWidth
	mov	ecx, 1
@X:
	; should we read the next byte now?
	sub	ecx, 1
	jnz	@F

	; read the byte
	mov	dl, BYTE PTR [rsi]

	; the amount of values in this byte
	mov	ecx, 8 / c_nBitsIn		; 8 for 1; 4 for 2; 2 for 4

	add	rsi, 1
@@:
	; shift to the right place within the byte and extract the value

 IF (c_order EQ 1)
	; rotate left since we start reading from the MSB (see the macro notes)
	rol	dl, c_nBitsIn	
 ENDIF

	xor	eax, eax
	mov	al, dl

 IF (c_order EQ 0)
	shl	dl, c_nBitsIn
 ENDIF

	and	al, c_mask

	; TODO: may be play with expand (unquantize) better?
 IFNB <c_palette>
	add	rax, c_palette
	mov	al, BYTE PTR [rax]
 ENDIF

	; copy the byte to output
	mov	BYTE PTR [rdi], al

	; offset to the next byte (may be negative)
	add	rdi, iAddX
	sub	ebx, 1		; ebx is iX here
	jnz	@X

	; offset to the next scanline (may be negative)
	add	rdi, iAddY
	add	rsi, iSrcAddY	; fill the gap if any
	sub	iY, 1
	jnz	@Y

@getsize:

	; return total size written
	movzxd	rax, iActualPitch
	movzxd	rcx, iAbsHeight
	imul	rax, rcx
	ret

@error:

	xor	rax, rax
	ret

 ENDENTRY PLIB_Decode&c_aFuncName
ENDM



;//////////////////////////////////////////////////////////////////////


; 'swizzle' rgb formats
TEMPLATE_PROC_DECODE_RGB R8,                      1, 1, <INLINE_DECODE_R8>
TEMPLATE_PROC_DECODE_RGB R8G8,                    2, 2, <INLINE_DECODE_R8G8 0>
TEMPLATE_PROC_DECODE_RGB G8R8,                    2, 2, <INLINE_DECODE_R8G8 1>
TEMPLATE_PROC_DECODE_RGB R8G8B8,                  3, 3, <INLINE_DECODE_R8G8B8 0, 0>
TEMPLATE_PROC_DECODE_RGB B8G8R8,                  3, 3, <INLINE_DECODE_R8G8B8 1, 0>
TEMPLATE_PROC_DECODE_RGB R8G8B8X8,                4, 3, <INLINE_DECODE_R8G8B8X8 0, 1>
TEMPLATE_PROC_DECODE_RGB B8G8R8X8,                4, 3, <INLINE_DECODE_R8G8B8X8 1, 1>
TEMPLATE_PROC_DECODE_RGB X8X8X8A8,                4, 1, <INLINE_DECODE_X8X8X8A8>
TEMPLATE_PROC_DECODE_RGB R8G8B8A8,                4, 4, <INLINE_DECODE_R8G8B8A8 0>
TEMPLATE_PROC_DECODE_RGB B8G8R8A8,                4, 4, <INLINE_DECODE_R8G8B8A8 1>
TEMPLATE_PROC_DECODE_RGB R16,                     2, 2, <INLINE_DECODE_R16>
TEMPLATE_PROC_DECODE_RGB R16G16,                  4, 4, <INLINE_DECODE_R16G16 0>
TEMPLATE_PROC_DECODE_RGB G16R16,                  4, 4, <INLINE_DECODE_R16G16 1>
TEMPLATE_PROC_DECODE_RGB R16G16B16,               6, 6, <INLINE_DECODE_R16G16B16 0>
TEMPLATE_PROC_DECODE_RGB B16G16R16,               6, 6, <INLINE_DECODE_R16G16B16 1>
TEMPLATE_PROC_DECODE_RGB R16G16B16X16,            8, 6, <INLINE_DECODE_R16G16B16X16 0>
TEMPLATE_PROC_DECODE_RGB B16G16R16X16,            8, 6, <INLINE_DECODE_R16G16B16X16 1>
TEMPLATE_PROC_DECODE_RGB X16X16X16A16,            8, 2, <INLINE_DECODE_X16X16X16A16>
TEMPLATE_PROC_DECODE_RGB R16G16B16A16,            8, 8, <INLINE_DECODE_R16G16B16A16 0>
TEMPLATE_PROC_DECODE_RGB B16G16R16A16,            8, 8, <INLINE_DECODE_R16G16B16A16 1>
TEMPLATE_PROC_DECODE_RGB R32,                     4, 4, <INLINE_DECODE_R32>
TEMPLATE_PROC_DECODE_RGB R32G32,                  8, 8, <INLINE_DECODE_R32G32 0>
TEMPLATE_PROC_DECODE_RGB G32R32,                  8, 8, <INLINE_DECODE_R32G32 1>
TEMPLATE_PROC_DECODE_RGB R32G32B32,               12,12, <INLINE_DECODE_R32G32B32 0>
TEMPLATE_PROC_DECODE_RGB B32G32R32,               12,12, <INLINE_DECODE_R32G32B32 1>
TEMPLATE_PROC_DECODE_RGB R32G32B32X32,            16,12, <INLINE_DECODE_R32G32B32X32 0>
TEMPLATE_PROC_DECODE_RGB B32G32R32X32,            16,12, <INLINE_DECODE_R32G32B32X32 1>
TEMPLATE_PROC_DECODE_RGB X32X32X32A32,            16, 4, <INLINE_DECODE_X32X32X32A32>
TEMPLATE_PROC_DECODE_RGB R32G32B32A32,            16,16, <INLINE_DECODE_R32G32B32A32 0>
TEMPLATE_PROC_DECODE_RGB B32G32R32A32,            16,16, <INLINE_DECODE_R32G32B32A32 1>

; specials
TEMPLATE_PROC_DECODE_RGB R8G8B8_UNORM,            3, 4, <INLINE_DECODE_R8G8B8 1, 1>
TEMPLATE_PROC_DECODE_RGB B8G8R8_UNORM,            3, 4, <INLINE_DECODE_R8G8B8 0, 1>
TEMPLATE_PROC_DECODE_RGB R8G8B8X8_UNORM,          4, 4, <INLINE_DECODE_R8G8B8X8 1, 0>
TEMPLATE_PROC_DECODE_RGB B8G8R8X8_UNORM,          4, 4, <INLINE_DECODE_R8G8B8X8 0, 0>
TEMPLATE_PROC_DECODE_RGB R8G8B8A8_UNORM,          4, 4, <INLINE_DECODE_R8G8B8A8 1>
TEMPLATE_PROC_DECODE_RGB B8G8R8A8_UNORM,          4, 4, <INLINE_DECODE_R8G8B8A8 0>

; packed integer formats
TEMPLATE_PROC_DECODE_RGB R3G3B2_UNORM,            1, 4, <INLINE_DECODE_R3G3B2 CH_2, CH_1, CH_0>
TEMPLATE_PROC_DECODE_RGB B2G3R3_UNORM,            1, 4, <INLINE_DECODE_B2G3R3 CH_0, CH_1, CH_2>
TEMPLATE_PROC_DECODE_RGB R3G3B2A8_UNORM,          2, 4, <INLINE_DECODE_R3G3B2A8 CH_2, CH_1, CH_0>
TEMPLATE_PROC_DECODE_RGB B2G3R3A8_UNORM,          2, 4, <INLINE_DECODE_B2G3R3A8 CH_0, CH_1, CH_2>
TEMPLATE_PROC_DECODE_RGB R4G4B4X4_UNORM,          2, 4, <INLINE_DECODE_R4G4B4X4 CH_2, CH_1, CH_0>
TEMPLATE_PROC_DECODE_RGB B4G4R4X4_UNORM,          2, 4, <INLINE_DECODE_R4G4B4X4 CH_0, CH_1, CH_2>
TEMPLATE_PROC_DECODE_RGB R4G4B4A4_UNORM,          2, 4, <INLINE_DECODE_R4G4B4A4 CH_2, CH_1, CH_0>
TEMPLATE_PROC_DECODE_RGB B4G4R4A4_UNORM,          2, 4, <INLINE_DECODE_R4G4B4A4 CH_0, CH_1, CH_2>
TEMPLATE_PROC_DECODE_RGB R5G6B5_UNORM,            2, 4, <INLINE_DECODE_R5G6B5 CH_2, CH_1, CH_0>
TEMPLATE_PROC_DECODE_RGB B5G6R5_UNORM,            2, 4, <INLINE_DECODE_R5G6B5 CH_0, CH_1, CH_2>
TEMPLATE_PROC_DECODE_RGB R5G5B5X1_UNORM,          2, 4, <INLINE_DECODE_R5G5B5X1 CH_2, CH_1, CH_0>
TEMPLATE_PROC_DECODE_RGB B5G5R5X1_UNORM,          2, 4, <INLINE_DECODE_R5G5B5X1 CH_0, CH_1, CH_2>
TEMPLATE_PROC_DECODE_RGB R5G5B5A1_UNORM,          2, 4, <INLINE_DECODE_R5G5B5A1 CH_2, CH_1, CH_0>
TEMPLATE_PROC_DECODE_RGB B5G5R5A1_UNORM,          2, 4, <INLINE_DECODE_R5G5B5A1 CH_0, CH_1, CH_2>

; c_bUsesEBX is a nasty hack to deal with the limited amount of registers...
; the packed normalized formats that are > 8 bits per component are all using it
TEMPLATE_PROC_DECODE_RGB R10G10B10X2_UNORM,       4, 6, <INLINE_DECODE_R10G10B10X2 CH_0, CH_1, CH_2>, 1
TEMPLATE_PROC_DECODE_RGB B10G10R10X2_UNORM,       4, 6, <INLINE_DECODE_R10G10B10X2 CH_2, CH_1, CH_0>, 1
TEMPLATE_PROC_DECODE_RGB X10X10X10A2_UNORM,       4, 1, <INLINE_DECODE_X10X10X10A2>
TEMPLATE_PROC_DECODE_RGB R10G10B10A2_UNORM,       4, 8, <INLINE_DECODE_R10G10B10A2 CH_0, CH_1, CH_2>, 1
TEMPLATE_PROC_DECODE_RGB B10G10R10A2_UNORM,       4, 8, <INLINE_DECODE_R10G10B10A2 CH_2, CH_1, CH_0>, 1

; packed signed norm as it happened...
TEMPLATE_PROC_DECODE_RGB U10V10W10X2,             4, 6, <INLINE_DECODE_R10G10B10X2_SNORM CH_0, CH_1, CH_2>, 1
TEMPLATE_PROC_DECODE_RGB W10V10U10X2,             4, 6, <INLINE_DECODE_R10G10B10X2_SNORM CH_2, CH_1, CH_0>, 1

TEMPLATE_PROC_DECODE_RGB R10G10B10A2_UINT,        4, 8, <INLINE_DECODE_R10G10B10A2_UINT CH_0, CH_1, CH_2>

; packed fixed point
TEMPLATE_PROC_DECODE_RGB R10G10B10X2_XR_BIAS,     4, 6, <INLINE_DECODE_R10G10B10X2_XR_BIAS CH_0, CH_1, CH_2>

; packed float formats
TEMPLATE_PROC_DECODE_RGB R9G9B9E5_FLOAT,          4,12, <INLINE_DECODE_R9G9B9E5_FLOAT CH_0, CH_1, CH_2>
TEMPLATE_PROC_DECODE_RGB R11G11B10_FLOAT,         4, 6, <INLINE_DECODE_R11G11B10_FLOAT CH_0, CH_1, CH_2>
TEMPLATE_PROC_DECODE_RGB B10G11R11_FLOAT,         4, 6, <INLINE_DECODE_B10G11R11_FLOAT CH_2, CH_1, CH_0>

; packed luminance and palette formats
TEMPLATE_PROC_DECODE_RGB L4A4,                    1, 2, <INLINE_DECODE_L4A4>
TEMPLATE_PROC_DECODE_RGB P4X4,                    1, 1, <INLINE_DECODE_P4X4>
TEMPLATE_PROC_DECODE_RGB X4A4,                    1, 1, <INLINE_DECODE_X4A4>
;;TEMPLATE_PROC_DECODE_RGB L6A2,                  1, 2, <INLINE_DECODE_L6A2>
TEMPLATE_PROC_DECODE_RGB L8A8,                    2, 2, <INLINE_DECODE_L8A8>
TEMPLATE_PROC_DECODE_RGB P8X8,                    2, 1, <INLINE_DECODE_P8X8>
TEMPLATE_PROC_DECODE_RGB X8A8,                    2, 1, <INLINE_DECODE_X8A8>
;;TEMPLATE_PROC_DECODE_RGB L12A4,                 2, 4, <INLINE_DECODE_L12A4>
TEMPLATE_PROC_DECODE_RGB L16A16,                  4, 4, <INLINE_DECODE_L16A16>

; single palette/luminance/alpha
TEMPLATE_PROC_DECODE_RGB L8,                      1, 1, <INLINE_DECODE_R8>
TEMPLATE_PROC_DECODE_RGB L16,                     2, 2, <INLINE_DECODE_R16>
TEMPLATE_PROC_DECODE_RGB L32,                     4, 4, <INLINE_DECODE_R32>
TEMPLATE_PROC_DECODE_RGB P8,                      1, 1, <INLINE_DECODE_R8>
TEMPLATE_PROC_DECODE_RGB P16,                     2, 2, <INLINE_DECODE_R16>
TEMPLATE_PROC_DECODE_RGB P32,                     4, 4, <INLINE_DECODE_R32>
TEMPLATE_PROC_DECODE_RGB A8,                      1, 1, <INLINE_DECODE_R8>
TEMPLATE_PROC_DECODE_RGB A16,                     2, 2, <INLINE_DECODE_R16>
TEMPLATE_PROC_DECODE_RGB A32,                     4, 4, <INLINE_DECODE_R32>

; packed bump dudv formats
TEMPLATE_PROC_DECODE_RGB U8V8CX,                  2, 2, <INLINE_DECODE_U8V8CX>
TEMPLATE_PROC_DECODE_RGB U5V5X6,                  2, 2, <INLINE_DECODE_U5V5X6>
TEMPLATE_PROC_DECODE_RGB X5X5L6,                  2, 1, <INLINE_DECODE_X5X5L6>
TEMPLATE_PROC_DECODE_RGB U8V8X8X8,                4, 2, <INLINE_DECODE_U8V8X8X8>
TEMPLATE_PROC_DECODE_RGB X8X8L8X8,                4, 1, <INLINE_DECODE_X8X8L8X8>

;yuv
;;TEMPLATE_PROC_DECODE_RGB AUYV,                    2, 3, <INLINE_DECODE_AUYV>

; depth/stencil
TEMPLATE_PROC_DECODE_RGB D24X8,                   4, 4, <INLINE_DECODE_D24X8>
TEMPLATE_PROC_DECODE_RGB X24S8,                   4, 1, <INLINE_DECODE_X24S8>
TEMPLATE_PROC_DECODE_RGB D15X1,                   2, 2, <INLINE_DECODE_D15X1>
TEMPLATE_PROC_DECODE_RGB X15S1,                   2, 1, <INLINE_DECODE_X15S1>
TEMPLATE_PROC_DECODE_RGB D32X8X24,                8, 4, <INLINE_DECODE_D32X8X24>
TEMPLATE_PROC_DECODE_RGB X32S8X24,                8, 1, <INLINE_DECODE_X32S8X24>




; special cases: bits decoding (less than 1 byte)

; luminance bits			     in(bits), mask, outbytes, msborder, <lut>
TEMPLATE_PROC_DECODE_BITS L1M,               1, 01h, 1, 1, OFFSET g_color8_2
TEMPLATE_PROC_DECODE_BITS L2M,               2, 03h, 1, 1, OFFSET g_color8_4
TEMPLATE_PROC_DECODE_BITS L4M,               4, 0Fh, 1, 1, OFFSET g_color8_16
TEMPLATE_PROC_DECODE_BITS L1L,               1, 01h, 1, 0, OFFSET g_color8_2
TEMPLATE_PROC_DECODE_BITS L2L,               2, 03h, 1, 0, OFFSET g_color8_4
TEMPLATE_PROC_DECODE_BITS L4L,               4, 0Fh, 1, 0, OFFSET g_color8_16

; palette bits
TEMPLATE_PROC_DECODE_BITS P1M,               1, 01h, 1, 1
TEMPLATE_PROC_DECODE_BITS P2M,               2, 03h, 1, 1
TEMPLATE_PROC_DECODE_BITS P4M,               4, 0Fh, 1, 1
TEMPLATE_PROC_DECODE_BITS P1L,               1, 01h, 1, 0
TEMPLATE_PROC_DECODE_BITS P2L,               2, 03h, 1, 0
TEMPLATE_PROC_DECODE_BITS P4L,               4, 0Fh, 1, 0


;//////////////////////////////////////////////////////////////////////

end
