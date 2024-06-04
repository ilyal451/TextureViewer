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

#include "format.h"

#define SHUFFLE_MASK( r, g, b, a ) ( ( r << 0 ) | ( g << 2 ) | ( b << 4 ) | ( a << 6 ) )
#define DEFAULT_SHUFFLE_MASK SHUFFLE_MASK( 0, 1, 2, 3 )

enum RotateTypes
{
	RT_0 = 0,
	RT_90CW,
	RT_90CCW,
	RT_180,
	NUM_ROTATE_TYPES
};

typedef struct ProcessingOptions_s
{
	bool bEnableProcessing;

	unsigned char afShuffleMask;

	bool bOverrideGamma;

	float flRangeMin; // 0.0 initially
	float flRangeMax; // 1.0 initially

	unsigned char eRotateType;
	bool bFlipWidth;
	bool bFlipHeight;

	Gamma_t gmInputGamma;
	Gamma_t gmOutputGamma;

} ProcessingOptions_t;


//#define NUM_COMPONENTS 4

typedef struct ImagePParams_s
{
	ChannelInfo_t aci[NUM_CHANNELS]; // the flags member of this structure is used to create the output mask, all other members are just informary
	int flags; // image flags
	int iImageWidth;
	int iImageHeight;
} ImageParams_t;

enum AnalysisSourceType_e
{
	ANALYZE_INPUT = 0,
	ANALYZE_OUTPUT,
	NUM_ANALYSIS_SOURCE_TYPES
};

class CFile
{
public:

	CFile( const wchar_t* pszFileName, CFormat* pFormat );
	~CFile();

	const wchar_t* GetFileName( void ) { return m_pszFileName; }
	int GetId( void );
	void SetId( int iId );
	void Load( wchar_t* pszSourcePath ); // each context has it's own source path (make files contain the full path?)
	void Unload( void );
	bool IsLoaded( void );		// indicates whether Load has been called on this file; the file may be invalid afterwards as well; a call to Load loads not only the file but also the texture and the mipmap
	bool IsValidFile( void );	// if the format was able to load the file
	int GetNumTextures( void );
	void SetTexture( int iTexture );
	int GetTexture( void );
	int GetNumMIPMaps( void );
	void SetMIPMap( int iMIPMap );
	int GetMIPMap( void );
	int GetNumSlices( void );
	void SetSlice( int iSlice );
	int GetSlice( void );
	const char* GetImageFormatStr( void );
	void GetImageInputGamma( Gamma_t* pcs );
	void GetImageOutputGamma( Gamma_t* pcs );
	int GetImageFlags( void );
	int GetNumPaletteColors( void );
	void GetPaletteData( void* buffer );
	int GetImageWidth( void ); // these return the original width/height specified by the plugin
	int GetImageHeight( void ); //
	int GetRotatedImageWidth( ProcessingOptions_t& fo ); // these return the cropped and rotated width/height (this is the final one)
	int GetRotatedImageHeight( ProcessingOptions_t& fo ); //
	int GetMaxBufferWidth(); // for the uncropped/unrotated
	int GetMaxBufferHeight(); //
	//int GetMaxRotatedBufferWidth( ProcessingOptions_t& fo ); // for the cropped/rotated
	//int GetMaxRotatedBufferHeight( ProcessingOptions_t& fo ); //
	bool IsValidImage( void );
	void GetImageData( void* buffer );

	// hm... this is the plugin's settings buffer... (UNDONE really)
	KEYVALUEBUFFER GetSettingsBuffer( void ); // GetPluginSettingsBuffer
	KEYVALUEBUFFER GetMetadataBuffer( void );

	void SetMarked( bool bMarked );
	bool GetMarked( void );
	
	void Read( SETTINGS settings );
	void Write( SETTINGS settings );

	void ReadChannelInfo( ChannelInfo_t* aci );
	void DoAnalysis( Rect_t* prect, int flags, float* aflMin, float* aflMax );

private:

	wchar_t* m_pszFileName;
	CFormat* m_pFormat;
	
	// unused currently
	KEYVALUEBUFFER m_hkvbufSettings;
	KEYVALUEBUFFER m_hkvbufMetadata;

	bool m_bLoaded;
	H_FILE m_hFile;

	short m_iId; // the id linked to the context file list (should be updated if file has moved)

	short m_iSet;
	H_SET m_hSet;

	short m_iTexture;
	short m_iMIPMap;
	short m_iSlice;
	int GetImageOffsetPixels( void );
};


#define MAX_CONTEXT_FILES 4096
#define NUM_PRECACHE_FILES 4

class CContext
{
public:

	CContext( void );
	~CContext();

	bool SetSourcePath( const wchar_t* pszPath );
	const wchar_t* GetSourcePath( void );
	void PopulateSourceList( const wchar_t* pszFilter );
	void Reset( void );
	int AddFile( const wchar_t* pszFileName );
	void RemoveFile( int iFile );
	int GetNumFiles( void );
	const wchar_t* GetFileName( int iFile );
	void SetFile( int iFile );
	int GetFile( void );
	CFile* GetFile( int iFile );

	void LoaderThread( void );

	void UpdateInputParams( bool bForeground, bool bUpdateImage );

	const ImageParams_t* GetOutputParams( void );

	void Load( const wchar_t* pszFileName );
	void Save( const wchar_t* pszFileName );

private:

	// files
	wchar_t* m_pszSourcePath;
	int m_nFiles;
	CFile** m_apFile;

	// loader state
	int m_iFile;
	int m_iNewFile;
	int m_aiIndices[NUM_PRECACHE_FILES];

	// the image params may change if the processing was applied
	// these are the image params after the processing
	ImageParams_t m_outputParams;

	void LoadFile( int iFile );
	void FreeFile( int iFile );
	void CancelJob( void );

};


extern CContext* g_pContext;

void InitImageLoader( void );
void InitContext( void );

