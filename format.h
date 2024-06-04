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

#ifndef _FORMAT_H
#define _FORMAT_H

#include "types.h"
 
#define FORMAT_INTERFACE_VERSION 2

// image flags
#define IMAGE_HAS_ALPHA 0x01 // the alpha channel contains meaningful data
#define IMAGE_GRAYSCALE 0x02 // the RGB channels of the image contain the same values and the image is considered to have no color (luminance only)
#define IMAGE_PREMULTIPLIED_ALPHA 0x10 // the color is premultiplied by alpha (DXT2, DXT4)
										// this flag is required to correctly display DXT2, DXT4, and other formats the color of which is premultiplied by the alpha

typedef void *SETTINGS;

typedef void *KEYVALUEKEY;
typedef void *KEYVALUEBUFFER;

typedef void *HF;
typedef struct FileSize
{
	union
	{
		struct
		{
			unsigned int iSizeLow;
			unsigned int iSizeHigh;
		};
		unsigned __int64 iSize;
	};
} FileSize_t;

typedef void *HMEM;

typedef void ( *PFNTHREADSTARTFUNC ) ( int iThread, void* parameter );


// plugin defined
typedef void *H_FILE;
typedef void *H_SET;


enum StreamType
{
	ST_IMAGE = 0,
	ST_PALETTE,
};
// don't use more than this
#define MAX_STREAMS 4

/*
class ISet
{
	virtual int GetImageArraySize( void ) = 0;
	// ...
};

class IFile
{
	virtual int GetNumSets( void ) = 0;
	virtual ISet* LoadSet( int iSet ) = 0;
	virtual void FreeSet( ISet* pSet ) = 0;
};

class IFormat
{
	virtual IFile* LoadFile( IStream* pStream, KEYVALUEBUFFER hkvbufSettings, KEYVALUEBUFFER hkvbufMetadata ) = 0;
	virtual void FreeFile( IFile* pFile ) = 0;
};
*/

typedef struct FormatFuncs_s
{
	H_FILE			( *pfnLoadFile )				( HF stream, KEYVALUEBUFFER hkvbufSettings, KEYVALUEBUFFER hkvbufMetadata );
	void			( *pfnFreeFile )				( H_FILE hFile );
	int				( *pfnGetNumImageSets )			( H_FILE hFile );
	H_SET			( *pfnLoadSet )					( H_FILE hFile, int iSet );
	void			( *pfnFreeSet )					( H_SET hSet );
	int				( *pfnGetArraySize )			( H_SET hSet );
	int				( *pfnGetNumMIPMaps )			( H_SET hSet );
	const char*		( *pfnGetFormatStr )			( H_SET hSet );
	void			( *pfnGetInputGamma )			( H_SET hSet, Gamma_t* pcs );
	void			( *pfnGetOutputGamma )			( H_SET hSet, Gamma_t* pcs );
	int				( *pfnGetImageFlags )			( H_SET hSet );
	int				( *pfnGetOriginalBitDepth )		( H_SET hSet, int iChannel );
	int				( *pfnGetImageWidth )			( H_SET hSet, int iMIPMap );
	int				( *pfnGetImageHeight )			( H_SET hSet, int iMIPMap );
	int				( *pfnGetImageDepth )			( H_SET hSet, int iMIPMap );
	int				( *pfnGetNumPaletteColors )		( H_SET hSet );
	int				( *pfnGetNumStreams )			( H_SET hSet, int eStreamType );
	void			( *pfnGetStreamPixelFormat )	( H_SET hSet, int eStreamType, int iStream, PixelFormat_t* ppf );
	HMEM			( *pfnGetStreamBits )			( H_SET hSet, int eStreamType, int iStream );

} FormatFuncs_t;


typedef struct SystemFuncs_s
{
	// thread-safe file
	HF ( *pfnOpenFile )( const wchar_t* pszFileName );
	void ( *pfnCloseFile )( HF h );
	void ( *pfnGetFileSize )( HF h, FileSize_t* pfsSize );
	void ( *pfnSetFilePointer )( HF h, FileSize_t* pfsPos );
	void ( *pfnGetFilePointer )( HF h, FileSize_t* pfsPos );
	unsigned int ( *pfnReadFile )( HF h, void* buffer, unsigned int iSize );

	// thread-safe memory (if you're not using these make sure your plugin uses the multithreaded version of the runtime)
	void* ( *pfnAllocMemory )( size_t iSize );
	void ( *pfnFreeMemory )( void* p );

	// sparse (moveable) memory
	HMEM ( *pfnAllocStreamMemory )( size_t iSize );
	void ( *pfnFreeStreamMemory )( HMEM hMem );
	void ( *pfnReadStreamMemory )( HMEM hMem, size_t Offset, size_t iSize, void* buffer );
	void ( *pfnWriteStreamMemory )( HMEM hMem, size_t Offset, size_t iSize, void* buffer );

	// thread pool
	int ( *pfnGetNumThreads )( void );
	void ( *pfnCallThread )( PFNTHREADSTARTFUNC pfn, void* parameter );
	void ( *pfnWaitForAllThreads )( void );
	// thread pool example:
	// while ( any data left )
	// {
	// 	CallThread( ProcessDataFunc, a portion of data );
	// 	prepare the next portion of data
	// }
	// WaitForAllThreadsToComplete();
	// grab the results here

	// keyvalue (UNUSED?)
	KEYVALUEKEY ( *pfnFindKey )( KEYVALUEBUFFER hkvbuf, const char* pszKey );
	KEYVALUEKEY ( *pfnAddKey )( KEYVALUEBUFFER hkvbuf, const char* pszKey, const char* pszValueInit );
	const char* ( *pfnGetKeyValue )( KEYVALUEKEY hkvkey );
	void ( *pfnSetKeyValue )( KEYVALUEKEY hkvkey, const char* pszValue );

	// plib.dll client side interface
	void* ( *pfnGetPLibClientFuncs )( void );

} SystemFuncs_t;


// library export prototypes
typedef int ( *PFNGETFORMATINTERFACEVERSION )( int iCurrentVersion );
typedef const FormatFuncs_t* ( *PFNLOADFORMATDLL )( const SystemFuncs_t* psystem, void* hWndMain );


#endif //_FORMAT_H

