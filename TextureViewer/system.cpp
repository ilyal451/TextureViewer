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

#include "defs.h"

#include "../shared/utils.h"

#include "memory.h"

#include "system.h"


// some system stuff (exported to the plugins)



//
// File
//


typedef struct FCStream_s
{
	HANDLE hFile;
	FileSize_t fs;
} FCStream_t;


HF SYS_OpenFile( const wchar_t* pszFileName )
{
	HANDLE hFile;
	//int iSize;

	hFile = CreateFile( pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		DWORD iSizeHigh;
		DWORD iSizeLow = GetFileSize( hFile, &iSizeHigh );

		FCStream_t* pstream = (FCStream_t*)malloc( sizeof(FCStream_t) );
		pstream->hFile = hFile;
		pstream->fs.iSizeLow = iSizeLow;
		pstream->fs.iSizeHigh = iSizeHigh;

		return ( HF )pstream;
	}

	return NULL;
}


void SYS_CloseFile( HF h )
{
	FCStream_t* pstream = ( FCStream_t* )h;
	CloseHandle( pstream->hFile );

	free( h );
}


void SYS_GetFileSize( HF h, FileSize_t* pfs )
{
	FCStream_t* pstream = ( FCStream_t* )h;
	pfs->iSizeLow = pstream->fs.iSizeLow;
	pfs->iSizeHigh = pstream->fs.iSizeHigh;
}


void SYS_SetFilePointer( HF h, FileSize_t* pfs )
{
	FCStream_t* pstream = ( FCStream_t* )h;
	LONG iSizeHigh = pfs->iSizeHigh;
	DWORD iSizeLow = pfs->iSizeLow;
	SetFilePointer( pstream->hFile, iSizeLow, &iSizeHigh, FILE_BEGIN );
}


void SYS_GetFilePointer( HF h, FileSize_t* pfs )
{
	FCStream_t* pstream = ( FCStream_t* )h;
	LONG iSizeHigh = 0;
	DWORD iSizeLow = SetFilePointer( pstream->hFile, 0, &iSizeHigh, FILE_CURRENT );
	pfs->iSizeLow = iSizeLow;
	pfs->iSizeHigh = iSizeHigh;
}


unsigned int SYS_ReadFile( HF h, void* buffer, unsigned int iSize )
{
	FCStream_t* pstream = ( FCStream_t* )h;
	DWORD iRead;

	if ( ReadFile( pstream->hFile, buffer, iSize, &iRead, NULL ) )
	{
		return iRead;
	}
	
	return 0;
}


//
// Memory
//

// this is is used to make sure the calling plugins use thread safe calls


void* SYS_AllocMemory( size_t iSize )
{
	return malloc( iSize );
}


void SYS_FreeMemory( void* p )
{
	free( p );
}


// these used to allocate large chunks of memory
// it also automatically aligns the memory by a 16 bytes boundary
// which is required by some SSE subsets

void* SYS_AllocBuffer( size_t iSize )
{
	return VirtualAlloc( NULL, iSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE );
}


void SYS_FreeBuffer( void* p )
{
	VirtualFree( p, 0, MEM_RELEASE );
}


//
// Stream (Sparse) Memory
//


// the sparse memory subsystem is usable on 32-bit systems where you may need
// a large chunk of memroy but the the address space may have been fragmented 
// and so the chunk won't be available; this system breaks the chunk into 
// smaller pieces and provides an API for reading/writing to them as if it be 
// one large chunk
// this won't be much usable on 64-bit systems

HMEM SYS_AllocStreamMemory( size_t iSize )
{
	return ( HMEM )_S_AllocMemory( iSize );
}


void SYS_FreeStreamMemory( HMEM hMem )
{
	_S_FreeMemory( ( mem_t* )hMem );
}


void SYS_ReadStreamMemory( HMEM hMem, size_t iOffset, size_t iSize, void* buffer )
{
	//unsigned char* ab = ( unsigned char* )hMem;
	//memcpy( buffer, &ab[iOffset], iSize );
	_S_ReadMemory( ( mem_t* )hMem, iOffset, iSize, buffer );
}


