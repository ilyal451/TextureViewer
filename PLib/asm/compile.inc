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

INCLUDE config.inc

; check the mode
IFDEF rax ; 64-bit mode
 _X64 equ 1
 PROTODEF equ PROTO
ELSE ; 32-bit mode
.686
 IFDEF _SSE
.XMM
 ENDIF
.MODEL FLAT, C
rax equ eax
rbx equ ebx
rcx equ ecx
rdx equ edx
rsi equ esi
rdi equ edi
rsp equ esp
rbp equ ebp
movsxd equ mov
movzxd equ mov
 PROTODEF equ PROTO NEAR C
ENDIF

OPTION CASEMAP:NONE

IFDEF _SSE
 IFNDEF mm0
  IFNDEF _X64
mm0 equ MM0
mm1 equ MM1
mm2 equ MM2
mm3 equ MM3
mm4 equ MM4
mm5 equ MM5
mm6 equ MM6
mm7 equ MM7
xmm0 equ XMM0
xmm1 equ XMM1
xmm2 equ XMM2
xmm3 equ XMM3
xmm4 equ XMM4
xmm5 equ XMM5
xmm6 equ XMM6
xmm7 equ XMM7
  ENDIF
 ENDIF
ENDIF
