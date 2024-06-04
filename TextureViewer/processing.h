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

#ifndef _FILE_SETTINGS_H
#define _FILE_SETTINGS_H


class CFileSettings
{
public:

	CFileSettings( HWND hWnd );
	~CFileSettings();

	void Destroy( void );
	HWND GetWnd( void );
	void EnsureVisible( RECT* prect );
	
	void SetFile( CFile* pFile );

	void SetPickingColor( bool bPicking );
	void DoAnalysis( Rect_t* prect );

	void ApplyFileSettings( void );

	static BOOL CALLBACK FileOptsDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK ScrollPanelWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

private:

	HWND m_hWnd;
	HWND m_hScrollPanel;
	HWND m_hFileOpts;
	int m_iHeight;
	int m_iFileOptsHeight;
	int m_yScroll;
	bool m_bUseChannelFormat;

	CFile* m_pFile;
	bool m_bValidFile;
	KEYVALUEBUFFER m_hkvbufSettings;
	ProcessingOptions_t* m_pfo;
	ChannelInfo_t m_aci[NUM_CHANNELS];

	void Load( void );
	void Store( void );

	void FileOpts_Init( HWND hWnd );
	void FileOpts_TrackControl( HWND hWnd, HWND hWndCtrl );
	void FileOpts_Command( HWND hWnd, int iCmd, int iControl, HWND hWndCtrl );
	void FillChannelInfo( ChannelInfo_t* pci, int iCtrlType, int iCtrlSize, int iCtrlSizeO );
	void UpdateChannelInfo( void );
	void FillResponseValue( int iChannel, float fl, int iCtrl, bool bValid );
	void FillResponseValues( int iChannel, float flMin, float flMax, int iCtrlMin, int iCtrlMax, bool bMin, bool bMax );
	void FillShuffleMask( unsigned int afMask );
	unsigned int GrabShuffleMask( void );
	void FillOriginalGamma( void );
	void Update( void );
	void UpdateReset( void );
	void UpdateRange( void );
	void ResetRange( void );
	void UpdateShuffle( void );
	void ResetShuffle( void );
	void UpdateRotate( void );
	void ResetRotate( void );
	void UpdateGamma( void );
	void ResetGamma( void );
	bool FOverrideGamma( void );
	void UpdateOverrideGamma( void );
	void UpdateGammaChoice( void );
	void OnScrollPanelSize( int fwSizeType, int nWidth, int nHeight );
	void OnScrollPanelScroll( int nScrollCode, int nPos, HWND hWndScrollBar );
	void OnSize( int fwSizeType, int nWidth, int nHeight );
	void OnCommand( int iCmd, int iControl, HWND hWndCtrl );
	void UpdateScroll( void );
	void Close( void );
};


extern CFileSettings* g_pFileSettings;

extern int g_iProcessingLeft;
extern int g_iProcessingTop;
extern int g_iProcessingWidth;
extern int g_iProcessingHeight;

extern float g_flAnalysisTopLimit;
extern float g_flAnalysisBottomLimit;
extern int g_eAnalysisSource;

void InitFileSettings(HWND hWndParent);

#endif // _FILE_SETTINGS_H

