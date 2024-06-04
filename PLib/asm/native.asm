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

EXTERN g_aiLinear2Linear:WORD
EXTERN g_aiSRGB2Linear:WORD
EXTERN g_aiLinear2SRGB:BYTE

PUBLIC PLIB_ConvertLinear
PUBLIC PLIB_ConvertWithTable
PUBLIC PLIB_ExtractRed
PUBLIC PLIB_ExtractGreen
PUBLIC PLIB_ExtractBlue
PUBLIC PLIB_ExtractAlpha
PUBLIC PLIB_OverlayAlpha


;//////////////////////////////////////////////////////////////////////
.const

;//////////////////////////////////////////////////////////////////////
.code
;//////////////////////////////////////////////////////////////////////


INLINE_SRGB MACRO _iCH

	movzx	eax, BYTE PTR [rdi+_iCH]
	mov	ax, WORD PTR [rdx+rax*2]
	mov	al, BYTE PTR [rsi+rax]
	mov	BYTE PTR [rdi+_iCH], al

ENDM


APIENTRY PLIB_ConvertLinear, USES rsi rdi, pOut:PTR BYTE, iCount:DWORD

	mov	rdi, pOut
	mov	ecx, iCount
	lea	rdx, g_aiLinear2Linear
	lea	rsi, g_aiLinear2SRGB
@next:
	INLINE_SRGB 00
	INLINE_SRGB 01
	INLINE_SRGB 02

	add	rdi, 4
	sub	ecx, 1
	jnz	@next
@quit:
	ret

ENDENTRY PLIB_ConvertLinear

;//////////////////////////////////////////////////////////////////////

INLINE_LOOKUP_TABLE MACRO _iCH

	movzx	eax, BYTE PTR [rdi+_iCH]
	mov	al, BYTE PTR [rsi+rax]
	mov	BYTE PTR [rdi+_iCH], al

ENDM

APIENTRY PLIB_ConvertWithTable, USES rsi rdi, pOut:PTR BYTE, iCount:DWORD, pLT:PTR BYTE

	mov	rdi, pOut
	mov	ecx, iCount
	mov	rsi, pLT
@next:
	INLINE_LOOKUP_TABLE 00
	INLINE_LOOKUP_TABLE 01
	INLINE_LOOKUP_TABLE 02

	add	rdi, 4
	sub	ecx, 1
	jnz	@next
@quit:
	ret

ENDENTRY PLIB_ConvertWithTable

;//////////////////////////////////////////////////////////////////////

APIENTRY PLIB_CopyPixels2, USES rsi rdi, pOut:PTR BYTE, pIn:PTR BYTE, iCount:DWORD

	mov	rdi, pOut
	mov	rsi, pIn
	mov	ecx, iCount

	rep	movsd

	ret

ENDENTRY PLIB_CopyPixels2

;//////////////////////////////////////////////////////////////////////

APIENTRY PLIB_ExtractRed, USES rdi, pOut:PTR BYTE, iCount:DWORD

	mov	rdi, pOut
	mov	ecx, iCount
@next:
	movzx	eax, BYTE PTR [rdi+CH_R]
	mov	BYTE PTR [rdi+CH_G], al
	mov	BYTE PTR [rdi+CH_B], al

	add	rdi, 4
	sub	ecx, 1
	jnz	@next
@quit:
	ret

ENDENTRY PLIB_ExtractRed

;//////////////////////////////////////////////////////////////////////

APIENTRY PLIB_ExtractGreen, USES rdi, pOut:PTR BYTE, iCount:DWORD

	mov	rdi, pOut
	mov	ecx, iCount
@next:
	movzx	eax, BYTE PTR [rdi+CH_G]
	mov	BYTE PTR [rdi+CH_R], al
	mov	BYTE PTR [rdi+CH_B], al

	add	rdi, 4
	sub	ecx, 1
	jnz	@next
@quit:
	ret

ENDENTRY PLIB_ExtractGreen

;//////////////////////////////////////////////////////////////////////

APIENTRY PLIB_ExtractBlue, USES rdi, pOut:PTR BYTE, iCount:DWORD

	mov	rdi, pOut
	mov	ecx, iCount
@next:
	movzx	eax, BYTE PTR [rdi+CH_B]
	mov	BYTE PTR [rdi+CH_R], al
	mov	BYTE PTR [rdi+CH_G], al

	add	rdi, 4
	sub	ecx, 1
	jnz	@next
@quit:
	ret

ENDENTRY PLIB_ExtractBlue

;//////////////////////////////////////////////////////////////////////

APIENTRY PLIB_ExtractAlpha, USES rdi, pOut:PTR BYTE, iCount:DWORD

	mov	rdi, pOut
	mov	ecx, iCount
@next:
	movzx	eax, BYTE PTR [rdi+CH_A]
	mov	BYTE PTR [rdi+CH_R], al
	mov	BYTE PTR [rdi+CH_G], al
	mov	BYTE PTR [rdi+CH_B], al

	add	rdi, 4
	sub	ecx, 1
	jnz	@next
@quit:
	ret

ENDENTRY PLIB_ExtractAlpha

;//////////////////////////////////////////////////////////////////////

; ialpha = 255 - alpha;
; dst = ((dst * ialpha) + (src * alpha)) / 255;

