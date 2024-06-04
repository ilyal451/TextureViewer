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

#pragma once

#include "../format.h"

HF SYS_OpenFile( const wchar_t* pszFileName );
void SYS_CloseFile( HF h );
void SYS_GetFileSize( HF h, FileSize_t* pfs );
void SYS_SetFilePointer( HF h, FileSize_t* pfs );
void SYS_GetFilePointer( HF h, FileSize_t* pfs );
unsigned int SYS_ReadFile( HF h, void* buffer, unsigned int iSize );

void* SYS_AllocMemory( size_t iSize );
void SYS_FreeMemory( void* p );

void* SYS_AllocBuffer( size_t iSize );
void SYS_FreeBuffer( void* p );

HMEM SYS_AllocStreamMemory( size_t iSize );
void SYS_FreeStreamMemory( HMEM hMem );
void SYS_ReadStreamMemory( HMEM hMem, size_t iOffset, size_t iSize, void* buffer );
void SYS_WriteStreamMemory( HMEM hMem, size_t iOffset, size_t iSize, void* buffer );

#define MAX_THREADS 32
int SYS_GetNumThreads( void );
void SYS_CallThread( PFNTHREADSTARTFUNC pfn, void* parameter );
void SYS_WaitForAllThreads( void );
void SYS_InitThreads( void );
