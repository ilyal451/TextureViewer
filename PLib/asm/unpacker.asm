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

INCLUDE compile.inc

;//////////////////////////////////////////////////////////////////////

INCLUDE defs.inc

PLIB_UnpackHalfFloat PROTODEF flParam:WORD
PUBLIC PLIB_UnpackHalfFloat

PUBLIC PLIB_ConvertOutputFloat
PUBLIC PLIB_EncodeGamma
PUBLIC PLIB_DecodeGamma
PUBLIC PLIB_ConvertGamma

;//////////////////////////////////////////////////////////////////////

.const

IFDEF _SSE
align 16
g_psZero		dd 0.0, 0.0, 0.0, 0.0
g_psOne			dd 1.0, 1.0, 1.0, 1.0
g_psDef			dd 0.0, 0.0, 0.0, 1.0
g_ps255			dd 255.0, 255.0, 255.0, 255.0
g_ps255_999		dd 437FFFFFH, 437FFFFFH, 437FFFFFH, 437FFFFFH		; used with truncate rounding (1.0 will give about 255.9999)
g_psOne_255		dd 3B808081H, 3B808081H, 3B808081H, 3B808081H		; 1 / 255.0
g_psOne_65535		dd 37800080H, 37800080H, 37800080H, 37800080H		; 1 / 65535.0
g_psOne_16777215	dd 33800001H, 33800001H, 33800001H, 33800001H		; 1 / 16777215.0
g_psOne_127		dd 3C010204H, 3C010204H, 3C010204H, 3C010204H
g_psOne_32767		dd 38000100H, 38000100H, 38000100H, 38000100H
g_psOne_8388607		dd 34000001H, 34000001H, 34000001H, 34000001H
g_ps_minus128i		dd 0FFFFFF80H, 0FFFFFF80H, 0FFFFFF80H, 0FFFFFF80H
g_ps_minus127i		dd 0FFFFFF81H, 0FFFFFF81H, 0FFFFFF81H, 0FFFFFF81H
g_ps_minus32768i	dd 0FFFF8000H, 0FFFF8000H, 0FFFF8000H, 0FFFF8000H
g_ps_minus32767i	dd 0FFFF8001H, 0FFFF8001H, 0FFFF8001H, 0FFFF8001H
g_ps_minus8388608i	dd 0FF800000H, 0FF800000H, 0FF800000H, 0FF800000H
g_ps_minus8388607i	dd 0FF800001H, 0FF800001H, 0FF800001H, 0FF800001H
g_psOverlayMask_R	dd 0FFFFFFFFH, 000000000H, 000000000H, 000000000H
g_psOverlayMask_R_I	dd 000000000H, 0FFFFFFFFH, 0FFFFFFFFH, 0FFFFFFFFH
g_psOverlayMask_G	dd 000000000H, 0FFFFFFFFH, 000000000H, 000000000H
g_psOverlayMask_G_I	dd 0FFFFFFFFH, 000000000H, 0FFFFFFFFH, 0FFFFFFFFH
g_psOverlayMask_B	dd 000000000H, 000000000H, 0FFFFFFFFH, 000000000H
g_psOverlayMask_B_I	dd 0FFFFFFFFH, 0FFFFFFFFH, 000000000H, 0FFFFFFFFH
g_psOverlayMask_A	dd 000000000H, 000000000H, 000000000H, 0FFFFFFFFH
g_psOverlayMask_A_I	dd 0FFFFFFFFH, 0FFFFFFFFH, 0FFFFFFFFH, 000000000H
g_psOverlayMask_RG	dd 0FFFFFFFFH, 0FFFFFFFFH, 000000000H, 000000000H
g_psOverlayMask_RG_I	dd 000000000H, 000000000H, 0FFFFFFFFH, 0FFFFFFFFH
g_psOverlayMask_RGB	dd 0FFFFFFFFH, 0FFFFFFFFH, 0FFFFFFFFH, 000000000H
g_psOverlayMask_RGB_I	dd 000000000H, 000000000H, 000000000H, 0FFFFFFFFH
g_psNOT			dd 0FFFFFFFFH, 0FFFFFFFFH, 0FFFFFFFFH, 0FFFFFFFFH	; used in conjunction with xorps
g_psKeepSign		dd 80000000H, 80000000H, 80000000H, 80000000H
g_psClearSign		dd 7FFFFFFFH, 7FFFFFFFH, 7FFFFFFFH, 7FFFFFFFH
g_psKeepAlpha		dd 0, 0, 0, 0FFFFFFFFH
g_psClearAlpha		dd 0FFFFFFFFH, 0FFFFFFFFH, 0FFFFFFFFH, 0
; very specific constants
g_psLOG2_MaskAND	dd 007FFFFFH, 007FFFFFH, 007FFFFFH, 007FFFFFH
g_psLOG2_MaskOR		dd 3F000000H, 3F000000H, 3F000000H, 3F000000H
g_psLOG2_1_192E7	dd 1.1920928955078125e-7, 1.1920928955078125e-7, 1.1920928955078125e-7, 1.1920928955078125e-7
g_psLOG2_124_225	dd 124.22551499, 124.22551499, 124.22551499, 124.22551499
g_psLOG2_1_498		dd 1.498030302, 1.498030302, 1.498030302, 1.498030302
g_psLOG2_1_725		dd 1.72587999, 1.72587999, 1.72587999, 1.72587999
g_psLOG2_0_352		dd 0.3520887068, 0.3520887068, 0.3520887068, 0.3520887068
g_psPOW2_s126max	dd -126.0, -126.0, -126.0, -126.0
g_psPOW2_121_274	dd 121.2740575, 121.2740575, 121.2740575, 121.2740575
g_psPOW2_27_728		dd 27.7280233, 27.7280233, 27.7280233, 27.7280233
g_psPOW2_4_842		dd 4.84252568, 4.84252568, 4.84252568, 4.84252568
g_psPOW2_1_490		dd 1.49012907, 1.49012907, 1.49012907, 1.49012907
g_psPOW2_1shl23		dd 8388608.0, 8388608.0, 8388608.0, 8388608.0
g_psSRGB_EncGamma	dd 0.41666666666666667, 0.41666666666666667, 0.41666666666666667, 0.41666666666666667	; 1/2.4
g_psSRGB_EncDelta	dd -0.055, -0.055, -0.055, -0.055
g_psSRGB_EncDeltaMul	dd 1.055, 1.055, 1.055, 1.055
g_psSRGB_EncRamp	dd 0.0031308, 0.0031308, 0.0031308, 0.0031308
g_psSRGB_EncRampMul	dd 12.92, 12.92, 12.92, 12.92
g_psSRGB_DecGamma	dd 2.4, 2.4, 2.4, 2.4
g_psSRGB_DecDelta	dd 0.055, 0.055, 0.055, 0.055
g_psSRGB_DecDeltaMul	dd 0.947867298578199, 0.947867298578199, 0.947867298578199, 0.947867298578199	; 1/1.055
g_psSRGB_DecRamp	dd 0.04045, 0.04045, 0.04045, 0.04045
g_psSRGB_DecRampMul	dd 0.07739938080495356, 0.07739938080495356, 0.07739938080495356, 0.07739938080495356	; 1/12.92
ENDIF

;IFNDEF _SSE
align 8
; fpu constants
; XXX: may use dt here
; TODO: requires more work to check the constants (eg the signed ones)
g_fl255			dq 255.0
g_fl255_999		dq 406FFFFFFFFFFFFFH
g_flOne_255		dq 3f70101010101010H	; 1.0 / 255.0
g_flOne_65535		dq 3ef0001000100010H	; 1.0 / 65535.0
g_flOne_16777215	dq 3E70000010000010H	; 1.0 / 16777215.0
g_flOne_127		dq 3F80204081020408H	; 1.0 / 127
g_flOne_32767		dq 3F00002000400080H	; 1.0 / 32767
g_flOne_8388607		dq 3E80000020000040H	; 1.0 / 8388607
g_flSRGB_EncGamma	dq 0.41666666666666667
g_flSRGB_EncDelta	dq -0.055
g_flSRGB_EncDeltaMul	dq 1.055
g_flSRGB_EncRamp	dq 0.0031308
g_flSRGB_EncRampMul	dq 12.92
g_flSRGB_DecGamma	dq 2.4
g_flSRGB_DecDelta	dq 0.055
g_flSRGB_DecDeltaMul	dq 0.947867298578199
g_flSRGB_DecRamp	dq 0.04045
g_flSRGB_DecRampMul	dq 0.07739938080495356
;ENDIF

align 4
g_c2			dd 2


;//////////////////////////////////////////////////////////////////////

.code

HF_SIGN equ 8000H
HF_EXP equ 7C00H
HF_MAN equ 03FFH

; TODO: needs thorough testing
; returns the result in eax
INLINE_UNPACK_HALF_FLOAT MACRO _src
LOCAL @denormzero, @zero, @denorm, @error, @infnan, @quit

	movzx	edx, WORD PTR _src
	mov	eax, edx

	and	eax, HF_EXP
	jz	@denormzero

	cmp	eax, HF_EXP
	je	@infnan

	; convert the exponent
	add	eax, 1C000H
	shl	eax, 14		; includes the space for the sign

	; get the sign
	test	edx, HF_SIGN
	setnz	al
	ror	eax, 1		; will make the sign to the front

	; get the mantissa
	and	edx, HF_MAN
	shl	edx, 13
	or	eax, edx
	jmp	@quit

@denormzero:
	mov	eax, edx
	test	eax, HF_MAN
	jnz	@denorm

@zero:
	; preserve the sign
	or	eax, HF_SIGN
	shl	eax, 16
	jmp	@quit

@denorm:
	; TODO: convert denorms to zero?
	and	edx, HF_SIGN
	shr	edx, 7		; put the sign (if any) before dl
@@:
	shl	eax, 1
	jz	@error		; the register is zero for some reason... (should never happen)
	add	dl, 1
	test	eax, 400H
	jz	@B
@@:
	; store the mantissa
	and	eax, HF_MAN
	shl	eax, 13

	; store the exponent
	neg	dl
	add	dl, 127 - 15
	js	@error		; shoudn't wrap
	shl	edx, 23		; also contains the sign bit
	or	eax, edx
	jmp	@quit

; NOTE: remove this as it's been tested
@error:
	int	3
	jmp	@quit

@infnan:
	mov	eax, edx
	and	eax, HF_SIGN
	shl	eax, 1
	mov	ah, -1
	shl	eax, 15

	and	edx, HF_MAN
	shl	edx, 13
	or	eax, edx

@quit:
	;mov	DWORD PTR [rsp], eax
	;fld	DWORD PTR [rsp]

ENDM


OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

; this proc is for the use in other modules
; XXX: consider using it in this module too, as it's quite lengthy to be inlined
; TODO: x64 calling syntax required
IFNDEF _X64
 APIENTRY PLIB_UnpackHalfFloat, flParam:WORD

	lea	rdx, flParam

	push	rax	; alloc some space

	INLINE_UNPACK_HALF_FLOAT [rdx]

	mov	DWORD PTR [rsp], eax
	fld	DWORD PTR [rsp]

	pop	rax

	ret

 ENDENTRY PLIB_UnpackHalfFloat
ENDIF

OPTION PROLOGUE:PROLOGUEDEF
OPTION EPILOGUE:EPILOGUEDEF


;//////////////////////////////////////////////////////////////////////


; clamp the value in eax to the range 0...1
INLINE_CLAMP_UNSIGNED_1 MACRO
LOCAL @skip

	test	eax, eax
	jz	@skip
	mov	eax, 1
@skip:

ENDM

; clamp the value in eax to the range 0...1 (signed version)
INLINE_CLAMP_SIGNED_1 MACRO
LOCAL @sign, @skip

	test	eax, eax
	js	@sign
	jz	@skip
	mov	eax, 1
	jmp	@skip
@sign:
	xor	eax, eax
@skip:

ENDM

; clamp the value in eax to the range 0...255 (signed version)
INLINE_CLAMP_SIGNED_255 MACRO
LOCAL @above, @sign, @neat

	test	eax, eax
	js	@sign
	cmp	eax, 255
	ja	@above
	jmp	@neat
@sign:
	xor	eax, eax
	jmp	@neat
@above:
	mov	eax, 255
@neat:

ENDM

; clip the value in al from the bottom
INLINE_CLIP_AL_SIGNED MACRO
LOCAL @sign, @neat

	test	al, al
	js	@sign
	shl	al, 1
	mov	dl, al
	shr	dl, 7
	or	al, dl
	jmp	@neat
@sign:
	xor	al, al
@neat:

ENDM


; this macro returns the result in al
; the other bits in rax/eax are assumed to be undefined
; the DF_FLOAT version expects 255.0 on the top of the fpu stack
; the DF_FLOAT version assumes there's some space on the stack at rsp/esp (1 DWORD)
INLINE_UNPACK_COMPONENT MACRO _src:REQ, _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ

 IF ((_eFmt EQ DF_UNORM) OR ((_eFmt EQ DF_UINT) AND (_bIntAsNorm EQ 1)))
  IF (_iSize EQ CS_8BIT)
	movzx	eax, BYTE PTR _src
  ELSEIF (_iSize EQ CS_16BIT)
	movzx	eax, BYTE PTR _src+1
  ELSEIF (_iSize EQ CS_32BIT)
	movzx	eax, BYTE PTR _src+3
  ENDIF
 ELSEIF ((_eFmt EQ DF_SNORM) OR ((_eFmt EQ DF_SINT) AND (_bIntAsNorm EQ 1)))
  IF (_bSignedRange EQ 1)
   IF (_iSize EQ CS_8BIT)
	movzx	eax, BYTE PTR _src
	add	al, 80H
   ELSEIF (_iSize EQ CS_16BIT)
	movzx	eax, BYTE PTR _src+1
	add	al, 80H
   ELSEIF (_iSize EQ CS_32BIT)
	movzx	eax, BYTE PTR _src+3
	add	al, 80H
   ENDIF
  ELSE
   IF (_iSize EQ CS_8BIT)
	movzx	eax, BYTE PTR _src
	INLINE_CLIP_AL_SIGNED
   ELSEIF (_iSize EQ CS_16BIT)
	movzx	eax, BYTE PTR _src+1
	INLINE_CLIP_AL_SIGNED
   ELSEIF (_iSize EQ CS_32BIT)
	movzx	eax, BYTE PTR _src+3
	INLINE_CLIP_AL_SIGNED
   ENDIF
  ENDIF
 ELSEIF (_eFmt EQ DF_UINT)
  IF (_iSize EQ CS_8BIT)
	movzx	eax, BYTE PTR _src
	INLINE_CLAMP_UNSIGNED_1
  ELSEIF (_iSize EQ CS_16BIT)
	movzx	eax, WORD PTR _src
	INLINE_CLAMP_UNSIGNED_1
  ELSEIF (_iSize EQ CS_32BIT)
	mov	eax, DWORD PTR _src
	INLINE_CLAMP_UNSIGNED_1
  ENDIF
 ELSEIF (_eFmt EQ DF_SINT)
  IF (_iSize EQ CS_8BIT)
	movsx	eax, BYTE PTR _src
	INLINE_CLAMP_SIGNED_1
  ELSEIF (_iSize EQ CS_16BIT)
	movsx	eax, WORD PTR _src
	INLINE_CLAMP_SIGNED_1
  ELSEIF (_iSize EQ CS_32BIT)
	mov	eax, DWORD PTR _src
	INLINE_CLAMP_SIGNED_1
  ENDIF
 ELSEIF (_eFmt EQ DF_FLOAT)
  IF (_iSize EQ CS_8BIT)
	; unsupported
	xor	eax, eax
  ELSEIF (_iSize EQ CS_16BIT)
	INLINE_UNPACK_HALF_FLOAT _src ;this is probably too lengthy to be inlined...
	;INVOKE PLIB_UnpackHalfFloat _src
	;movzx	eax, WORD PTR _src
	;push	rax
	;call	PLIB_UnpackHalfFloat
	;pop	rax
	mov	DWORD PTR [rsp], eax
	fld	DWORD PTR [rsp]
	fmul	st(0), st(1)	; 255.0 should be on the fpu stack
	fistp	DWORD PTR [rsp]	; some space should be allocated under rsp/esp
	mov	eax, DWORD PTR [rsp]
	INLINE_CLAMP_SIGNED_255
  ELSEIF (_iSize EQ CS_32BIT)
	fld	DWORD PTR _src
	fmul	st(0), st(1)	; 255.0 should be on the fpu stack
	fistp	DWORD PTR [rsp]	; some space should be allocated under rsp/esp
	mov	eax, DWORD PTR [rsp]
	INLINE_CLAMP_SIGNED_255
  ENDIF
 ENDIF