; ialpha = 255 - alpha;
; ialpha = ((ialpha * opacity) / 255);
; alpha = 255 - ialpha;
; dst = ((dst * ialpha) + (src * alpha)) / 255;

INLINE_OVERLAY_ALPHA1 MACRO _iCH

	movzx	eax, BYTE PTR [clrAlpha+_iCH]
	mul	ialpha

	mov	esi, eax

	movzx	eax, BYTE PTR [rdi+_iCH]
	mul	alpha

	add	eax, esi
	div	ebx

	mov	BYTE PTR [rdi+_iCH], al

ENDM

APIENTRY PLIB_OverlayAlpha1, USES rsi rdi rbx, pOut:PTR BYTE, iCount:DWORD, clrAlpha:DWORD, iOpacity:DWORD

	LOCAL	alpha		:DWORD
	LOCAL	ialpha		:DWORD

	mov	rdi, pOut
	mov	ecx, iCount
	mov	ebx, 255

	; check if fully opaque
	cmp	ebx, iOpacity
	je	@fullop

@next:
	mov	eax, ebx
	sub	al, BYTE PTR [rdi+CH_A]		; ialpha = 255 - alpha
	xor	edx, edx
	mul	iOpacity			; ialpha *= iOpacity
	div	ebx				; ialpha /= 255
	mov	edx, ebx
	sub	edx, eax			; alpha = 255 - ialpha
	mov	alpha, edx
	mov	ialpha, eax

	xor	edx, edx

	INLINE_OVERLAY_ALPHA1 00
	INLINE_OVERLAY_ALPHA1 01
	INLINE_OVERLAY_ALPHA1 02

	add	rdi, 4
	sub	ecx, 1
	jnz	@next

	ret

@fullop:
	movzx	edx, BYTE PTR [rdi+CH_A]
	mov	eax, ebx
	sub	eax, edx			; ialpha = 255 - alpha
	mov	alpha, edx
	mov	ialpha, eax

	xor	edx, edx

	INLINE_OVERLAY_ALPHA1 00
	INLINE_OVERLAY_ALPHA1 01
	INLINE_OVERLAY_ALPHA1 02

	add	rdi, 4
	sub	ecx, 1
	jnz	@fullop

	ret

ENDENTRY PLIB_OverlayAlpha1


;//////////////////////////////////////////////////////////////////////

; dst = (alpha*(src-dst)+dst*255)/255
; this is a bit incorrect but faster version (it divides by 256 instead of 255)
INLINE_OVERLAY_ALPHANG MACRO _iCH

	movzx	eax, BYTE PTR [rdi+_iCH]
	movzx	esi, BYTE PTR [clrAlpha+_iCH]
	sub	eax, esi
	mul	ebx		; ebx is alpha
	shl	esi, 8
	add	eax, esi
	shr	eax, 8
	mov	BYTE PTR [rdi+_iCH], al
ENDM

; gamma version
; dst = (alpha*(src-dst)+dst*255)/255
INLINE_OVERLAY_ALPHA MACRO _iCH

	movzx	eax, BYTE PTR [rdi+_iCH]
	movzx	eax, WORD PTR [g_aiSRGB2Linear+rax*2]
	movzx	esi, BYTE PTR [clrAlpha+_iCH]
	movzx	esi, WORD PTR [g_aiSRGB2Linear+rsi*2]	; TODO: can be optimized
	sub	eax, esi
	mul	ebx		; ebx is alpha
	shl	esi, 8
	add	eax, esi
	shr	eax, 8
	movzx	eax, BYTE PTR [g_aiLinear2SRGB+rax]
	mov	BYTE PTR [rdi+_iCH], al
ENDM


APIENTRY PLIB_OverlayAlpha, USES rsi rdi rbx, pOut:PTR BYTE, iCount:DWORD, clrAlpha:DWORD, iOpacity:DWORD

	LOCAL opacitylt[256]:BYTE

	; fill the opacity table
	; XXX: should we even use opacity at all?
	movzx	esi, BYTE PTR iOpacity
	lea	rdi, opacitylt
	mov	ebx, 255
	xor	ecx, ecx
@@:
	xor	edx, edx
	mov	eax, ebx
	sub	eax, ecx
	mul	esi
	div	ebx
	mov	edx, ebx
	sub	edx, eax
	mov	BYTE PTR[rdi], dl
	add	edi, 1
	add	ecx, 1
	cmp	ecx, 256
	jb	@B
@@:
	; begin processing
	mov	rdi, pOut
	mov	ecx, iCount
	xor	edx, edx
	xor	ebx, ebx
@next:
	movzx	ebx, BYTE PTR [rdi+CH_A]
	mov	bl, BYTE PTR [opacitylt+rbx]
	cmp	bl, 255
	je	@skip		; fully opaque, unchanged
	test	ebx, ebx
	jz	@fsrc		; fully transparent, copy the color

	INLINE_OVERLAY_ALPHA 00
	INLINE_OVERLAY_ALPHA 01
	INLINE_OVERLAY_ALPHA 02
@skip:
	add	rdi, 4
	sub	ecx, 1
	jnz	@next
	jmp	quit
@fsrc:
	mov	eax, [clrAlpha]
	mov	[rdi], eax

	add	rdi, 4
	sub	ecx, 1
	jnz	@next
quit:
	ret

ENDENTRY PLIB_OverlayAlpha

end
;//////////////////////////////////////////////////////////////////////