void SYS_WriteStreamMemory( HMEM hMem, size_t iOffset, size_t iSize, void* buffer )
{
	//unsigned char* ab = ( unsigned char* )hMem;
	//memcpy( &ab[iOffset], buffer, iSize );
	_S_WriteMemory( ( mem_t* )hMem, iOffset, iSize, buffer );
}



//
// Thread Pool
//


PFNTHREADSTARTFUNC g_pfnThreadStart;
void* g_threadParameter;
HANDLE g_hThreadStartEvent;
HANDLE g_hThreadStartedEvent;
HANDLE g_hThreadFinishedEvent;
int g_iThreadCounter;
CRITICAL_SECTION g_csThreadCounter;
int g_nThreads;
HANDLE g_ahThread[MAX_THREADS];
//EVENT g_ahThreadFinishedEvent[MAX_THREADS];


DWORD WINAPI _SYS_ThreadPool( LPVOID param )
{
	int iThread = ( int )param;

	while ( true )
	{
		// wair for input (will release only one thread at a time, then reset)
		WaitForSingleObject( g_hThreadStartEvent, INFINITE );

		// save the pointers
		PFNTHREADSTARTFUNC pfn = g_pfnThreadStart;
		void* parameter = g_threadParameter;

		EnterCriticalSection( &g_csThreadCounter );
		g_iThreadCounter++;
		LeaveCriticalSection( &g_csThreadCounter );

		// tell the main thread it can leave safely
		SetEvent( g_hThreadStartedEvent );

		// call the job procedure
		pfn( iThread, parameter );

		EnterCriticalSection( &g_csThreadCounter );
		g_iThreadCounter--;
		LeaveCriticalSection( &g_csThreadCounter );

		SetEvent( g_hThreadFinishedEvent );
	}

	return 0;
}


int SYS_GetNumThreads( void )
{
	return g_nThreads;
}


void SYS_CallThread( PFNTHREADSTARTFUNC pfn, void* parameter )
{
	while ( true )
	{
		int iCount;

		EnterCriticalSection( &g_csThreadCounter );
		iCount = g_iThreadCounter;
		LeaveCriticalSection( &g_csThreadCounter );
		if ( iCount < g_nThreads )
		{
			break;
		}

		// if all threads are busy wait for any thread to finish
		WaitForSingleObject( g_hThreadFinishedEvent, INFINITE );
	}

	g_pfnThreadStart = pfn;
	g_threadParameter = parameter;

	ResetEvent( g_hThreadStartedEvent );
	SetEvent( g_hThreadStartEvent );

	WaitForSingleObject( g_hThreadStartedEvent, INFINITE );
}


void SYS_WaitForAllThreads( void )
{
	while ( true )
	{
		int iCount;

		EnterCriticalSection( &g_csThreadCounter );
		iCount = g_iThreadCounter;
		LeaveCriticalSection( &g_csThreadCounter );
		if ( iCount == 0 )
			break;
		if ( iCount < 0 )
			__DEBUG_BREAK;
		if ( iCount > g_nThreads )
			__DEBUG_BREAK;

		WaitForSingleObject( g_hThreadFinishedEvent, INFINITE );
	}
}


void SYS_InitThreads( void )
{
	g_hThreadStartEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	g_hThreadStartedEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	g_hThreadFinishedEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

	g_iThreadCounter = 0;
	InitializeCriticalSection( &g_csThreadCounter );

	// determine the number of processors/cores
	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );
	g_nThreads = max( 1, min( MAX_THREADS, sysinfo.dwNumberOfProcessors ) );

	for ( int i = 0; i < g_nThreads; i++ )
	{
		DWORD tid;
		g_ahThread[i] = CreateThread(NULL, 0, _SYS_ThreadPool, (void*)i, 0, &tid);
		//SetThreadPriority(m_hLoaderThread, THREAD_PRIORITY_LOWEST);
	}
}