ENDM


; simply expands the value to 32 bits (sse cvtdq2ps ready)
; returns the value in eax
INLINE_EXPAND_SSE_COMPONENT MACRO _src:REQ, _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ

 IF ((_eFmt EQ DF_UNORM) OR ((_eFmt EQ DF_UINT) AND (_bIntAsNorm EQ 1)))
  IF (_iSize EQ CS_8BIT)
	movzx	eax, BYTE PTR _src
  ELSEIF (_iSize EQ CS_16BIT)
	movzx	eax, WORD PTR _src
  ELSEIF (_iSize EQ CS_32BIT)
	mov	eax, DWORD PTR _src
	shr	eax, 8	; convert to 24 bits as we don't have enough mantissa precision for the full 32-bits anyway
  ENDIF
 ELSEIF ((_eFmt EQ DF_SNORM) OR ((_eFmt EQ DF_SINT) AND (_bIntAsNorm EQ 1)))
  IF (_bSignedRange EQ 1)
   IF (_iSize EQ CS_8BIT)
	movzx	eax, BYTE PTR _src
	add	al, 80H
   ELSEIF (_iSize EQ CS_16BIT)
	movzx	eax, WORD PTR _src
	add	ax, 8000H
   ELSEIF (_iSize EQ CS_32BIT)
	mov	eax, DWORD PTR _src
	add	eax, 80000000H
	shr	eax, 8	; to 24 bits
   ENDIF
  ELSE
   IF (_iSize EQ CS_8BIT)
	movsx	eax, BYTE PTR _src
   ELSEIF (_iSize EQ CS_16BIT)
	movsx	eax, WORD PTR _src
   ELSEIF (_iSize EQ CS_32BIT)
	mov	eax, DWORD PTR _src
	sar	eax, 8	; to 24 bits
   ENDIF
  ENDIF
 ELSEIF (_eFmt EQ DF_UINT)
  IF (_iSize EQ CS_8BIT)
	movzx	eax, BYTE PTR _src
  ELSEIF (_iSize EQ CS_16BIT)
	movzx	eax, WORD PTR _src
  ELSEIF (_iSize EQ CS_32BIT)
	mov	eax, DWORD PTR _src
	and	eax, 7FFFFFFFH	; fix due to the signed nature of cvtdq2ps
				; so, we cannot support any higher values here...
  ENDIF
 ELSEIF (_eFmt EQ DF_SINT)
  IF (_iSize EQ CS_8BIT)
	movsx	eax, BYTE PTR _src
  ELSEIF (_iSize EQ CS_16BIT)
	movsx	eax, WORD PTR _src
  ELSEIF (_iSize EQ CS_32BIT)
	mov	eax, DWORD PTR _src
  ENDIF
 ELSEIF (_eFmt EQ DF_FLOAT)
  IF (_iSize EQ CS_8BIT)
	xor	eax, eax	; unsupported
  ELSEIF (_iSize EQ CS_16BIT)
	INLINE_UNPACK_HALF_FLOAT _src
  ELSEIF (_iSize EQ CS_32BIT)
	mov	eax, DWORD PTR _src
  ENDIF
 ENDIF

ENDM

; loads the equivalent of 0.0 for the current format
INLINE_EXPAND_SSE_0 MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ

	xor	eax, eax

ENDM

; loads the equivalent of 1.0 for the current format
INLINE_EXPAND_SSE_1 MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ

 IF ((_eFmt EQ DF_UNORM) OR ((_eFmt EQ DF_UINT) AND (_bIntAsNorm EQ 1)))
  IF (_iSize EQ CS_8BIT)
	mov	eax, 0FFH
  ELSEIF (_iSize EQ CS_16BIT)
	mov	eax, 0FFFFH
  ELSEIF (_iSize EQ CS_32BIT)
	mov	eax, 0FFFFFFH	; 24 bits
  ENDIF
 ELSEIF ((_eFmt EQ DF_SNORM) OR ((_eFmt EQ DF_SINT) AND (_bIntAsNorm EQ 1)))
  IF (_bSignedRange EQ 1)
   IF (_iSize EQ CS_8BIT)
	mov	eax, 0FFH
   ELSEIF (_iSize EQ CS_16BIT)
	mov	eax, 0FFFFH
   ELSEIF (_iSize EQ CS_32BIT)
	mov	eax, 0FFFFFFH	; 24 bits
   ENDIF
  ELSE
   IF (_iSize EQ CS_8BIT)
	mov	eax, 07FH
   ELSEIF (_iSize EQ CS_16BIT)
	mov	eax, 07FFFH
   ELSEIF (_iSize EQ CS_32BIT)
	mov	eax, 07FFFFFH	; 24 bits
   ENDIF
  ENDIF
 ELSEIF (_eFmt EQ DF_UINT)
 	mov	eax, 1
 ELSEIF (_eFmt EQ DF_SINT)
 	mov	eax, 1
 ELSEIF (_eFmt EQ DF_FLOAT)
	mov	eax, DWORD PTR g_psOne
 ENDIF

ENDM


INLINE_UNPACK_FLOAT_SSE MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bOverlay:REQ, _overlayMask:REQ, _overlayMaskInverse:REQ

 IF ((_eFmt EQ DF_UNORM) OR ((_eFmt EQ DF_UINT) AND (_bIntAsNorm EQ 1)))
	cvtdq2ps	xmm0, [rsp]
  IF (_iSize EQ CS_8BIT)
	mulps		xmm0, g_psOne_255
  ELSEIF (_iSize EQ CS_16BIT)
	mulps		xmm0, g_psOne_65535
  ELSEIF (_iSize EQ CS_32BIT)
	mulps		xmm0, g_psOne_16777215
  ENDIF
 ELSEIF ((_eFmt EQ DF_SNORM) OR ((_eFmt EQ DF_SINT) AND (_bIntAsNorm EQ 1)))
  IF (_bSignedRange EQ 1)
	cvtdq2ps	xmm0, [rsp]
   IF (_iSize EQ CS_8BIT)
	mulps		xmm0, g_psOne_255
   ELSEIF (_iSize EQ CS_16BIT)
	mulps		xmm0, g_psOne_65535
   ELSEIF (_iSize EQ CS_32BIT)
	mulps		xmm0, g_psOne_16777215
   ENDIF
  ELSE
	; signed is a bit more tricky as we need to map -128 to -127 for example
	movaps		xmm0, [rsp]
	movaps		xmm2, xmm0
   IF (_iSize EQ CS_8BIT)
	pcmpeqd		xmm2, g_ps_minus128i
	movaps		xmm1, g_ps_minus127i
   ELSEIF (_iSize EQ CS_16BIT)
	pcmpeqd		xmm2, g_ps_minus32768i
	movaps		xmm1, g_ps_minus32767i
   ELSEIF (_iSize EQ CS_32BIT)
	pcmpeqd		xmm2, g_ps_minus8388608i
	movaps		xmm1, g_ps_minus8388607i
   ENDIF
	andps		xmm1, xmm2
	xorps		xmm2, g_psNOT
	andps		xmm0, xmm2
	orps		xmm0, xmm1
	cvtdq2ps	xmm0, xmm0
   IF (_iSize EQ CS_8BIT)
	mulps		xmm0, g_psOne_127
   ELSEIF (_iSize EQ CS_16BIT)
	mulps		xmm0, g_psOne_32767
   ELSEIF (_iSize EQ CS_32BIT)
	mulps		xmm0, g_psOne_8388607
   ENDIF
  ENDIF
 ELSEIF (_eFmt EQ DF_UINT)
	cvtdq2ps	xmm0, [rsp]
 ELSEIF (_eFmt EQ DF_SINT)
 	cvtdq2ps	xmm0, [rsp]
 ELSEIF (_eFmt EQ DF_FLOAT)
	movaps		xmm0, [rsp]
 ENDIF

 IF (_bOverlay EQ 1)
	movaps		xmm1, [edi]
	andps		xmm0, _overlayMask
	andps		xmm1, _overlayMaskInverse
	orps		xmm0, xmm1
 ENDIF

	movaps		[edi], xmm0

ENDM


; this macro returns the result in st(0)
; this macro may expect some constants on the top of the fpu stack
INLINE_UNPACK_COMPONENT_FLOAT MACRO _src:REQ, _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ
LOCAL @L1, @L2, @L3

 IF ((_eFmt EQ DF_UNORM) OR ((_eFmt EQ DF_UINT) AND (_bIntAsNorm EQ 1)))
  IF (_iSize EQ CS_8BIT)
	movzx	eax, BYTE PTR _src
	mov	DWORD PTR[rsp], eax
	fild	DWORD PTR[rsp]
	fmul	st(0), st(1)
  ELSEIF (_iSize EQ CS_16BIT)
	movzx	eax, WORD PTR _src
	mov	DWORD PTR[rsp], eax
	fild	DWORD PTR[rsp]
	fmul	st(0), st(1)
  ELSEIF (_iSize EQ CS_32BIT)
	mov	eax, DWORD PTR _src
	shr	eax, 8	; to 24 bis
	mov	DWORD PTR[rsp], eax
	fild	DWORD PTR[rsp]
	fmul	st(0), st(1)
  ENDIF
 ELSEIF ((_eFmt EQ DF_SNORM) OR ((_eFmt EQ DF_SINT) AND (_bIntAsNorm EQ 1)))
  IF (_bSignedRange EQ 1)
   IF (_iSize EQ CS_8BIT)
	movzx	eax, BYTE PTR _src
	add	al, 80H
	mov	DWORD PTR[rsp], eax
	fild	DWORD PTR[rsp]
	fmul	st(0), st(1)
   ELSEIF (_iSize EQ CS_16BIT)
	movzx	eax, WORD PTR _src
	add	ax, 8000H
	mov	DWORD PTR[rsp], eax
	fild	DWORD PTR[rsp]
	fmul	st(0), st(1)
   ELSEIF (_iSize EQ CS_32BIT)
	mov	eax, DWORD PTR _src
	add	eax, 80000000H
	shr	eax, 8	; to 24 bits
	mov	DWORD PTR[rsp], eax
	fild	DWORD PTR[rsp]
	fmul	st(0), st(1)
   ENDIF
  ELSE
   IF (_iSize EQ CS_8BIT)
	movsx	eax, BYTE PTR _src
	cmp	al, 80H
	je	@L1
	jmp	@F
@L1:
	mov	al, 81H		; map -128 to 127
@@:
	mov	DWORD PTR[rsp], eax
	fild	DWORD PTR[rsp]
	fmul	st(0), st(1)
   ELSEIF (_iSize EQ CS_16BIT)
	movsx	eax, WORD PTR _src
	cmp	ax, 8000H
	je	@L2
	jmp	@F
@L2:
	mov	ax, 8001H
@@:
	mov	DWORD PTR[rsp], eax
	fild	DWORD PTR[rsp]
	fmul	st(0), st(1)
   ELSEIF (_iSize EQ CS_32BIT)
	mov	eax, DWORD PTR _src
	cmp	eax, 80000000H
	je	@L3
	jmp	@F
@L3:
	mov	eax, 80000100H
@@:
	sar	eax, 8 ; to 24 bits
	mov	DWORD PTR[rsp], eax
	fild	DWORD PTR[rsp]
	fmul	st(0), st(1)
   ENDIF
  ENDIF
 ELSEIF (_eFmt EQ DF_UINT)
  IF (_iSize EQ CS_8BIT)
	movzx	eax, BYTE PTR _src
	mov	DWORD PTR[rsp], eax
	fild	DWORD PTR[rsp]
  ELSEIF (_iSize EQ CS_16BIT)
	movzx	eax, WORD PTR _src
	mov	DWORD PTR[rsp], eax
	fild	DWORD PTR[rsp]
  ELSEIF (_iSize EQ CS_32BIT)
	mov	eax, DWORD PTR _src
	mov	DWORD PTR[rsp], eax
	xor	edx, edx
	mov	DWORD PTR[rsp+04], edx	; signed fix
	fild	QWORD PTR[rsp]
  ENDIF
 ELSEIF (_eFmt EQ DF_SINT)
  IF (_iSize EQ CS_8BIT)
	movsx	eax, BYTE PTR _src
	mov	DWORD PTR[rsp], eax
	fild	DWORD PTR[rsp]
  ELSEIF (_iSize EQ CS_16BIT)
	movsx	eax, WORD PTR _src
	mov	DWORD PTR[rsp], eax
	fild	DWORD PTR[rsp]
  ELSEIF (_iSize EQ CS_32BIT)
	fild	DWORD PTR _src
  ENDIF
 ELSEIF (_eFmt EQ DF_FLOAT)
  IF (_iSize EQ CS_8BIT)
	fldz	; unsupported
  ELSEIF (_iSize EQ CS_16BIT)
	INLINE_UNPACK_HALF_FLOAT _src
	mov	DWORD PTR [rsp], eax
	fld	DWORD PTR [rsp]
	;movzx	eax, WORD PTR _src
	;push	rax
	;call	PLIB_UnpackHalfFloat
	;pop	rax
	;INVOKE PLIB_UnpackHalfFloat eax
  ELSEIF (_iSize EQ CS_32BIT)
	fld	DWORD PTR _src
  ENDIF
 ENDIF

ENDM




;//////////////////////////////////////////////////////////////////////

R_MASK equ 0FF0000H
G_MASK equ 0FF00H
B_MASK equ 0FFH
A_MASK equ 0FF000000H
RG_MASK equ 0FFFF00H
RGB_MASK equ 0FFFFFFH

;//////////////////////////////////////////////////////////////////////

; a void unpacker, just fills the pixel with the default values
INLINE_UNPACK_VOID MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 IF (_bFloat EQ 0)
  IF (_bOverlay EQ 0)
	;;mov	BYTE PTR [rdi+CH_R], DEFAULT_COLOR
	;;mov	BYTE PTR [rdi+CH_G], DEFAULT_COLOR
	;;mov	BYTE PTR [rdi+CH_B], DEFAULT_COLOR
	;;mov	BYTE PTR [rdi+CH_A], DEFAULT_ALPHA
	xor	eax, eax
	or	eax, A_MASK
	mov	DWORD PTR [rdi], eax
  ENDIF
 ELSE
  IF (_bOverlay EQ 0)
	fldz
	fst	DWORD PTR [rdi+CH_R*4]
	fst	DWORD PTR [rdi+CH_G*4]
	fstp	DWORD PTR [rdi+CH_B*4]
	fld1
	fstp	DWORD PTR [rdi+CH_A*4]
  ENDIF
 ENDIF

ENDM


INLINE_UNPACK_P MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 ; should be an index
 IF (_eFmt NE DF_UINT)
	error
 ENDIF

 IF (_iSize EQ CS_8BIT)
	movzx	edx, BYTE PTR [rsi]
 ELSEIF (_iSize EQ CS_16BIT)
	movzx	edx, WORD PTR [rsi]
 ELSEIF (_iSize EQ CS_32BIT)
	mov	edx, DWORD PTR [rsi]
 ENDIF

 IF (_bFloat EQ 0)
	mov	eax, DWORD PTR [rbx+rdx*4] ; the palette
  IF (_bOverlay EQ 0)
	;;and	eax, RGB_MASK
	or	eax, A_MASK
	mov	DWORD PTR [rdi], eax
  ELSE
	mov	edx, DWORD PTR [rdi]
	and	eax, RGB_MASK
	and	edx, NOT RGB_MASK
	or	eax, edx
	mov	DWORD PTR [rdi], eax
  ENDIF
 ELSE
	shl	edx, 4	; mul by 16 (the palette pixel is 16 bytes here)
	lea	rdx, [rbx+rdx]
	mov	eax, DWORD PTR [rdx+CH_0*4]
	mov	DWORD PTR [rdi+CH_0*4], eax
	mov	eax, DWORD PTR [rdx+CH_1*4]
	mov	DWORD PTR [rdi+CH_1*4], eax
	mov	eax, DWORD PTR [rdx+CH_2*4]
	mov	DWORD PTR [rdi+CH_2*4], eax
  IF (_bOverlay EQ 0)
	fld1
	fstp	DWORD PTR [rdi+CH_3*4]
  ENDIF
 ENDIF

ENDM


INLINE_UNPACK_R MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 IF (_bFloat EQ 0)
	INLINE_UNPACK_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_R], al
  IF (_bOverlay EQ 0)
	mov	BYTE PTR [rdi+CH_G], DEFAULT_COLOR
	mov	BYTE PTR [rdi+CH_B], DEFAULT_COLOR
	mov	BYTE PTR [rdi+CH_A], DEFAULT_ALPHA
  ENDIF
 ELSE
  IFNDEF _SSE
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_0*4]
   IF (_bOverlay EQ 0)
	fldz
	fst	DWORD PTR [rdi+CH_1*4]
	fstp	DWORD PTR [rdi+CH_2*4]
	fld1
	fstp	DWORD PTR [rdi+CH_3*4]
   ENDIF
  ELSE
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_0*4], eax
	INLINE_EXPAND_SSE_0 _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_1*4], eax
	mov	DWORD PTR [rsp+CH_2*4], eax
	INLINE_EXPAND_SSE_1 _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_3*4], eax
	INLINE_UNPACK_FLOAT_SSE _iSize, _eFmt, _bIntAsNorm, _bSignedRange, _bOverlay, g_psOverlayMask_R, g_psOverlayMask_R_I
  ENDIF
 ENDIF

ENDM


INLINE_UNPACK_G MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 IF (_bFloat EQ 0)
	INLINE_UNPACK_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_G], al
  IF (_bOverlay EQ 0)
	mov	BYTE PTR [rdi+CH_R], DEFAULT_COLOR
	mov	BYTE PTR [rdi+CH_B], DEFAULT_COLOR
	mov	BYTE PTR [rdi+CH_A], DEFAULT_ALPHA
  ENDIF
 ELSE
  IFNDEF _SSE
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_1*4]
   IF (_bOverlay EQ 0)
	fldz
	fst	DWORD PTR [rdi+CH_0*4]
	fstp	DWORD PTR [rdi+CH_2*4]
	fld1
	fstp	DWORD PTR [rdi+CH_3*4]
   ENDIF
  ELSE
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_1*4], eax
	INLINE_EXPAND_SSE_0 _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_0*4], eax
	mov	DWORD PTR [rsp+CH_2*4], eax
	INLINE_EXPAND_SSE_1 _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_3*4], eax
	INLINE_UNPACK_FLOAT_SSE _iSize, _eFmt, _bIntAsNorm, _bSignedRange, _bOverlay, g_psOverlayMask_G, g_psOverlayMask_G_I
  ENDIF
 ENDIF

ENDM


INLINE_UNPACK_B MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 IF (_bFloat EQ 0)
	INLINE_UNPACK_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_B], al
  IF (_bOverlay EQ 0)
	mov	BYTE PTR [rdi+CH_R], DEFAULT_COLOR
	mov	BYTE PTR [rdi+CH_G], DEFAULT_COLOR
	mov	BYTE PTR [rdi+CH_A], DEFAULT_ALPHA
  ENDIF
 ELSE
  IFNDEF _SSE
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_2*4]
   IF (_bOverlay EQ 0)
	fldz
	fst	DWORD PTR [rdi+CH_0*4]
	fstp	DWORD PTR [rdi+CH_1*4]
	fld1
	fstp	DWORD PTR [rdi+CH_3*4]
   ENDIF
  ELSE
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_2*4], eax
	INLINE_EXPAND_SSE_0 _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_0*4], eax
	mov	DWORD PTR [rsp+CH_1*4], eax
	INLINE_EXPAND_SSE_1 _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_3*4], eax
	INLINE_UNPACK_FLOAT_SSE _iSize, _eFmt, _bIntAsNorm, _bSignedRange, _bOverlay, g_psOverlayMask_B, g_psOverlayMask_B_I
  ENDIF
 ENDIF

ENDM


INLINE_UNPACK_A MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 IF (_bFloat EQ 0)
	INLINE_UNPACK_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_A], al
  IF (_bOverlay EQ 0)
	mov	BYTE PTR [rdi+CH_R], DEFAULT_COLOR
	mov	BYTE PTR [rdi+CH_G], DEFAULT_COLOR
	mov	BYTE PTR [rdi+CH_B], DEFAULT_COLOR
  ENDIF
 ELSE
  IFNDEF _SSE
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_3*4]
   IF (_bOverlay EQ 0)
	fldz
	fst	DWORD PTR [rdi+CH_0*4]
	fst	DWORD PTR [rdi+CH_1*4]
	fstp	DWORD PTR [rdi+CH_2*4]
   ENDIF
  ELSE
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_3*4], eax
	INLINE_EXPAND_SSE_0 _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_0*4], eax
	mov	DWORD PTR [rsp+CH_1*4], eax
	mov	DWORD PTR [rsp+CH_2*4], eax
	INLINE_UNPACK_FLOAT_SSE _iSize, _eFmt, _bIntAsNorm, _bSignedRange, _bOverlay, g_psOverlayMask_A, g_psOverlayMask_A_I
  ENDIF
 ENDIF

ENDM


INLINE_UNPACK_L MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 IF (_bFloat EQ 0)
	INLINE_UNPACK_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_R], al
	mov	BYTE PTR [rdi+CH_G], al
	mov	BYTE PTR [rdi+CH_B], al
  IF (_bOverlay EQ 0)
	mov	BYTE PTR [rdi+CH_A], DEFAULT_ALPHA
  ENDIF
 ELSE
  IFNDEF _SSE
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fst	DWORD PTR [rdi+CH_0*4]
	fst	DWORD PTR [rdi+CH_1*4]
	fstp	DWORD PTR [rdi+CH_2*4]
   IF (_bOverlay EQ 0)
	fld1
	fstp	DWORD PTR [rdi+CH_3*4]
   ENDIF
  ELSE
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_0*4], eax
	mov	DWORD PTR [rsp+CH_1*4], eax
	mov	DWORD PTR [rsp+CH_2*4], eax
	INLINE_EXPAND_SSE_1 _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_3*4], eax
	INLINE_UNPACK_FLOAT_SSE _iSize, _eFmt, _bIntAsNorm, _bSignedRange, _bOverlay, g_psOverlayMask_RGB, g_psOverlayMask_RGB_I
  ENDIF
 ENDIF

ENDM


INLINE_UNPACK_LA MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 IF (_bFloat EQ 0)
	INLINE_UNPACK_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_R], al
	mov	BYTE PTR [rdi+CH_G], al
	mov	BYTE PTR [rdi+CH_B], al
	INLINE_UNPACK_COMPONENT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_A], al
 ELSE
  IFNDEF _SSE
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fst	DWORD PTR [rdi+CH_0*4]
	fst	DWORD PTR [rdi+CH_1*4]
	fstp	DWORD PTR [rdi+CH_2*4]
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_3*4]
  ELSE
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_0*4], eax
	mov	DWORD PTR [rsp+CH_1*4], eax
	mov	DWORD PTR [rsp+CH_2*4], eax
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_3*4], eax
	INLINE_UNPACK_FLOAT_SSE _iSize, _eFmt, _bIntAsNorm, _bSignedRange, 0, 0, 0
  ENDIF
 ENDIF

ENDM


INLINE_UNPACK_I MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 IF (_bFloat EQ 0)
	INLINE_UNPACK_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_R], al
	mov	BYTE PTR [rdi+CH_G], al
	mov	BYTE PTR [rdi+CH_B], al
	mov	BYTE PTR [rdi+CH_A], al
 ELSE
  IFNDEF _SSE
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fst	DWORD PTR [rdi+CH_0*4]
	fst	DWORD PTR [rdi+CH_1*4]
	fst	DWORD PTR [rdi+CH_2*4]
	fstp	DWORD PTR [rdi+CH_3*4]
  ELSE
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_0*4], eax
	mov	DWORD PTR [rsp+CH_1*4], eax
	mov	DWORD PTR [rsp+CH_2*4], eax
	mov	DWORD PTR [rsp+CH_3*4], eax
	INLINE_UNPACK_FLOAT_SSE _iSize, _eFmt, _bIntAsNorm, _bSignedRange, 0, 0, 0
  ENDIF
 ENDIF

ENDM


INLINE_UNPACK_RG MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 IF (_bFloat EQ 0)
	INLINE_UNPACK_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_R], al
	INLINE_UNPACK_COMPONENT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_G], al
  IF (_bOverlay EQ 0)
	mov	BYTE PTR [rdi+CH_B], DEFAULT_COLOR
	mov	BYTE PTR [rdi+CH_A], DEFAULT_ALPHA
  ENDIF
 ELSE
  IFNDEF _SSE
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_0*4]
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_1*4]
   IF (_bOverlay EQ 0)
	fldz
	fstp	DWORD PTR [rdi+CH_2*4]
	fld1
	fstp	DWORD PTR [rdi+CH_3*4]
   ENDIF
  ELSE
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_0*4], eax
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_1*4], eax
	INLINE_EXPAND_SSE_0 _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_2*4], eax
	INLINE_EXPAND_SSE_1 _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_3*4], eax
	INLINE_UNPACK_FLOAT_SSE _iSize, _eFmt, _bIntAsNorm, _bSignedRange, _bOverlay, g_psOverlayMask_RG, g_psOverlayMask_RG_I
  ENDIF
 ENDIF

ENDM


INLINE_UNPACK_GR MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 IF (_bFloat EQ 0)

	INLINE_UNPACK_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_G], al
	INLINE_UNPACK_COMPONENT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_R], al
  IF (_bOverlay EQ 0)
	mov	BYTE PTR [rdi+CH_B], DEFAULT_COLOR
	mov	BYTE PTR [rdi+CH_A], DEFAULT_ALPHA
  ENDIF
 ELSE
  IFNDEF _SSE
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_1*4]
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_0*4]
   IF (_bOverlay EQ 0)
	fldz
	fstp	DWORD PTR [rdi+CH_2*4]
	fld1
	fstp	DWORD PTR [rdi+CH_3*4]
   ENDIF
  ELSE
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_1*4], eax
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_0*4], eax
	INLINE_EXPAND_SSE_0 _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_2*4], eax
	INLINE_EXPAND_SSE_1 _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_3*4], eax
	INLINE_UNPACK_FLOAT_SSE _iSize, _eFmt, _bIntAsNorm, _bSignedRange, _bOverlay, g_psOverlayMask_RG, g_psOverlayMask_RG_I
  ENDIF
 ENDIF

ENDM


; used for the RGB also
INLINE_UNPACK_RGBX MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 IF (_bFloat EQ 0)
	INLINE_UNPACK_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_R], al
	INLINE_UNPACK_COMPONENT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_G], al
	INLINE_UNPACK_COMPONENT [rsi+CH_2*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_B], al
  IF (_bOverlay EQ 0)
	mov	BYTE PTR [rdi+CH_A], DEFAULT_ALPHA
  ENDIF
 ELSE
  IFNDEF _SSE
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_0*4]
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_1*4]
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_2*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_2*4]
   IF (_bOverlay EQ 0)
	fld1
	fstp	DWORD PTR [rdi+CH_3*4]
   ENDIF
  ELSE
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_0*4], eax
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_1*4], eax
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_2*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_2*4], eax
	INLINE_EXPAND_SSE_1 _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_3*4], eax
	INLINE_UNPACK_FLOAT_SSE _iSize, _eFmt, _bIntAsNorm, _bSignedRange, _bOverlay, g_psOverlayMask_RGB, g_psOverlayMask_RGB_I
  ENDIF
 ENDIF

ENDM

; used for the BGR also
INLINE_UNPACK_BGRX MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 IF (_bFloat EQ 0)
	INLINE_UNPACK_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_B], al
	INLINE_UNPACK_COMPONENT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_G], al
	INLINE_UNPACK_COMPONENT [rsi+CH_2*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_R], al
  IF (_bOverlay EQ 0)
	mov	BYTE PTR [rdi+CH_A], DEFAULT_ALPHA
  ENDIF
 ELSE
  IFNDEF _SSE
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_2*4]
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_1*4]
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_2*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_0*4]
   IF (_bOverlay EQ 0)
	fld1
	fstp	DWORD PTR [rdi+CH_3*4]
   ENDIF
  ELSE
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_2*4], eax
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_1*4], eax
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_2*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_0*4], eax
	INLINE_EXPAND_SSE_1 _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_3*4], eax
	INLINE_UNPACK_FLOAT_SSE _iSize, _eFmt, _bIntAsNorm, _bSignedRange, _bOverlay, g_psOverlayMask_RGB, g_psOverlayMask_RGB_I
  ENDIF
 ENDIF

ENDM


INLINE_UNPACK_RGBA MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 IF (_bFloat EQ 0)
	INLINE_UNPACK_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_R], al
	INLINE_UNPACK_COMPONENT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_G], al
	INLINE_UNPACK_COMPONENT [rsi+CH_2*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_B], al
	INLINE_UNPACK_COMPONENT [rsi+CH_3*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_A], al
 ELSE
  IFNDEF _SSE
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_0*4]
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_1*4]
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_2*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_2*4]
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_3*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_3*4]
  ELSE
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_0*4], eax
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_1*4], eax
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_2*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_2*4], eax
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_3*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_3*4], eax
	INLINE_UNPACK_FLOAT_SSE _iSize, _eFmt, _bIntAsNorm, _bSignedRange, 0, 0, 0
  ENDIF
 ENDIF

ENDM


INLINE_UNPACK_BGRA MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 IF (_bFloat EQ 0)
	INLINE_UNPACK_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_B], al
	INLINE_UNPACK_COMPONENT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_G], al
	INLINE_UNPACK_COMPONENT [rsi+CH_2*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_R], al
	INLINE_UNPACK_COMPONENT [rsi+CH_3*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	BYTE PTR [rdi+CH_A], al
 ELSE
  IFNDEF _SSE
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_2*4]
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_1*4]
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_2*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_0*4]
	INLINE_UNPACK_COMPONENT_FLOAT [rsi+CH_3*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	fstp	DWORD PTR [rdi+CH_3*4]
  ELSE
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_0*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_2*4], eax
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_1*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_1*4], eax
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_2*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_0*4], eax
	INLINE_EXPAND_SSE_COMPONENT [rsi+CH_3*_iSize], _iSize, _eFmt, _bIntAsNorm, _bSignedRange
	mov	DWORD PTR [rsp+CH_3*4], eax
	INLINE_UNPACK_FLOAT_SSE _iSize, _eFmt, _bIntAsNorm, _bSignedRange, 0, 0, 0
  ENDIF
 ENDIF

ENDM


;//////////////////////////////////////////////////////////////////////
; special cases (speed up)


INLINE_UNPACK_B8G8R8X8_UNORM MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 IF (_bFloat EQ 0)
  IF (_bOverlay EQ 0)
	mov	eax, DWORD PTR [rsi]
	or	eax, A_MASK
	mov	DWORD PTR [rdi], eax
  ELSE
	mov	eax, DWORD PTR [rsi]
	mov	edx, DWORD PTR [rdi]
	and	eax, RGB_MASK
	and	edx, NOT RGB_MASK
	or	eax, edx
	mov	DWORD PTR [rdi], eax
  ENDIF
 ELSE
	INLINE_UNPACK_BGRX _iSize, _eFmt, _bIntAsNorm, _bSignedRange, _bFloat, _bOverlay
 ENDIF

ENDM


INLINE_UNPACK_B8G8R8A8_UNORM MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 IF (_bFloat EQ 0)
	mov	eax, DWORD PTR [rsi]
	mov	DWORD PTR [rdi], eax
 ELSE
	INLINE_UNPACK_BGRA _iSize, _eFmt, _bIntAsNorm, _bSignedRange, _bFloat, _bOverlay
 ENDIF

ENDM



; the input bits are in edx
INLINE_DECODE_SE_COMPONENT MACRO _iCH, _nBits, _bShift, _bFloat

	mov	eax, edx
 IF (_bShift EQ 1)
	shr	edx, _nBits
 ENDIF
 IF (_nBits EQ 9)
	and	eax, 1FFH
 ELSE
	error
 ENDIF
	mov	DWORD PTR [rsp+04], eax
	fild	DWORD PTR [rsp+04]
	fscale
 IF (_bFloat EQ 0)
	fmul	st(0), st(1)	; 255.0 should be on the fpu stack
	fistp	DWORD PTR [rsp+04]
	mov	eax, DWORD PTR [rsp+04]
	INLINE_CLAMP_SIGNED_255
	mov	BYTE PTR [rsp+_iCH], al
 ELSE
	fstp	DWORD PTR [rdi+_iCH*4]
 ENDIF

ENDM

; this macro expects some space under rsp/esp
; the output is a triple of 32-bit floats
INLINE_DECODE_R9G9B9E5_FLOAT MACRO _iCH_0, _iCH_1, _iCH_2, _bFloat

	mov	edx, DWORD PTR [rsi]
	mov	eax, edx
	shr	eax, 27	; extract the exponent
	sub	eax, 15 + 9 ; apply the bias
	; do it simple for now
	mov	DWORD PTR [rsp+04], eax
	fild	DWORD PTR [rsp+04]
	INLINE_DECODE_SE_COMPONENT _iCH_0, 9, 1, _bFloat
	INLINE_DECODE_SE_COMPONENT _iCH_1, 9, 1, _bFloat
	INLINE_DECODE_SE_COMPONENT _iCH_2, 9, 0, _bFloat
	fstp	st(0)

ENDM



INLINE_UNPACK_R9G9B9E5_FLOAT MACRO _iSize:REQ, _eFmt:REQ, _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ

 IF (_bFloat EQ 0)
	INLINE_DECODE_R9G9B9E5_FLOAT CH_R, CH_G, CH_B, 0
  IF (_bOverlay EQ 0)
	mov	eax, DWORD PTR [rsp]
	or	eax, A_MASK
	mov	DWORD PTR [rdi], eax
  ELSE
	mov	eax, DWORD PTR [rsp]
	mov	edx, DWORD PTR [rdi]
	and	eax, RGB_MASK
	and	edx, NOT RGB_MASK
	or	eax, edx
	mov	DWORD PTR [rdi], eax
  ENDIF
 ELSE
	INLINE_DECODE_R9G9B9E5_FLOAT CH_0, CH_1, CH_2, 1
 ENDIF

ENDM


;//////////////////////////////////////////////////////////////////////


LOOP_UNPACK_RGB MACRO _bIntAsNorm:REQ, _bSignedRange:REQ, _bFloat:REQ, _bOverlay:REQ
LOCAL @I

align 16
@I:
	; do the function
	%_fnDo _iSize, _eFmt, _bIntAsNorm, _bSignedRange, _bFloat, _bOverlay

	; offset to the next pixel and check the counter
	add	rsi, _nBytesIn
	add	rdi, _nBytesOut
	sub	ecx, 1
	jnz	@I

ENDM



comment '

the swapping pseudocode

iImagePitch = iImageWidth*BYTES_OUT;
iImageSize = iImagePitch*iImageHeight;

if ( SWAP_XY )
{
	if ( SWAP_TOP_BOTTOM )
	{
		if ( SWAP_LEFT_RIGHT )
		{
			iAddX = -iImagePitch;
			iAddY = iImageSize-BYTES_OUT;
			p += (iImageHeight-1)*iImagePitch+(iImageWidth-iStartLine-1)*BYTES_OUT;
		}
		else
		{
			iAddX = -iImagePitch;
			iAddY = iImageSize+BYTES_OUT;
			p += (iImageHeight-1)*iImagePitch+iStartLine*BYTES_OUT;
		}
	}
	else
	{
		if ( SWAP_LEFT_RIGHT )
		{
			iAddX = iImagePitch;
			iAddY = -iImageSize-BYTES_OUT;
			p += (iImageWidth-iStartLine-1)*BYTES_OUT;
		}
		else
		{
			iAddX = iImagePitch;
			iAddY = -iImageSize+BYTES_OUT;
			p += iStartLine*BYTES_OUT;
		}
	}
}
else
{
	if ( SWAP_TOP_BOTTOM )
	{
		if ( SWAP_LEFT_RIGHT )
		{
			iAddX = -BYTES_OUT;
			iAddY = 0;
			p += (iImageHeight-iStartLine-1)*iImagePitch+(iImagePitch-BYTES_OUT);
		}
		else
		{
			iAddX = BYTES_OUT;
			iAddY = -iImagePitch*2;
			p += (iImageHeight-iStartLine-1)*iImagePitch;
		}
	}
	else
	{
		if ( SWAP_LEFT_RIGHT )
		{
			iAddX = -BYTES_OUT;
			iAddY = iImagePitch*2;
			p += iStartLine*iImagePitch+(iImagePitch-BYTES_OUT);
		}
		else
		{
			iAddX = BYTES_OUT;
			iAddY = 0;
			p += iStartLine*iImagePitch;
		}
	}
}

// this can be converted to:

if ( SWAP_XY )
{
	if ( SWAP_TOP_BOTTOM )
	{
		iAddX = -iImagePitch;
		iAddY = (iImagePitch*iImageHeight);
		p += iImagePitch*(iImageHeight-1);
	}
	else
	{
		iAddX = iImagePitch;
		iAddY = -(iImagePitch*iImageHeight);
	}

	if ( SWAP_LEFT_RIGHT )
	{
		iAddY += -BYTES_OUT;
		p += (iImageWidth-iStartLine-1)*BYTES_OUT;
	}
	else
	{
		iAddY += BYTES_OUT;
		p += iStartLine*BYTES_OUT;
	}
}
else
{
	if ( SWAP_LEFT_RIGHT )
	{
		iAddX = -BYTES_OUT;
		iAddY = iImagePitch;
		p += iImagePitch-BYTES_OUT;
	}
	else
	{
		iAddX = BYTES_OUT;
		iAddY = -iImagePitch;
	}

	if ( SWAP_TOP_BOTTOM )
	{
		iAddY += -iImagePitch;
		p += (iImageHeight-iStartLine-1)*iImagePitch;
	}
	else
	{
		iAddY += iImagePitch;
		p += iStartLine*iImagePitch;
	}
}


'


BYTES_OUT equ 4
BYTES_OUT_FLOAT equ 16

; this should unpack any of the known internal formats to the native format (BGRA)
; or to an array of 32-bit floats (RGBA), acceptable by the image processor (OBSOLETE)
TEMPLATE_PROC_UNPACK_RGB_FUNC MACRO _aFuncName:REQ, _nComps:REQ, _iSize:REQ, _eFmt:REQ, _fnDo:REQ, _palette, _bFloat, _bOverlay
 LOCAL _nBytesIn, _nBytesOut, _supportedOpts, _bProcessingSupported, _bPushUnp, _pushUnpVal

	; evaluate the macro params first
 _nBytesIn = _nComps * _iSize	; how much to offset pIn with each pixel
 IF (_bFloat EQ 0)
  _nBytesOut = BYTES_OUT
 ELSE
  _nBytesOut = BYTES_OUT_FLOAT
 ENDIF
 IFNB <_palette>		; no palette formats support options
  _supportedOpts = 0
; ELSEIF (_bFloat EQ 1)		; conversion to float doesn't support any options currently
;  _supportedOpts = 0
 ELSEIF (_eFmt EQ DF_UNORM)
  _supportedOpts = 0
 ELSEIF (_eFmt EQ DF_SNORM)
  _supportedOpts = DPO_SIGNED_RANGE
 ELSEIF (_eFmt EQ DF_UINT)
  _supportedOpts = DPO_INT_NORM
 ELSEIF (_eFmt EQ DF_SINT)
  _supportedOpts = DPO_SIGNED_RANGE + DPO_INT_NORM
 ELSEIF (_eFmt EQ DF_FLOAT)
  _supportedOpts = 0
 ;;ELSEIF (_eFmt EQ DF_INDEX)
 ;; _supportedOpts = 0
 ENDIF
 ;;IF (_eFmt EQ DF_INDEX)
 ;IFNDEF _SSE
 IF (_bFloat EQ 0)
  IF (_eFmt EQ DF_FLOAT)
   _pushUnpVal = g_fl255
   _bPushUnp = 1
  ELSE
   _bPushUnp = 0
  ENDIF
 ELSE
  IF (_eFmt EQ DF_UNORM)
   IF (_iSize EQ CS_8BIT)
    _pushUnpVal = g_flOne_255
   ELSEIF (_iSize EQ CS_16BIT)
    _pushUnpVal = g_flOne_65535
   ELSEIF (_iSize EQ CS_32BIT)
    _pushUnpVal = g_flOne_16777215
   ENDIF
   _bPushUnp = 1
  ELSEIF (_eFmt EQ DF_SNORM)
   IF (_iSize EQ CS_8BIT)
    _pushUnpVal = g_flOne_127
   ELSEIF (_iSize EQ CS_16BIT)
    _pushUnpVal = g_flOne_32767
   ELSEIF (_iSize EQ CS_32BIT)
    _pushUnpVal = g_flOne_8388607
   ENDIF
   _bPushUnp = 1
  ELSE
   _bPushUnp = 0
  ENDIF
 ENDIF
 ;ENDIF

 APIENTRY PLIB_Unpack&_aFuncName, USES rsi rdi rbx, pOut:PTR BYTE, pIn:PTR BYTE, iImageWidth:DWORD, iImageHeight:DWORD, iStartLine:DWORD, nLines:DWORD, pPal:PTR BYTE, opts:DWORD

	%LOCAL	pStackPtr	:PTR BYTE
	%LOCAL	iAddY		:PTR BYTE	; flat offset change per Y step (can be negative)
	%LOCAL	iAddX		:PTR BYTE	; same for X step
	%LOCAL	iWidth		:DWORD		; this depend on the SWAP_XY flag, it is iImageHeight if the flag is set, iImageWidth otherwise
	%LOCAL	iImagePitch	:DWORD		; iImageWidth * _nBytesOut
	%LOCAL	iY		:DWORD		; counter (out of registers, used when there's a palette, stored in ebx otherwise)

	; init source, dest, and the counter
	mov	rsi, pIn
	mov	rdi, pOut

	mov	ebx, opts

	; init width
	mov	eax, iImageWidth
	test	ebx, DPO_SWAP_XY
	jz	@F
	mov	eax, iImageHeight
@@:
	mov	iWidth, eax

	; init pitch
	mov	eax, _nBytesOut
	mov	ecx, iImageWidth
	imul	eax, ecx
	mov	iImagePitch, eax

	; see the above pseudocode for more info
	test	ebx, DPO_SWAP_XY
	jnz	@swapXY

	mov	eax, iImagePitch
	mov	ecx, _nBytesOut
	neg	rax
	test	ebx, DPO_SWAP_LEFT_RIGHT
	jz	@F
	sub	rdi, rax	; p += iImagePitch-BYTES_OUT;
	sub	rdi, rcx	;  rax is negative here
	neg	rax
	neg	rcx
@@:
	mov	iAddX, rcx
	mov	iAddY, rax

	mov	eax, iStartLine
	mov	ecx, iImagePitch
	test	ebx, DPO_SWAP_TOP_BOTTOM
	jz	@F
	add	eax, 1			; (iImageHeight-iStartLine-1)
	neg	eax			;
	add	eax, iImageHeight	;
	neg	rcx
@@:
	add	iAddY, rcx
	mov	ecx, iImagePitch
	imul	rax, rcx
	add	rdi, rax
	jmp	@L1

@swapXY:
	mov	eax, iImageHeight
	mov	ecx, iImagePitch
	imul	rax, rcx
	neg	rax
	test	ebx, DPO_SWAP_TOP_BOTTOM
	jz	@F
	sub	rdi, rax	; p += iImagePitch*(iImageHeight-1);
	sub	rdi, rcx	;  rax is negative here
	neg	rax
	neg	rcx
@@:
	mov	iAddX, rcx
	mov	iAddY, rax

	mov	eax, iStartLine
	mov	ecx, _nBytesOut
	test	ebx, DPO_SWAP_LEFT_RIGHT
	jz	@F
	add	eax, 1			; (iImageWidth-iStartLine-1)
	neg	eax			;
	add	eax, iImageWidth	;
	neg	rcx
@@:
	add	iAddY, rcx
	mov	ecx, _nBytesOut
	imul	rax, rcx
	add	rdi, rax
@L1:


 IFNB <_palette>
	mov	rbx, pPal
 ENDIF
	; save the stack ptr
	mov	pStackPtr, rsp
	; align the stack properly
	and	rsp, -16 ; align the current pointer by 16
	; allocate several additional dwords for the fpu/xmm load/store ops
	sub	rsp, 16

	; some sub routines may expect a constant on the fpu stack
 ;IFNDEF _SSE
  IF ( _bPushUnp NE 0 )
	fld	_pushUnpVal
  ENDIF
 ;ENDIF

	; TODO: unsupport these
	; check the options
	mov	eax, opts
 IF ((_supportedOpts AND DPO_INT_NORM) NE 0)
	test	eax, DPO_INT_NORM
	jnz	@intnorm
 ENDIF
 IF ((_supportedOpts AND DPO_SIGNED_RANGE) NE 0)
	test	eax, DPO_SIGNED_RANGE
	jnz	@srange1
 ENDIF

	; TODO: try unrolling
	;;shr	ecx, 2
	;;jz	@low

 IFNB <_palette>
	mov	eax, nLines
	mov	iY, eax
 ELSE
	mov	ebx, nLines
 ENDIF
@Y:
	mov	ecx, iWidth
align 16
@X:
	; do the function
	%_fnDo _iSize, _eFmt, 0, 0, _bFloat, _bOverlay

	; offset to the next pixel and check the counter
	add	rsi, _nBytesIn
	add	rdi, iAddX
	sub	ecx, 1
	jnz	@X
@@:
	add	rdi, iAddY
 IFNB <_palette>
	sub	iY, 1
 ELSE
	sub	ebx, 1
 ENDIF
	jnz	@Y


 IF ((_supportedOpts AND DPO_SIGNED_RANGE) NE 0)
	jmp	@quit	; this is put here for the above code...
@srange1:
align 16
@I1:
	; do the function
	%_fnDo _iSize, _eFmt, 0, 1, _bFloat, _bOverlay

	; offset to the next pixel and check the counter
	add	rsi, _nBytesIn
	add	rdi, _nBytesOut
	sub	ecx, 1
	jnz	@I1
@@:
 ENDIF

 IF ((_supportedOpts AND DPO_INT_NORM) NE 0)
	jmp	@quit	; this is put here for the above code...
@intnorm:
  IF ((_supportedOpts AND DPO_SIGNED_RANGE) NE 0)
	test	eax, DPO_SIGNED_RANGE
	jnz	@srange2
  ENDIF
align 16
@I2:
	; do the function
	%_fnDo _iSize, _eFmt, 1, 0, _bFloat, _bOverlay

	; offset to the next pixel and check the counter
	add	rsi, _nBytesIn
	add	rdi, _nBytesOut
	sub	ecx, 1
	jnz	@I2
@@:

  IF ((_supportedOpts AND DPO_SIGNED_RANGE) NE 0)
	jmp	@quit	; this is put here for the above code...
@srange2:
align 16
@I3:
	; do the function
	%_fnDo _iSize, _eFmt, 1, 1, _bFloat, _bOverlay

	; offset to the next pixel and check the counter
	add	rsi, _nBytesIn
	add	rdi, _nBytesOut
	sub	ecx, 1
	jnz	@I3
@@:
  ENDIF
 ENDIF

@quit:

 ;IFNDEF _SSE
  IF ( _bPushUnp NE 0 )
	fstp	st(0)
  ENDIF
 ;ENDIF

	; restore the stack
	mov	rsp, pStackPtr

	ret

 ENDENTRY PLIB_Unpack&_aFuncName

ENDM


TEMPLATE_PROC_UNPACK_RGB MACRO _aFuncName:REQ, _nComps:REQ, _iSize:REQ, _eFmt:REQ, _fnDo:REQ, _palette

	TEMPLATE_PROC_UNPACK_RGB_FUNC _aFuncName, _nComps, _iSize, _eFmt, _fnDo, _palette, 0, 0
	TEMPLATE_PROC_UNPACK_RGB_FUNC O&_aFuncName, _nComps, _iSize, _eFmt, _fnDo, _palette, 0, 1
	TEMPLATE_PROC_UNPACK_RGB_FUNC F&_aFuncName, _nComps, _iSize, _eFmt, _fnDo, _palette, 1, 0
	TEMPLATE_PROC_UNPACK_RGB_FUNC FO&_aFuncName, _nComps, _iSize, _eFmt, _fnDo, _palette, 1, 1

ENDM



;//////////////////////////////////////////////////////////////////////

; expects 255.0 in st(0)
; ~ gamma in st2
; ~ delta mul in st3
; ~ delta in st4
CONVERT_OUTPUT_FLOAT1 MACRO _src:REQ, _dst:REQ, _eOutputGamma:REQ
LOCAL @zero, @one, @done

	; clip it from the bottom here (also required for INLINE_POW to work)
	; all that is negative is clipped to 0.0
	mov	eax, DWORD PTR _src
	test	eax, eax
	js	@zero

	; clip from the top... (we can do it simple as well here)
	; clip to 1.0 from the top
	cmp	eax, 3F800000H
	jae	@one

	fld	DWORD PTR _src
 IF (_eOutputGamma EQ GM_LINEAR)
	; do nothing
 ELSEIF (_eOutputGamma EQ GM_SRGB)
	INLINE_POW st(2)	; st2 is gamma value
	fmul	st(3)		; st3 is delta mul
	fsub	st(4)		; st4 is delta
 ELSEIF (_eOutputGamma EQ GM_SPECIFY)
	INLINE_POW st(2)	; st2 is gamma value
 ELSE
	error
 ENDIF
	fmul	st(0), st(1)		; st(1) is 255.0
	fistp	DWORD PTR [rsp]
	mov	eax, DWORD PTR [rsp]

	; clamp again as the FPU might have spoiled the berries
	test	eax, 0FFFFFF00H
	js	@zero
	jnz	@one
	jmp	@done
@zero:
	xor	eax, eax
	jmp	@done
@one:
	mov	eax, 255
@done:
	mov	BYTE PTR _dst, al

ENDM

; expects 255.0 in st(0)
CONVERT_OUTPUT_FLOAT MACRO _msk:REQ, _src:REQ, _dst:REQ, _cDef:REQ
LOCAL @zero, @one, @done, @def

	mov	eax, _msk
	test	eax, eax
	jz	@def

	fld	DWORD PTR _src
	fmul	st(0), st(1)		; st(1) is 255.9999
	fistp	DWORD PTR [rsp]
	mov	eax, DWORD PTR [rsp]

	; clamp 
	test	eax, 0FFFFFF00H
	js	@zero
	jnz	@one
	jmp	@done
@zero:
	xor	eax, eax
	jmp	@done
@one:
	mov	al, 255
	jmp	@done
@def:
	mov	al, _cDef
@done:
	mov	BYTE PTR _dst, al

ENDM

; XXX: ensure the source is 16-bytes aligned, using MOVUPS until then
APIENTRY PLIB_ConvertOutputFloat, USES rsi rdi rbx, pOut:PTR BYTE, pIn:PTR BYTE, iOutWidth:DWORD, iOutHeight:DWORD, prect:PTR RECT, psChannelMask:PTR DWORD

	%LOCAL	pStackPtr			:PTR BYTE
	%LOCAL	iInWidth			:DWORD
	%LOCAL	iInHeight			:DWORD
	%LOCAL	iOutAddY			:DWORD
	%LOCAL	iOutPitch			:DWORD

	; save the stack ptr
	mov	pStackPtr, rsp
	; align the stack properly
	and	rsp, -16 ; align the current pointer by 16
	; allocate several additional dwords for the fpu/xmm load/store ops
	sub	rsp, 16

	mov	rbx, prect

	; calculate the input dimensions
	mov	eax, (RECT PTR [rbx]).right
	sub	eax, (RECT PTR [rbx]).left
	mov	iInWidth, eax
	mov	eax, (RECT PTR [rbx]).bottom
	sub	eax, (RECT PTR [rbx]).top
	mov	iInHeight, eax

	; output pitch
	mov	edx, iOutWidth
	imul	eax, edx, BYTES_OUT
	mov	iOutPitch, eax

	; output offset after the current line finish
	mov	edx, iOutWidth
	sub	edx, iInWidth	; it's just the difference between the two widths
	imul	eax, edx, BYTES_OUT
	mov	iOutAddY, eax

	;int	3

	; load constants
 IFDEF _SSE
	movaps	xmm1, g_psZero
	movaps	xmm2, g_psOne
	movaps	xmm3, g_ps255_999
	movaps	xmm4, g_psDef
	mov	rax, psChannelMask
	movups	xmm5, [rax]
	movaps	xmm6, xmm5
	pxor	xmm6, g_psNOT
	andps	xmm4, xmm6
 ELSE
	fld	g_fl255_999
 ENDIF

	; init source, dest
	mov	rsi, pIn
	mov	rdi, pOut
	; offset from the top
	mov	eax, (RECT PTR [rbx]).top
	imul	eax, iOutPitch
	add	rdi, rax
	; offset from the left
	mov	edx, (RECT PTR [rbx]).left
	imul	eax, edx, BYTES_OUT
	add	rdi, rax

	mov	rdx, psChannelMask

	mov	ebx, iInHeight
@Y:
	mov	ecx, iInWidth

align 16
@X:

 IFNDEF _SSE
	CONVERT_OUTPUT_FLOAT [rdx+00], [rsi+00], [rdi+CH_R], 0
	CONVERT_OUTPUT_FLOAT [rdx+04], [rsi+04], [rdi+CH_G], 0
	CONVERT_OUTPUT_FLOAT [rdx+08], [rsi+08], [rdi+CH_B], 0
	CONVERT_OUTPUT_FLOAT [rdx+12], [rsi+12], [rdi+CH_A], -1
 ELSE
	movups		xmm0, [rsi]

	; clamp to 0.0 - 1.0
	maxps		xmm0, xmm1	; xmm1 = 0.0
	minps		xmm0, xmm2	; xmm2 = 1.0

	andps		xmm0, xmm5	; xmm5 = mask
	orps		xmm0, xmm4	; xmm4 = masked defaults

	; convert to integer
	mulps		xmm0, xmm3	; xmm3 = 255.9999

	; old mmx code;
	;cvtps2pi	mm1, xmm0
	;movq		[rsp], mm1
	;movhlps		xmm0, xmm0
	;cvtps2pi	mm1, xmm0
	;movq		[rsp+8], mm1

	cvttps2dq	xmm0, xmm0
	movups		[rsp], xmm0

	; XXX: can do this via shifting eax (not much needed though)
	mov	eax, DWORD PTR [rsp+00]
	mov	BYTE PTR [rdi+CH_R], al
	mov	eax, DWORD PTR [rsp+04]
	mov	BYTE PTR [rdi+CH_G], al
	mov	eax, DWORD PTR [rsp+08]
	mov	BYTE PTR [rdi+CH_B], al
	mov	eax, DWORD PTR [rsp+12]
	mov	BYTE PTR [rdi+CH_A], al
 ENDIF

	; offset to the next pixel and check the counter
	add	rsi, BYTES_OUT_FLOAT
	add	rdi, BYTES_OUT
	sub	ecx, 1
	jnz	@X
@@:
	mov	eax, iOutAddY
	add	rdi, rax
	sub	ebx, 1
	jnz	@Y
@QUIT:

	; cleanup
 IFNDEF _SSE
	fstp	st(0)
 ELSE
	;emms ;-- no need right now (used for MMX only)
 ENDIF

	; restore the stack
	mov	rsp, pStackPtr

	ret

ENDENTRY PLIB_ConvertOutputFloat

;//////////////////////////////////////////////////////////////////////


; adapted from github romeric/fastapprox
; the value of X is in xmm0, stores the result in xmm0
; uses xmm 0 thru 3 (0, 1, 2, and 3)
; negative values for x are not accepted here
INLINE_POW_SSE MACRO _flY

	; pow(x,y)=pow2(log2(x)*y)

	;int	3

	; do log2

	movaps		xmm1, xmm0
	andps		xmm1, g_psLOG2_MaskAND
	orps		xmm1, g_psLOG2_MaskOR

	cvtdq2ps	xmm0, xmm0
	mulps		xmm0, g_psLOG2_1_192E7
	subps		xmm0, g_psLOG2_124_225

	movaps		xmm2, xmm1
	mulps		xmm2, g_psLOG2_1_498
	subps		xmm0, xmm2

	movaps		xmm2, g_psLOG2_1_725
	addps		xmm1, g_psLOG2_0_352
	divps		xmm2, xmm1
	subps		xmm0, xmm2


	; mul y

	;int	3

	mulps		xmm0, _flY

	

	; do pow2

	; the offset for the negative values
	movaps		xmm1, xmm0
	cmpltps		xmm1, g_psZero
	cvtdq2ps	xmm3, xmm1

	; clip to -126 form the bottom
	maxps		xmm0, g_psPOW2_s126max

	; get the remainder
	movaps		xmm1, xmm0
	cvttps2dq	xmm2, xmm0
	cvtdq2ps	xmm2, xmm2
	subps		xmm1, xmm2
	subps		xmm1, xmm3

	addps		xmm0, g_psPOW2_121_274

	movaps		xmm2, g_psPOW2_27_728
	movaps		xmm3, g_psPOW2_4_842
	subps		xmm3, xmm1
	divps		xmm2, xmm3
	addps		xmm0, xmm2

	mulps		xmm1, g_psPOW2_1_490
	subps		xmm0, xmm1

	mulps		xmm0, g_psPOW2_1shl23
	cvttps2dq	xmm0, xmm0

ENDM


; expects gamma in xmm5 if _eGamma is GM_SPECIFY
INLINE_ENCODE_GAMMA_SSE MACRO _src:REQ, _eGamma:REQ

	; load
	movups	xmm0, _src

	; need to check if the values are not negative before loading them
	movaps		xmm4, xmm0
	andps		xmm4, g_psKeepSign ; preserve the signs, then simply OR them on the exit
	andps		xmm0, g_psClearSign

 IF (_eGamma EQ GM_SRGB)
	movaps	xmm5, xmm0
	; store the masks
	movaps	xmm6, xmm0
	cmpleps	xmm6, g_psSRGB_EncRamp
	movaps	xmm7, xmm6
	xorps	xmm7, g_psNOT
	; do the gamma curve branch
	INLINE_POW_SSE g_psSRGB_EncGamma
	mulps	xmm0, g_psSRGB_EncDeltaMul
	addps	xmm0, g_psSRGB_EncDelta
	; do the gamma ramp branch
	mulps	xmm5, g_psSRGB_EncRampMul
	; select the proper branch
	andps	xmm0, xmm7
	andps	xmm5, xmm6
	orps	xmm0, xmm5
 ELSEIF (_eGamma EQ GM_SPECIFY)
	; simple pow
	INLINE_POW_SSE xmm5
 ENDIF

	; restore the signs
	orps		xmm0, xmm4

	; mask out the alpha channel
	andps	xmm0, g_psClearAlpha
	movups	xmm1, _src
	andps	xmm1, g_psKeepAlpha
	orps	xmm0, xmm1

	; store
	movups	_src, xmm0

ENDM


; pow( x, y )
; a very simple and unoptimized version
; assumes x in st(0) and y in _flPowReg
; doesn't perform the check for x >= 0, you should do this manually
INLINE_POW MACRO _flPowReg:REQ
LOCAL @neg

	; pow(x,y)=2^exp2(y*log2(x));

	; compute y*log2(x)
	fld	_flPowReg
	fxch
	fyl2x

	; convert to the range -1.0-+1.0 for f2xm1
	fld1
	fld	st(1)
	fprem

	; compute 2^exp2(y*log2(x))
	f2xm1
	fadd
	fscale

	; cleanup and return the value
	fxch
	fstp	st(0)

ENDM

; XXX: we can optimize it, knowing that the encoding gamma goes
; straight to OutputFloat, so, clamping the value here may help to speed it up
INLINE_ENCODE_GAMMA_COMPONENT MACRO _src:REQ, _eGamma:REQ
LOCAL @ramp

	fld	DWORD PTR _src

	mov	ebx, DWORD PTR _src
	test	ebx, ebx
	jns	@F
	fchs
@@:

 IF (_eGamma EQ GM_SRGB)
	fcom	st(5)
	fnstsw	ax
	and	ax, FPU_CONTROL_WORD
	cmp	ax, FPU_LESS
	je	@ramp
	INLINE_POW st(1)
	fmul	st(0), st(2)
	fadd	st(0), st(3)
	jmp	@F
@ramp:
	fmul	st(0), st(4)
@@:
 ELSEIF (_eGamma EQ GM_SPECIFY)
	INLINE_POW st(1)
 ENDIF

	test	ebx, ebx
	jns	@F
	fchs
@@:
	fstp	DWORD PTR _src

ENDM



INLINE_ENCODE_GAMMA MACRO _eGamma:REQ
LOCAL @I

	; load the constants
 IF (_eGamma EQ GM_SRGB)
  IFNDEF _SSE
	fld	QWORD PTR g_flSRGB_EncRamp
	fld	QWORD PTR g_flSRGB_EncRampMul
	fld	QWORD PTR g_flSRGB_EncDelta
	fld	QWORD PTR g_flSRGB_EncDeltaMul
	fld	QWORD PTR g_flSRGB_EncGamma
  ELSE
	; nothing
  ENDIF
 ELSEIF (_eGamma EQ GM_SPECIFY)
  IFNDEF _SSE
	fld1
	fld	(GAMMA PTR [rbx]).flGamma
	fdiv
  ELSE
	movd	xmm0, (GAMMA PTR [rbx]).flGamma
	shufps	xmm0, xmm0, 0
	movaps	xmm5, g_psOne
	divps	xmm5, xmm0
  ENDIF
 ENDIF

align 16
@I:
 IFNDEF _SSE
	INLINE_ENCODE_GAMMA_COMPONENT [rdi+00], _eGamma
	INLINE_ENCODE_GAMMA_COMPONENT [rdi+04], _eGamma
	INLINE_ENCODE_GAMMA_COMPONENT [rdi+08], _eGamma
	;INLINE_DECODE_GAMMA_COMPONENT [rdi+12], GM_LINEAR
 ELSE
	INLINE_ENCODE_GAMMA_SSE [rdi], _eGamma
 ENDIF

	; offset to the next pixel and check the counter
	add	rdi, BYTES_OUT_FLOAT
	sub	ecx, 1
	jnz	@I
@@:

 	; pop the constants
 IF (_eGamma EQ GM_SRGB)
  IFNDEF _SSE
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
  ENDIF
 ELSEIF (_eGamma EQ GM_SPECIFY)
  IFNDEF _SSE
	fstp	st(0)
  ENDIF
 ENDIF

ENDM


APIENTRY PLIB_EncodeGamma, USES rsi rdi rbx, pDst:PTR BYTE, iCount:DWORD, pGamma:PTR GAMMA

	mov	rdi, pDst
	mov	ecx, iCount

	mov	ebx, pGamma
	mov	eax, (GAMMA PTR [rbx]).eGamma
	cmp	eax, GM_LINEAR
	je	@quit
	cmp	eax, GM_SRGB
	je	@srgb
	cmp	eax, GM_SPECIFY
	je	@specify
	int	3
@srgb:
	INLINE_ENCODE_GAMMA GM_SRGB
	jmp	@quit
@specify:
	INLINE_ENCODE_GAMMA GM_SPECIFY
	jmp	@quit
@quit:
	ret

ENDENTRY PLIB_EncodeGamma



; expects gamma in xmm5 if _eGamma is GM_SPECIFY
INLINE_DECODE_GAMMA_SSE MACRO _src:REQ, _eGamma:REQ

	; load
	movups	xmm0, _src

	; need to check if the values are not negative before loading them
	movaps		xmm4, xmm0
	andps		xmm4, g_psKeepSign ; preserve the signs, then simply OR them on the exit
	andps		xmm0, g_psClearSign

 IF (_eGamma EQ GM_SRGB)
	movaps	xmm5, xmm0
	; store the masks
	movaps	xmm6, xmm0
	cmpleps	xmm6, g_psSRGB_DecRamp
	movaps	xmm7, xmm6
	xorps	xmm7, g_psNOT
	; do the curve branch
	addps	xmm0, g_psSRGB_DecDelta
	mulps	xmm0, g_psSRGB_DecDeltaMul
	INLINE_POW_SSE g_psSRGB_DecGamma
	; do the ramp branch
	mulps	xmm5, g_psSRGB_DecRampMul
	; select the branch that gets for the specified number
	andps	xmm0, xmm7
	andps	xmm5, xmm6
	orps	xmm0, xmm5
 ELSEIF (_eGamma EQ GM_SPECIFY)
	; simple pow
	INLINE_POW_SSE xmm5
 ENDIF

	; restore the signs
	orps		xmm0, xmm4

	; mask out the alpha channel
	andps	xmm0, g_psClearAlpha
	movups	xmm1, _src
	andps	xmm1, g_psKeepAlpha
	orps	xmm0, xmm1

	; store
	movups	_src, xmm0

ENDM


INLINE_DECODE_GAMMA_COMPONENT MACRO _src:REQ, _eGamma:REQ
LOCAL @ramp

	fld	DWORD PTR _src

	mov	ebx, DWORD PTR _src
	test	ebx, ebx
	jns	@F
	fchs
@@:

 IF (_eGamma EQ GM_SRGB)
	fcom	st(5)
	fnstsw	ax
	and	ax, FPU_CONTROL_WORD
	cmp	ax, FPU_LESS
	je	@ramp
	fadd	st(0), st(3)
	fmul	st(0), st(2)
	INLINE_POW st(1)
	jmp	@F
@ramp:
	fmul	st(0), st(4)
@@:
 ELSEIF (_eGamma EQ GM_SPECIFY)
	INLINE_POW st(1)
 ENDIF

	test	ebx, ebx
	jns	@F
	fchs
@@:
	fstp	DWORD PTR _src

ENDM


INLINE_DECODE_GAMMA MACRO _eGamma:REQ
LOCAL @I

	; load the constants
 IF (_eGamma EQ GM_SRGB)
  IFNDEF _SSE
	fld	QWORD PTR g_flSRGB_DecRamp
	fld	QWORD PTR g_flSRGB_DecRampMul
	fld	QWORD PTR g_flSRGB_DecDelta
	fld	QWORD PTR g_flSRGB_DecDeltaMul
	fld	QWORD PTR g_flSRGB_DecGamma
  ELSE
	; nothing
  ENDIF
 ELSEIF (_eGamma EQ GM_SPECIFY)
  IFNDEF _SSE
	fld	(GAMMA PTR [rbx]).flGamma
  ELSE
	movd	xmm5, (GAMMA PTR [rbx]).flGamma
	shufps	xmm5, xmm5, 0
  ENDIF
 ENDIF

align 16
@I:
 IFNDEF _SSE
	INLINE_DECODE_GAMMA_COMPONENT [rdi+00], _eGamma
	INLINE_DECODE_GAMMA_COMPONENT [rdi+04], _eGamma
	INLINE_DECODE_GAMMA_COMPONENT [rdi+08], _eGamma
	;INLINE_DECODE_GAMMA_COMPONENT [rdi+12], GM_LINEAR
 ELSE
	INLINE_DECODE_GAMMA_SSE [rdi], _eGamma
 ENDIF

	; offset to the next pixel and check the counter
	add	rdi, BYTES_OUT_FLOAT
	sub	ecx, 1
	jnz	@I
@@:

 	; pop the constants
 IF (_eGamma EQ GM_SRGB)
  IFNDEF _SSE
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
  ENDIF
 ELSEIF (_eGamma EQ GM_SPECIFY)
  IFNDEF _SSE
	fstp	st(0)
  ENDIF
 ENDIF

ENDM


APIENTRY PLIB_DecodeGamma, USES rsi rdi rbx, pDst:PTR BYTE, iCount:DWORD, pGamma:PTR GAMMA

	mov	rdi, pDst
	mov	ecx, iCount

	mov	ebx, pGamma
	mov	eax, (GAMMA PTR [rbx]).eGamma
	cmp	eax, GM_LINEAR
	je	@quit
	cmp	eax, GM_SRGB
	je	@srgb
	cmp	eax, GM_SPECIFY
	je	@specify
	int	3
@srgb:
	INLINE_DECODE_GAMMA GM_SRGB
	jmp	@quit
@specify:
	INLINE_DECODE_GAMMA GM_SPECIFY
	jmp	@quit
@quit:
	ret

ENDENTRY PLIB_DecodeGamma


APIENTRY PLIB_ConvertGamma, USES rsi rdi rbx, pDst:PTR BYTE, pInGamma:PTR GAMMA, pOutGamma:PTR GAMMA

	ret

ENDENTRY PLIB_ConvertGamma


;//////////////////////////////////////////////////////////////////////


INLINE_EXPAND_RANGE_COMPONENT MACRO _src:REQ

	fld	DWORD PTR _src

	fsub	st(0), st(2)	; - flRangeMin
	fmul	st(0), st(1)	; * (1.0/flRangeLength)

	fstp	DWORD PTR _src

ENDM


APIENTRY PLIB_ExpandRange, USES rsi rdi rbx, pDst:PTR BYTE, iCount:DWORD, flRangeMin:DWORD, flRangeMax:DWORD

	mov	rdi, pDst
	mov	ecx, iCount

; IFNDEF _SSE
	fld	DWORD PTR flRangeMin
	fld	DWORD PTR flRangeMax
	fsub	st(0), st(1)		; get the length
	fld1
	fdivr				; use mul instead of div (this will prepare the value)
align 16
@I:
	INLINE_EXPAND_RANGE_COMPONENT [rdi+00]
	INLINE_EXPAND_RANGE_COMPONENT [rdi+04]
	INLINE_EXPAND_RANGE_COMPONENT [rdi+08]
	;INLINE_EXPAND_RANGE_COMPONENT [rdi+12]

	add	rdi, BYTES_OUT_FLOAT
	sub	ecx, 1
	jnz	@I
@@:
	fstp	st(0)
	fstp	st(0)
; ELSE


; ENDIF

	ret

ENDENTRY PLIB_ExpandRange


INLINE_SCALE_COMPONENT MACRO _src:REQ, _iST

	fld	DWORD PTR _src

	fmul	st(0), st(_iST+1)

	fstp	DWORD PTR _src

ENDM


APIENTRY PLIB_Scale, USES rsi rdi rbx, pDst:PTR BYTE, iCount:DWORD, aflValue:DWORD

	mov	rdi, pDst
	mov	ecx, iCount

; IFNDEF _SSE
	mov	rax, aflValue
	fld	DWORD PTR [rax+00]
	fld	DWORD PTR [rax+04]
	fld	DWORD PTR [rax+08]
	fld	DWORD PTR [rax+12]
align 16
@I:
	INLINE_SCALE_COMPONENT [rdi+00], 3
	INLINE_SCALE_COMPONENT [rdi+04], 2
	INLINE_SCALE_COMPONENT [rdi+08], 1
	INLINE_SCALE_COMPONENT [rdi+12], 0

	add	rdi, BYTES_OUT_FLOAT
	sub	ecx, 1
	jnz	@I
@@:
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
; ELSE


; ENDIF

	ret

ENDENTRY PLIB_Scale



INLINE_EXPONENTIATE_COMPONENT MACRO _src:REQ

	fld	DWORD PTR _src

	INLINE_POW st(1)

	fstp	DWORD PTR _src

ENDM

APIENTRY PLIB_Exponentiate, USES rsi rdi rbx, pDst:PTR BYTE, iCount:DWORD, flValue:DWORD

	mov	rdi, pDst
	mov	ecx, iCount

; IFNDEF _SSE
	fld	DWORD PTR flValue
align 16
@I:
	INLINE_EXPONENTIATE_COMPONENT [rdi+00]
	INLINE_EXPONENTIATE_COMPONENT [rdi+04]
	INLINE_EXPONENTIATE_COMPONENT [rdi+08]
	;INLINE_EXPONENTIATE_COMPONENT [rdi+12]

	add	rdi, BYTES_OUT_FLOAT
	sub	ecx, 1
	jnz	@I
@@:
	fstp	st(0)
; ELSE


; ENDIF

	ret

ENDENTRY PLIB_Exponentiate


; edx is the mask
; TODO: can we optimize the mask lookup?
INLINE_SHUFFLE_COMPONENT_FLOAT MACRO _iCH:REQ

	mov	eax, edx
	shr	eax, _iCH*2
	and	eax, 3
	mov	eax, DWORD PTR [rdi+rax*4]
	mov	DWORD PTR [rsp+_iCH*4], eax

ENDM


; uses a SSE compatible shuffling mask
;  unfortunately we cannot use SSE here because shufps doesn't support register/memory masks
APIENTRY PLIB_ShuffleChannelsFloat, USES rsi rdi rbx, pDst:PTR BYTE, iCount:DWORD, afMask:DWORD

	mov	rdi, pDst
	mov	ecx, iCount

; IFNDEF _SSE
	mov	edx, afMask
	sub	rsp, 16
align 16
@I:
	INLINE_SHUFFLE_COMPONENT_FLOAT CH_0
	INLINE_SHUFFLE_COMPONENT_FLOAT CH_1
	INLINE_SHUFFLE_COMPONENT_FLOAT CH_2
	INLINE_SHUFFLE_COMPONENT_FLOAT CH_3
	mov	eax, DWORD PTR [rsp+CH_0*4]
	mov	DWORD PTR [rdi+CH_0*4], eax
	mov	eax, DWORD PTR [rsp+CH_1*4]
	mov	DWORD PTR [rdi+CH_1*4], eax
	mov	eax, DWORD PTR [rsp+CH_2*4]
	mov	DWORD PTR [rdi+CH_2*4], eax
	mov	eax, DWORD PTR [rsp+CH_3*4]
	mov	DWORD PTR [rdi+CH_3*4], eax

	add	rdi, BYTES_OUT_FLOAT
	sub	ecx, 1
	jnz	@I
@@:
	add	rsp, 16
; ELSE


; ENDIF

	ret

ENDENTRY PLIB_ShuffleChannelsFloat


; edx is the mask
; TODO: can we optimize the mask lookup?
INLINE_SHUFFLE_COMPONENT MACRO _iCH:REQ, _iMaskCH:REQ

	mov	eax, edx
	shr	eax, _iMaskCH*2
	and	eax, 2
	mov	al, BYTE PTR [rdi+rax]
	mov	BYTE PTR [rsp+_iCH], al

ENDM

APIENTRY PLIB_ShuffleChannels, USES rsi rdi rbx, pDst:PTR BYTE, iCount:DWORD, afMask:DWORD

	mov	rdi, pDst
	mov	ecx, iCount

; IFNDEF _SSE
	mov	edx, afMask
	sub	rsp, 4
align 16
@I:
	INLINE_SHUFFLE_COMPONENT CH_R, CH_0
	INLINE_SHUFFLE_COMPONENT CH_G, CH_1
	INLINE_SHUFFLE_COMPONENT CH_B, CH_2
	INLINE_SHUFFLE_COMPONENT CH_A, CH_3
	mov	eax, DWORD PTR [rsp]
	mov	DWORD PTR [rdi], eax

	add	rdi, BYTES_OUT
	sub	ecx, 1
	jnz	@I
@@:
	add	rsp, 4
; ELSE


; ENDIF

	ret

ENDENTRY PLIB_ShuffleChannels


;//////////////////////////////////////////////////////////////////////

INLINE_ANALYZE_RESPONSE_COMPONENT MACRO _src:REQ, _min:REQ, _max:REQ
LOCAL @skip

	fld	DWORD PTR _src

	fcom	st(1)		; bottom limit
	fnstsw	ax
	and	ax, FPU_CONTROL_WORD
	cmp	ax, FPU_LESS
	je	@skip

	fcom	st(2)		; top limit
	fnstsw	ax
	and	ax, FPU_CONTROL_WORD
	cmp	ax, FPU_GREATER
	je	@skip

	fld	DWORD PTR _min
	fcomp
	fnstsw	ax
	and	ax, FPU_CONTROL_WORD
	cmp	ax, FPU_LESS
	je	@F

	fst	DWORD PTR _min
@@:
	fld	DWORD PTR _max
	fcomp
	fnstsw	ax
	and	ax, FPU_CONTROL_WORD
	cmp	ax, FPU_GREATER
	je	@F

	fst	DWORD PTR _max
@@:
@skip:
	fstp	st(0)

ENDM

APIENTRY PLIB_AnalyzeResponse1, USES rsi rdi rbx, pDst:PTR BYTE, iCount:DWORD, aflResponseMin:PTR BYTE, aflResponseMax:PTR BYTE, flTopLimit:DWORD, flBottomLimit:DWORD

	mov	rdi, pDst
	mov	ecx, iCount

; IFNDEF _SSE
	mov	ebx, aflResponseMin
	mov	edx, aflResponseMax
	fld	DWORD PTR flTopLimit
	fld	DWORD PTR flBottomLimit
align 16
@I:
	INLINE_ANALYZE_RESPONSE_COMPONENT [rdi+00], [ebx+00], [edx+00]
	INLINE_ANALYZE_RESPONSE_COMPONENT [rdi+04], [ebx+04], [edx+04]
	INLINE_ANALYZE_RESPONSE_COMPONENT [rdi+08], [ebx+08], [edx+08]
	INLINE_ANALYZE_RESPONSE_COMPONENT [rdi+12], [ebx+12], [edx+12]


	add	rdi, BYTES_OUT_FLOAT
	sub	ecx, 1
	jnz	@I
@@:
	fstp	st(0)
	fstp	st(0)
; ELSE

; ENDIF

	ret

ENDENTRY PLIB_AnalyzeResponse1


APIENTRY PLIB_AnalyzeResponse, USES rsi rdi rbx, buffer:PTR BYTE, prectIn:PTR RECT, prectAnalysis:PTR RECT, aflResponseMin:PTR BYTE, aflResponseMax:PTR BYTE, flTopLimit:DWORD, flBottomLimit:DWORD

	%LOCAL	iAddY			:PTR BYTE
	%LOCAL	iStartX			:DWORD
	%LOCAL	iStartY			:DWORD
	%LOCAL	iCountX			:DWORD
	%LOCAL	iCountY			:DWORD

	mov	rsi, prectIn
	mov	rdi, prectAnalysis

	; determine the intersection of the two rects
	mov	eax, (RECT PTR [rsi]).left
	mov	edx, (RECT PTR [rdi]).left
	cmp	eax, edx
	cmovl	eax, edx
	mov	iStartX, eax
	mov	eax, (RECT PTR [rsi]).right
	mov	edx, (RECT PTR [rdi]).right
	cmp	eax, edx
	cmovg	eax, edx
	sub	eax, iStartX
	jle	@quit
	mov	iCountX, eax
	mov	eax, (RECT PTR [rsi]).top
	mov	edx, (RECT PTR [rdi]).top
	cmp	eax, edx
	cmovl	eax, edx
	mov	iStartY, eax
	mov	eax, (RECT PTR [rsi]).bottom
	mov	edx, (RECT PTR [rdi]).bottom
	cmp	eax, edx
	cmovg	eax, edx
	sub	eax, iStartY
	jle	@quit
	mov	iCountY, eax

	; translate to the buffer space
	mov	eax, (RECT PTR [rsi]).left
	sub	iStartX, eax
	mov	eax, (RECT PTR [rsi]).top
	sub	iStartY, eax

	; init the buffer
	mov	rdi, buffer

	; offsets
	mov	eax, (RECT PTR [rsi]).right	; get the buffer scanline size
	sub	eax, (RECT PTR [rsi]).left	; ~
	imul	eax, BYTES_OUT_FLOAT		; ~
	mov	iAddY, rax
	; offset the buffer
	mov	ecx, iStartY
	imul	eax, ecx			; eax is the size of a scanline
	add	rdi, rax			; offset from the top
	mov	eax, iStartX
	imul	eax, BYTES_OUT_FLOAT
	add	rdi, rax			; offset from the left
	; adjust iAddY by the amount of bytes processed each line
	mov	eax, iCountX
	imul	eax, BYTES_OUT_FLOAT
	sub	iAddY, rax

; IFNDEF _SSE
	mov	ebx, aflResponseMin
	mov	edx, aflResponseMax
	fld	DWORD PTR flTopLimit
	fld	DWORD PTR flBottomLimit

	mov	esi, iCountY
@Y:
	mov	ecx, iCountX
align 16
@X:
	INLINE_ANALYZE_RESPONSE_COMPONENT [rdi+00], [ebx+00], [edx+00]
	INLINE_ANALYZE_RESPONSE_COMPONENT [rdi+04], [ebx+04], [edx+04]
	INLINE_ANALYZE_RESPONSE_COMPONENT [rdi+08], [ebx+08], [edx+08]
	INLINE_ANALYZE_RESPONSE_COMPONENT [rdi+12], [ebx+12], [edx+12]

	add	rdi, BYTES_OUT_FLOAT
	sub	ecx, 1
	jnz	@X
@@:
	add	rdi, iAddY
	sub	esi, 1
	jnz	@Y
@@:
	fstp	st(0)
	fstp	st(0)
; ELSE

; ENDIF

@quit:

	ret

ENDENTRY PLIB_AnalyzeResponse



;//////////////////////////////////////////////////////////////////////

;			name		comps, csize, ctype, func, (pal)

TEMPLATE_PROC_UNPACK_RGB VOID,                 0, 1, DF_UNORM, <INLINE_UNPACK_VOID>

TEMPLATE_PROC_UNPACK_RGB R8_UNORM,             1, 1, DF_UNORM, <INLINE_UNPACK_R>
TEMPLATE_PROC_UNPACK_RGB R8_SNORM,             1, 1, DF_SNORM, <INLINE_UNPACK_R>
TEMPLATE_PROC_UNPACK_RGB R8_UINT,              1, 1, DF_UINT, <INLINE_UNPACK_R>
TEMPLATE_PROC_UNPACK_RGB R8_SINT,              1, 1, DF_SINT, <INLINE_UNPACK_R>
TEMPLATE_PROC_UNPACK_RGB R16_UNORM,            1, 2, DF_UNORM, <INLINE_UNPACK_R>
TEMPLATE_PROC_UNPACK_RGB R16_SNORM,            1, 2, DF_SNORM, <INLINE_UNPACK_R>
TEMPLATE_PROC_UNPACK_RGB R16_UINT,             1, 2, DF_UINT, <INLINE_UNPACK_R>
TEMPLATE_PROC_UNPACK_RGB R16_SINT,             1, 2, DF_SINT, <INLINE_UNPACK_R>
TEMPLATE_PROC_UNPACK_RGB R16_FLOAT,            1, 2, DF_FLOAT, <INLINE_UNPACK_R>
TEMPLATE_PROC_UNPACK_RGB R32_UNORM,            1, 4, DF_UNORM, <INLINE_UNPACK_R>
TEMPLATE_PROC_UNPACK_RGB R32_SNORM,            1, 4, DF_SNORM, <INLINE_UNPACK_R>
TEMPLATE_PROC_UNPACK_RGB R32_UINT,             1, 4, DF_UINT, <INLINE_UNPACK_R>
TEMPLATE_PROC_UNPACK_RGB R32_SINT,             1, 4, DF_SINT, <INLINE_UNPACK_R>
TEMPLATE_PROC_UNPACK_RGB R32_FLOAT,            1, 4, DF_FLOAT, <INLINE_UNPACK_R>

TEMPLATE_PROC_UNPACK_RGB G8_UNORM,             1, 1, DF_UNORM, <INLINE_UNPACK_G>
TEMPLATE_PROC_UNPACK_RGB G8_SNORM,             1, 1, DF_SNORM, <INLINE_UNPACK_G>
TEMPLATE_PROC_UNPACK_RGB G8_UINT,              1, 1, DF_UINT, <INLINE_UNPACK_G>
TEMPLATE_PROC_UNPACK_RGB G8_SINT,              1, 1, DF_SINT, <INLINE_UNPACK_G>
TEMPLATE_PROC_UNPACK_RGB G16_UNORM,            1, 2, DF_UNORM, <INLINE_UNPACK_G>
TEMPLATE_PROC_UNPACK_RGB G16_SNORM,            1, 2, DF_SNORM, <INLINE_UNPACK_G>
TEMPLATE_PROC_UNPACK_RGB G16_UINT,             1, 2, DF_UINT, <INLINE_UNPACK_G>
TEMPLATE_PROC_UNPACK_RGB G16_SINT,             1, 2, DF_SINT, <INLINE_UNPACK_G>
TEMPLATE_PROC_UNPACK_RGB G16_FLOAT,            1, 2, DF_FLOAT, <INLINE_UNPACK_G>
TEMPLATE_PROC_UNPACK_RGB G32_UNORM,            1, 4, DF_UNORM, <INLINE_UNPACK_G>
TEMPLATE_PROC_UNPACK_RGB G32_SNORM,            1, 4, DF_SNORM, <INLINE_UNPACK_G>
TEMPLATE_PROC_UNPACK_RGB G32_UINT,             1, 4, DF_UINT, <INLINE_UNPACK_G>
TEMPLATE_PROC_UNPACK_RGB G32_SINT,             1, 4, DF_SINT, <INLINE_UNPACK_G>
TEMPLATE_PROC_UNPACK_RGB G32_FLOAT,            1, 4, DF_FLOAT, <INLINE_UNPACK_G>

TEMPLATE_PROC_UNPACK_RGB B8_UNORM,             1, 1, DF_UNORM, <INLINE_UNPACK_B>
TEMPLATE_PROC_UNPACK_RGB B8_SNORM,             1, 1, DF_SNORM, <INLINE_UNPACK_B>
TEMPLATE_PROC_UNPACK_RGB B8_UINT,              1, 1, DF_UINT, <INLINE_UNPACK_B>
TEMPLATE_PROC_UNPACK_RGB B8_SINT,              1, 1, DF_SINT, <INLINE_UNPACK_B>
TEMPLATE_PROC_UNPACK_RGB B16_UNORM,            1, 2, DF_UNORM, <INLINE_UNPACK_B>
TEMPLATE_PROC_UNPACK_RGB B16_SNORM,            1, 2, DF_SNORM, <INLINE_UNPACK_B>
TEMPLATE_PROC_UNPACK_RGB B16_UINT,             1, 2, DF_UINT, <INLINE_UNPACK_B>
TEMPLATE_PROC_UNPACK_RGB B16_SINT,             1, 2, DF_SINT, <INLINE_UNPACK_B>
TEMPLATE_PROC_UNPACK_RGB B16_FLOAT,            1, 2, DF_FLOAT, <INLINE_UNPACK_B>
TEMPLATE_PROC_UNPACK_RGB B32_UNORM,            1, 4, DF_UNORM, <INLINE_UNPACK_B>
TEMPLATE_PROC_UNPACK_RGB B32_SNORM,            1, 4, DF_SNORM, <INLINE_UNPACK_B>
TEMPLATE_PROC_UNPACK_RGB B32_UINT,             1, 4, DF_UINT, <INLINE_UNPACK_B>
TEMPLATE_PROC_UNPACK_RGB B32_SINT,             1, 4, DF_SINT, <INLINE_UNPACK_B>
TEMPLATE_PROC_UNPACK_RGB B32_FLOAT,            1, 4, DF_FLOAT, <INLINE_UNPACK_B>

TEMPLATE_PROC_UNPACK_RGB A8_UNORM,             1, 1, DF_UNORM, <INLINE_UNPACK_A>
TEMPLATE_PROC_UNPACK_RGB A8_SNORM,             1, 1, DF_SNORM, <INLINE_UNPACK_A>
TEMPLATE_PROC_UNPACK_RGB A8_UINT,              1, 1, DF_UINT, <INLINE_UNPACK_A>
TEMPLATE_PROC_UNPACK_RGB A8_SINT,              1, 1, DF_SINT, <INLINE_UNPACK_A>
TEMPLATE_PROC_UNPACK_RGB A16_UNORM,            1, 2, DF_UNORM, <INLINE_UNPACK_A>
TEMPLATE_PROC_UNPACK_RGB A16_SNORM,            1, 2, DF_SNORM, <INLINE_UNPACK_A>
TEMPLATE_PROC_UNPACK_RGB A16_UINT,             1, 2, DF_UINT, <INLINE_UNPACK_A>
TEMPLATE_PROC_UNPACK_RGB A16_SINT,             1, 2, DF_SINT, <INLINE_UNPACK_A>
TEMPLATE_PROC_UNPACK_RGB A16_FLOAT,            1, 2, DF_FLOAT, <INLINE_UNPACK_A>
TEMPLATE_PROC_UNPACK_RGB A32_UNORM,            1, 4, DF_UNORM, <INLINE_UNPACK_A>
TEMPLATE_PROC_UNPACK_RGB A32_SNORM,            1, 4, DF_SNORM, <INLINE_UNPACK_A>
TEMPLATE_PROC_UNPACK_RGB A32_UINT,             1, 4, DF_UINT, <INLINE_UNPACK_A>
TEMPLATE_PROC_UNPACK_RGB A32_SINT,             1, 4, DF_SINT, <INLINE_UNPACK_A>
TEMPLATE_PROC_UNPACK_RGB A32_FLOAT,            1, 4, DF_FLOAT, <INLINE_UNPACK_A>

TEMPLATE_PROC_UNPACK_RGB L8_UNORM,             1, 1, DF_UNORM, <INLINE_UNPACK_L>
TEMPLATE_PROC_UNPACK_RGB L8_SNORM,             1, 1, DF_SNORM, <INLINE_UNPACK_L>
TEMPLATE_PROC_UNPACK_RGB L8_UINT,              1, 1, DF_UINT, <INLINE_UNPACK_L>
TEMPLATE_PROC_UNPACK_RGB L8_SINT,              1, 1, DF_SINT, <INLINE_UNPACK_L>
TEMPLATE_PROC_UNPACK_RGB L16_UNORM,            1, 2, DF_UNORM, <INLINE_UNPACK_L>
TEMPLATE_PROC_UNPACK_RGB L16_SNORM,            1, 2, DF_SNORM, <INLINE_UNPACK_L>
TEMPLATE_PROC_UNPACK_RGB L16_UINT,             1, 2, DF_UINT, <INLINE_UNPACK_L>
TEMPLATE_PROC_UNPACK_RGB L16_SINT,             1, 2, DF_SINT, <INLINE_UNPACK_L>
TEMPLATE_PROC_UNPACK_RGB L16_FLOAT,            1, 2, DF_FLOAT, <INLINE_UNPACK_L>
TEMPLATE_PROC_UNPACK_RGB L32_UNORM,            1, 4, DF_UNORM, <INLINE_UNPACK_L>
TEMPLATE_PROC_UNPACK_RGB L32_SNORM,            1, 4, DF_SNORM, <INLINE_UNPACK_L>
TEMPLATE_PROC_UNPACK_RGB L32_UINT,             1, 4, DF_UINT, <INLINE_UNPACK_L>
TEMPLATE_PROC_UNPACK_RGB L32_SINT,             1, 4, DF_SINT, <INLINE_UNPACK_L>
TEMPLATE_PROC_UNPACK_RGB L32_FLOAT,            1, 4, DF_FLOAT, <INLINE_UNPACK_L>

TEMPLATE_PROC_UNPACK_RGB L8A8_UNORM,           2, 1, DF_UNORM, <INLINE_UNPACK_LA>
TEMPLATE_PROC_UNPACK_RGB L8A8_SNORM,           2, 1, DF_SNORM, <INLINE_UNPACK_LA>
TEMPLATE_PROC_UNPACK_RGB L8A8_UINT,            2, 1, DF_UINT, <INLINE_UNPACK_LA>
TEMPLATE_PROC_UNPACK_RGB L8A8_SINT,            2, 1, DF_SINT, <INLINE_UNPACK_LA>
TEMPLATE_PROC_UNPACK_RGB L16A16_UNORM,         2, 2, DF_UNORM, <INLINE_UNPACK_LA>
TEMPLATE_PROC_UNPACK_RGB L16A16_SNORM,         2, 2, DF_SNORM, <INLINE_UNPACK_LA>
TEMPLATE_PROC_UNPACK_RGB L16A16_UINT,          2, 2, DF_UINT, <INLINE_UNPACK_LA>
TEMPLATE_PROC_UNPACK_RGB L16A16_SINT,          2, 2, DF_SINT, <INLINE_UNPACK_LA>
TEMPLATE_PROC_UNPACK_RGB L16A16_FLOAT,         2, 2, DF_FLOAT, <INLINE_UNPACK_LA>
TEMPLATE_PROC_UNPACK_RGB L32A32_UNORM,         2, 4, DF_UNORM, <INLINE_UNPACK_LA>
TEMPLATE_PROC_UNPACK_RGB L32A32_SNORM,         2, 4, DF_SNORM, <INLINE_UNPACK_LA>
TEMPLATE_PROC_UNPACK_RGB L32A32_UINT,          2, 4, DF_UINT, <INLINE_UNPACK_LA>
TEMPLATE_PROC_UNPACK_RGB L32A32_SINT,          2, 4, DF_SINT, <INLINE_UNPACK_LA>
TEMPLATE_PROC_UNPACK_RGB L32A32_FLOAT,         2, 4, DF_FLOAT, <INLINE_UNPACK_LA>

TEMPLATE_PROC_UNPACK_RGB P8_UINT,              1, 1, DF_UINT, <INLINE_UNPACK_P>, 1
TEMPLATE_PROC_UNPACK_RGB P16_UINT,             1, 2, DF_UINT, <INLINE_UNPACK_P>, 1
TEMPLATE_PROC_UNPACK_RGB P32_UINT,             1, 4, DF_UINT, <INLINE_UNPACK_P>, 1

TEMPLATE_PROC_UNPACK_RGB I8_UNORM,             1, 1, DF_UNORM, <INLINE_UNPACK_I>
TEMPLATE_PROC_UNPACK_RGB I8_SNORM,             1, 1, DF_SNORM, <INLINE_UNPACK_I>
TEMPLATE_PROC_UNPACK_RGB I8_UINT,              1, 1, DF_UINT, <INLINE_UNPACK_I>
TEMPLATE_PROC_UNPACK_RGB I8_SINT,              1, 1, DF_SINT, <INLINE_UNPACK_I>
TEMPLATE_PROC_UNPACK_RGB I16_UNORM,            1, 2, DF_UNORM, <INLINE_UNPACK_I>
TEMPLATE_PROC_UNPACK_RGB I16_SNORM,            1, 2, DF_SNORM, <INLINE_UNPACK_I>
TEMPLATE_PROC_UNPACK_RGB I16_UINT,             1, 2, DF_UINT, <INLINE_UNPACK_I>
TEMPLATE_PROC_UNPACK_RGB I16_SINT,             1, 2, DF_SINT, <INLINE_UNPACK_I>
TEMPLATE_PROC_UNPACK_RGB I16_FLOAT,            1, 2, DF_FLOAT, <INLINE_UNPACK_I>
TEMPLATE_PROC_UNPACK_RGB I32_UNORM,            1, 4, DF_UNORM, <INLINE_UNPACK_I>
TEMPLATE_PROC_UNPACK_RGB I32_SNORM,            1, 4, DF_SNORM, <INLINE_UNPACK_I>
TEMPLATE_PROC_UNPACK_RGB I32_UINT,             1, 4, DF_UINT, <INLINE_UNPACK_I>
TEMPLATE_PROC_UNPACK_RGB I32_SINT,             1, 4, DF_SINT, <INLINE_UNPACK_I>
TEMPLATE_PROC_UNPACK_RGB I32_FLOAT,            1, 4, DF_FLOAT, <INLINE_UNPACK_I>

TEMPLATE_PROC_UNPACK_RGB R8G8_UNORM,           2, 1, DF_UNORM, <INLINE_UNPACK_RG>
TEMPLATE_PROC_UNPACK_RGB R8G8_SNORM,           2, 1, DF_SNORM, <INLINE_UNPACK_RG>
TEMPLATE_PROC_UNPACK_RGB R8G8_UINT,            2, 1, DF_UINT, <INLINE_UNPACK_RG>
TEMPLATE_PROC_UNPACK_RGB R8G8_SINT,            2, 1, DF_SINT, <INLINE_UNPACK_RG>
TEMPLATE_PROC_UNPACK_RGB R16G16_UNORM,         2, 2, DF_UNORM, <INLINE_UNPACK_RG>
TEMPLATE_PROC_UNPACK_RGB R16G16_SNORM,         2, 2, DF_SNORM, <INLINE_UNPACK_RG>
TEMPLATE_PROC_UNPACK_RGB R16G16_UINT,          2, 2, DF_UINT, <INLINE_UNPACK_RG>
TEMPLATE_PROC_UNPACK_RGB R16G16_SINT,          2, 2, DF_SINT, <INLINE_UNPACK_RG>
TEMPLATE_PROC_UNPACK_RGB R16G16_FLOAT,         2, 2, DF_FLOAT, <INLINE_UNPACK_RG>
TEMPLATE_PROC_UNPACK_RGB R32G32_UNORM,         2, 4, DF_UNORM, <INLINE_UNPACK_RG>
TEMPLATE_PROC_UNPACK_RGB R32G32_SNORM,         2, 4, DF_SNORM, <INLINE_UNPACK_RG>
TEMPLATE_PROC_UNPACK_RGB R32G32_UINT,          2, 4, DF_UINT, <INLINE_UNPACK_RG>
TEMPLATE_PROC_UNPACK_RGB R32G32_SINT,          2, 4, DF_SINT, <INLINE_UNPACK_RG>
TEMPLATE_PROC_UNPACK_RGB R32G32_FLOAT,         2, 4, DF_FLOAT, <INLINE_UNPACK_RG>

TEMPLATE_PROC_UNPACK_RGB G8R8_UNORM,           2, 1, DF_UNORM, <INLINE_UNPACK_GR>
TEMPLATE_PROC_UNPACK_RGB G8R8_SNORM,           2, 1, DF_SNORM, <INLINE_UNPACK_GR>
TEMPLATE_PROC_UNPACK_RGB G8R8_UINT,            2, 1, DF_UINT, <INLINE_UNPACK_GR>
TEMPLATE_PROC_UNPACK_RGB G8R8_SINT,            2, 1, DF_SINT, <INLINE_UNPACK_GR>
TEMPLATE_PROC_UNPACK_RGB G16R16_UNORM,         2, 2, DF_UNORM, <INLINE_UNPACK_GR>
TEMPLATE_PROC_UNPACK_RGB G16R16_SNORM,         2, 2, DF_SNORM, <INLINE_UNPACK_GR>
TEMPLATE_PROC_UNPACK_RGB G16R16_UINT,          2, 2, DF_UINT, <INLINE_UNPACK_GR>
TEMPLATE_PROC_UNPACK_RGB G16R16_SINT,          2, 2, DF_SINT, <INLINE_UNPACK_GR>
TEMPLATE_PROC_UNPACK_RGB G16R16_FLOAT,         2, 2, DF_FLOAT, <INLINE_UNPACK_GR>
TEMPLATE_PROC_UNPACK_RGB G32R32_UNORM,         2, 4, DF_UNORM, <INLINE_UNPACK_GR>
TEMPLATE_PROC_UNPACK_RGB G32R32_SNORM,         2, 4, DF_SNORM, <INLINE_UNPACK_GR>
TEMPLATE_PROC_UNPACK_RGB G32R32_UINT,          2, 4, DF_UINT, <INLINE_UNPACK_GR>
TEMPLATE_PROC_UNPACK_RGB G32R32_SINT,          2, 4, DF_SINT, <INLINE_UNPACK_GR>
TEMPLATE_PROC_UNPACK_RGB G32R32_FLOAT,         2, 4, DF_FLOAT, <INLINE_UNPACK_GR>

TEMPLATE_PROC_UNPACK_RGB R8G8B8_UNORM,         3, 1, DF_UNORM, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R8G8B8_SNORM,         3, 1, DF_SNORM, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R8G8B8_UINT,          3, 1, DF_UINT, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R8G8B8_SINT,          3, 1, DF_SINT, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R16G16B16_UNORM,      3, 2, DF_UNORM, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R16G16B16_SNORM,      3, 2, DF_SNORM, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R16G16B16_UINT,       3, 2, DF_UINT, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R16G16B16_SINT,       3, 2, DF_SINT, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R16G16B16_FLOAT,      3, 2, DF_FLOAT, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R32G32B32_UNORM,      3, 4, DF_UNORM, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R32G32B32_SNORM,      3, 4, DF_SNORM, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R32G32B32_UINT,       3, 4, DF_UINT, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R32G32B32_SINT,       3, 4, DF_SINT, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R32G32B32_FLOAT,      3, 4, DF_FLOAT, <INLINE_UNPACK_RGBX>

TEMPLATE_PROC_UNPACK_RGB B8G8R8_UNORM,         3, 1, DF_UNORM, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B8G8R8_SNORM,         3, 1, DF_SNORM, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B8G8R8_UINT,          3, 1, DF_UINT, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B8G8R8_SINT,          3, 1, DF_SINT, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B16G16R16_UNORM,      3, 2, DF_UNORM, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B16G16R16_SNORM,      3, 2, DF_SNORM, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B16G16R16_UINT,       3, 2, DF_UINT, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B16G16R16_SINT,       3, 2, DF_SINT, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B16G16R16_FLOAT,      3, 2, DF_FLOAT, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B32G32R32_UNORM,      3, 4, DF_UNORM, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B32G32R32_SNORM,      3, 4, DF_SNORM, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B32G32R32_UINT,       3, 4, DF_UINT, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B32G32R32_SINT,       3, 4, DF_SINT, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B32G32R32_FLOAT,      3, 4, DF_FLOAT, <INLINE_UNPACK_BGRX>

TEMPLATE_PROC_UNPACK_RGB R8G8B8X8_UNORM,       4, 1, DF_UNORM, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R8G8B8X8_SNORM,       4, 1, DF_SNORM, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R8G8B8X8_UINT,        4, 1, DF_UINT, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R8G8B8X8_SINT,        4, 1, DF_SINT, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R16G16B16X16_UNORM,   4, 2, DF_UNORM, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R16G16B16X16_SNORM,   4, 2, DF_SNORM, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R16G16B16X16_UINT,    4, 2, DF_UINT, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R16G16B16X16_SINT,    4, 2, DF_SINT, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R16G16B16X16_FLOAT,   4, 2, DF_FLOAT, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R32G32B32X32_UNORM,   4, 4, DF_UNORM, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R32G32B32X32_SNORM,   4, 4, DF_SNORM, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R32G32B32X32_UINT,    4, 4, DF_UINT, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R32G32B32X32_SINT,    4, 4, DF_SINT, <INLINE_UNPACK_RGBX>
TEMPLATE_PROC_UNPACK_RGB R32G32B32X32_FLOAT,   4, 4, DF_FLOAT, <INLINE_UNPACK_RGBX>

TEMPLATE_PROC_UNPACK_RGB B8G8R8X8_UNORM,       4, 1, DF_UNORM, <INLINE_UNPACK_B8G8R8X8_UNORM>
TEMPLATE_PROC_UNPACK_RGB B8G8R8X8_SNORM,       4, 1, DF_SNORM, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B8G8R8X8_UINT,        4, 1, DF_UINT, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B8G8R8X8_SINT,        4, 1, DF_SINT, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B16G16R16X16_UNORM,   4, 2, DF_UNORM, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B16G16R16X16_SNORM,   4, 2, DF_SNORM, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B16G16R16X16_UINT,    4, 2, DF_UINT, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B16G16R16X16_SINT,    4, 2, DF_SINT, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B16G16R16X16_FLOAT,   4, 2, DF_FLOAT, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B32G32R32X32_UNORM,   4, 4, DF_UNORM, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B32G32R32X32_SNORM,   4, 4, DF_SNORM, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B32G32R32X32_UINT,    4, 4, DF_UINT, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B32G32R32X32_SINT,    4, 4, DF_SINT, <INLINE_UNPACK_BGRX>
TEMPLATE_PROC_UNPACK_RGB B32G32R32X32_FLOAT,   4, 4, DF_FLOAT, <INLINE_UNPACK_BGRX>

TEMPLATE_PROC_UNPACK_RGB R8G8B8A8_UNORM,       4, 1, DF_UNORM, <INLINE_UNPACK_RGBA>
TEMPLATE_PROC_UNPACK_RGB R8G8B8A8_SNORM,       4, 1, DF_SNORM, <INLINE_UNPACK_RGBA>
TEMPLATE_PROC_UNPACK_RGB R8G8B8A8_UINT,        4, 1, DF_UINT, <INLINE_UNPACK_RGBA>
TEMPLATE_PROC_UNPACK_RGB R8G8B8A8_SINT,        4, 1, DF_SINT, <INLINE_UNPACK_RGBA>
TEMPLATE_PROC_UNPACK_RGB R16G16B16A16_UNORM,   4, 2, DF_UNORM, <INLINE_UNPACK_RGBA>
TEMPLATE_PROC_UNPACK_RGB R16G16B16A16_SNORM,   4, 2, DF_SNORM, <INLINE_UNPACK_RGBA>
TEMPLATE_PROC_UNPACK_RGB R16G16B16A16_UINT,    4, 2, DF_UINT, <INLINE_UNPACK_RGBA>
TEMPLATE_PROC_UNPACK_RGB R16G16B16A16_SINT,    4, 2, DF_SINT, <INLINE_UNPACK_RGBA>
TEMPLATE_PROC_UNPACK_RGB R16G16B16A16_FLOAT,   4, 2, DF_FLOAT, <INLINE_UNPACK_RGBA>
TEMPLATE_PROC_UNPACK_RGB R32G32B32A32_UNORM,   4, 4, DF_UNORM, <INLINE_UNPACK_RGBA>
TEMPLATE_PROC_UNPACK_RGB R32G32B32A32_SNORM,   4, 4, DF_SNORM, <INLINE_UNPACK_RGBA>
TEMPLATE_PROC_UNPACK_RGB R32G32B32A32_UINT,    4, 4, DF_UINT, <INLINE_UNPACK_RGBA>
TEMPLATE_PROC_UNPACK_RGB R32G32B32A32_SINT,    4, 4, DF_SINT, <INLINE_UNPACK_RGBA>
TEMPLATE_PROC_UNPACK_RGB R32G32B32A32_FLOAT,   4, 4, DF_FLOAT, <INLINE_UNPACK_RGBA>

TEMPLATE_PROC_UNPACK_RGB B8G8R8A8_UNORM,       4, 1, DF_UNORM, <INLINE_UNPACK_B8G8R8A8_UNORM>
TEMPLATE_PROC_UNPACK_RGB B8G8R8A8_SNORM,       4, 1, DF_SNORM, <INLINE_UNPACK_BGRA>
TEMPLATE_PROC_UNPACK_RGB B8G8R8A8_UINT,        4, 1, DF_UINT, <INLINE_UNPACK_BGRA>
TEMPLATE_PROC_UNPACK_RGB B8G8R8A8_SINT,        4, 1, DF_SINT, <INLINE_UNPACK_BGRA>
TEMPLATE_PROC_UNPACK_RGB B16G16R16A16_UNORM,   4, 2, DF_UNORM, <INLINE_UNPACK_BGRA>
TEMPLATE_PROC_UNPACK_RGB B16G16R16A16_SNORM,   4, 2, DF_SNORM, <INLINE_UNPACK_BGRA>
TEMPLATE_PROC_UNPACK_RGB B16G16R16A16_UINT,    4, 2, DF_UINT, <INLINE_UNPACK_BGRA>
TEMPLATE_PROC_UNPACK_RGB B16G16R16A16_SINT,    4, 2, DF_SINT, <INLINE_UNPACK_BGRA>
TEMPLATE_PROC_UNPACK_RGB B16G16R16A16_FLOAT,   4, 2, DF_FLOAT, <INLINE_UNPACK_BGRA>
TEMPLATE_PROC_UNPACK_RGB B32G32R32A32_UNORM,   4, 4, DF_UNORM, <INLINE_UNPACK_BGRA>
TEMPLATE_PROC_UNPACK_RGB B32G32R32A32_SNORM,   4, 4, DF_SNORM, <INLINE_UNPACK_BGRA>
TEMPLATE_PROC_UNPACK_RGB B32G32R32A32_UINT,    4, 4, DF_UINT, <INLINE_UNPACK_BGRA>
TEMPLATE_PROC_UNPACK_RGB B32G32R32A32_SINT,    4, 4, DF_SINT, <INLINE_UNPACK_BGRA>
TEMPLATE_PROC_UNPACK_RGB B32G32R32A32_FLOAT,   4, 4, DF_FLOAT, <INLINE_UNPACK_BGRA>

; special cases
TEMPLATE_PROC_UNPACK_RGB R9G9B9E5_FLOAT,       4, 1, DF_FLOAT, <INLINE_UNPACK_R9G9B9E5_FLOAT>


;//////////////////////////////////////////////////////////////////////

end

