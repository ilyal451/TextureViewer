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
#include "globals.h"
#include <stdio.h>

#include "../shared/utils.h"
#include "../shared/plib.h"

#include "../format.h"

#include "float.h"

#include "format.h"
#include "context.h"
#include "viewer.h"

#include "processing.h"

#include "resource.h"

// the processing window
// the name FileSettings is a leagacy and can be changed


CFileSettings* g_pFileSettings;

extern ProcessingOptions_t g_fo;

HWND g_hFileOpts; // required to make the tabstob work

int g_iProcessingLeft;
int g_iProcessingTop;
int g_iProcessingWidth;
int g_iProcessingHeight;

float g_flAnalysisTopLimit = 10000;
float g_flAnalysisBottomLimit = -10000;
int g_eAnalysisSource = ANALYZE_INPUT;


void CFileSettings::FileOpts_Init( HWND hWnd )
{
	g_hFileOpts = hWnd;

	// analysis sources
	static const wchar_t* apszSource[NUM_ANALYSIS_SOURCE_TYPES] =
	{
		L"Input",
		L"Output",
	};
	for ( int i = 0; i < NUM_ANALYSIS_SOURCE_TYPES; i++ )
	{
		SendDlgItemMessage( hWnd, IDC_FILEOPT_ANALYSIS_SOURCE, CB_ADDSTRING, NULL, (LPARAM)apszSource[i] );
	}

	// shuffle masks
	static const wchar_t* apszShuffleChannel[NUM_CHANNELS] =
	{
		L"R",
		L"G",
		L"B",
		L"A",
	};
	for ( int i = 0; i < NUM_CHANNELS; i++ )
	{
		SendDlgItemMessage( hWnd, IDC_FILEOPT_SHUFFLE_R, CB_ADDSTRING, NULL, (LPARAM)apszShuffleChannel[i] );
		SendDlgItemMessage( hWnd, IDC_FILEOPT_SHUFFLE_G, CB_ADDSTRING, NULL, (LPARAM)apszShuffleChannel[i] );
		SendDlgItemMessage( hWnd, IDC_FILEOPT_SHUFFLE_B, CB_ADDSTRING, NULL, (LPARAM)apszShuffleChannel[i] );
		SendDlgItemMessage( hWnd, IDC_FILEOPT_SHUFFLE_A, CB_ADDSTRING, NULL, (LPARAM)apszShuffleChannel[i] );
	}

	static const wchar_t* apszRotateOpt[NUM_ROTATE_TYPES] =
	{
		L"0\x00B0",
		L"90\x00B0 CW",
		L"90\x00B0 CCW",
		L"180\x00B0",
	};

	// add rotate options
	for ( int i = 0; i < NUM_ROTATE_TYPES; i++ )
	{
		SendDlgItemMessage( hWnd, IDC_FILEOPT_ROTATE, CB_ADDSTRING, NULL, (LPARAM)apszRotateOpt[i] );
	}

	static const wchar_t* apszGammaOpt[NUM_GAMMA_OPTIONS] =
	{
		L"Linear",
		L"sRGB"
	};

	// add gamma options
	for ( int i = 0; i < 2; i++ )
	{
		SendDlgItemMessage( hWnd, IDC_FILEOPT_GAMMA_IN, CB_ADDSTRING, NULL, (LPARAM)apszGammaOpt[i] );
		SendDlgItemMessage( hWnd, IDC_FILEOPT_GAMMA_OUT, CB_ADDSTRING, NULL, (LPARAM)apszGammaOpt[i] );
	}
}


void CFileSettings::FileOpts_TrackControl( HWND hWnd, HWND hWndCtrl )
{
	RECT rect;
	GetWindowRect( hWndCtrl, &rect );
	MapWindowPoints( NULL, hWnd, ( POINT* )&rect, 2 );
	EnsureVisible( &rect );
}


void CFileSettings::FileOpts_Command( HWND hWnd, int iCmd, int iControl, HWND hWndCtrl )
{
	wchar_t buf[64];

	if ( iCmd == BN_CLICKED )
	{
		switch ( iControl )
		{
			case IDC_FILEOPT_ANALYSIS_PICK:
				{
					bool bCheck = SendDlgItemMessage( hWnd, IDC_FILEOPT_ANALYSIS_PICK, BM_GETCHECK, 0, NULL ) != 0;
					SetPickingColor( !bCheck );
				}
				break;
			case IDC_FILEOPT_ANALYSIS_ANALYZE:
				DoAnalysis( NULL );
				break;
			case IDC_FILEOPT_OVERRIDE_GAMMA:
				UpdateOverrideGamma();
				break;
				/*
			case IDC_FILEOPT_UPDATE:
				Update();
				break;
			case IDC_FILEOPT_RESET:
				Reset();
				break;
			case IDC_FILEOPT_RESET_ALL:
				ResetAll();
				break;
				*/
			case IDC_FILEOPT_RANGE_UPDATE:
				UpdateRange();
				break;
			case IDC_FILEOPT_RANGE_RESET:
				ResetRange();
				break;
			case IDC_FILEOPT_SHUFFLE_UPDATE:
				UpdateShuffle();
				break;
			case IDC_FILEOPT_SHUFFLE_RESET:
				ResetShuffle();
				break;
			case IDC_FILEOPT_ROTATE_UPDATE:
				UpdateRotate();
				break;
			case IDC_FILEOPT_ROTATE_RESET:
				ResetRotate();
				break;
			case IDC_FILEOPT_GAMMA_UPDATE:
				UpdateGamma();
				break;
			case IDC_FILEOPT_GAMMA_RESET2:
				ResetGamma();
				break;
			case IDC_FILEOPT_ENABLE_PROCESSING:
				Update();
				break;
		}
	}

	if ( iCmd == CBN_SELCHANGE )
	{
		switch ( iControl )
		{
		case IDC_FILEOPT_GAMMA_IN:
			UpdateGammaChoice();
			break;
		case IDC_FILEOPT_GAMMA_OUT:
			UpdateGammaChoice();
			break;
		}
	}

	// track on tabstops
	if ( iCmd == EN_SETFOCUS )
	{
		GetClassName( hWndCtrl, buf, 64 );
		if ( FStrEqW( buf, L"EDIT" ) )
		{
			FileOpts_TrackControl( hWnd, hWndCtrl );
		}
	}
	if ( iCmd == BN_SETFOCUS )
	{
		GetClassName( hWndCtrl, buf, 64 );
		if ( FStrEqW( buf, L"BUTTON" ) )
		{
			FileOpts_TrackControl( hWnd, hWndCtrl );
		}
	}
	if ( iCmd == CBN_SETFOCUS )
	{
		GetClassName( hWndCtrl, buf, 64 );
		if ( FStrEqW( buf, L"COMBOBOX" ) )
		{
			FileOpts_TrackControl( hWnd, hWndCtrl );
		}
	}
}


BOOL CALLBACK CFileSettings::FileOptsDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch ( uMsg )
	{
		case WM_INITDIALOG:
			g_pFileSettings->FileOpts_Init( hWnd );
			break;

		case WM_COMMAND:
			g_pFileSettings->FileOpts_Command( hWnd, HIWORD(wParam), LOWORD(wParam), ( HWND )lParam );
			break;

		default:
			return 0;
	}

	return 1;
}




LRESULT CALLBACK CFileSettings::ScrollPanelWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch (uMsg)
	{
    case WM_CREATE:
        break;

    case WM_DESTROY:
        break;

	case WM_SIZE:
		g_pFileSettings->OnScrollPanelSize( ( int )wParam, LOWORD( lParam ), HIWORD( lParam ) );
		break;

	case WM_VSCROLL:
		g_pFileSettings->OnScrollPanelScroll( LOWORD( wParam ), HIWORD( wParam ), ( HWND )lParam );
		break;

	default:
		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	}

	return 0;
}


CFileSettings::CFileSettings( HWND hWnd )
{
	g_pFileSettings = this;

	m_hWnd = hWnd;

	m_pFile = NULL;
	m_bValidFile = false;
	m_hkvbufSettings = NULL;
	m_pfo = NULL;

	const wchar_t* pszWindowClass = L"ScrollPanelClassEx";
	WNDCLASSEX wc = { sizeof( WNDCLASSEX ), 0, ScrollPanelWndProc, 0, sizeof( void* ), g_hInst, NULL, LoadCursor( NULL, IDC_ARROW ), GetSysColorBrush( COLOR_BTNFACE ), NULL, pszWindowClass, 0 };
	RegisterClassEx( &wc );
	m_hScrollPanel = CreateWindowEx( WS_EX_STATICEDGE, pszWindowClass, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hWnd, NULL, g_hInst, NULL );
	m_hFileOpts = CreateDialog( g_hInst, MAKEINTRESOURCE( IDD_FILEOPTS ), m_hScrollPanel, FileOptsDlgProc );

	HFONT hFont = ( HFONT )GetStockObject( DEFAULT_GUI_FONT );
	//m_hEnableProcessingCheckBox = CreateWindow( L"BUTTON", L"Enable processing", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 0, 0, 0, 0, hWnd, ( HMENU )IDC_FILEOPT_ENABLE_PROCESSING, g_hInst, NULL );
	//SendMessage( m_hEnableProcessingCheckBox, WM_SETFONT, ( WPARAM )hFont, NULL );
	//m_hUpdateBtn = CreateWindow( L"BUTTON", L"Apply", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 0, 0, 0, 0, hWnd, ( HMENU )IDC_FILEOPT_UPDATE, g_hInst, NULL );
	//SendMessage( m_hUpdateBtn, WM_SETFONT, ( WPARAM )hFont, NULL );
	//m_hResetBtn = CreateWindow( L"BUTTON", L"Reset", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hWnd, ( HMENU )IDC_FILEOPT_RESET, g_hInst, NULL );
	//SendMessage( m_hResetBtn, WM_SETFONT, ( WPARAM )hFont, NULL );
	//m_hResetAllBtn = CreateWindow( L"BUTTON", L"Reset All", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hWnd, ( HMENU )IDC_FILEOPT_RESET_ALL, g_hInst, NULL );
	//SendMessage( m_hResetAllBtn, WM_SETFONT, ( WPARAM )hFont, NULL );

	RECT rect;
	GetWindowRect( m_hFileOpts, &rect );
	m_iFileOptsHeight = rect.bottom - rect.top;

	m_iHeight = 0;
	m_yScroll = 0;

	m_bUseChannelFormat = false;
}


CFileSettings::~CFileSettings()
{

}


void CFileSettings::Destroy( void )
{
	DestroyWindow( m_hWnd );
}


HWND CFileSettings::GetWnd( void )
{
	return m_hWnd;
}


void CFileSettings::SetFile( CFile* pFile )
{
	m_pFile = pFile;
	m_hkvbufSettings = pFile->GetSettingsBuffer();
	m_pfo = &g_fo;

	if ( m_pFile->IsValidFile() )
	{
		m_bValidFile = true;
		ShowWindow( m_hFileOpts, SW_SHOWNORMAL );
		//EnableWindow( m_hScrollPanel, TRUE );
		//EnableWindow( m_hUpdateBtn, TRUE );
		//EnableWindow( m_hResetBtn, TRUE );
		//EnableWindow( m_hResetAllBtn, TRUE );

		UpdateChannelInfo();

		Load();
	}
	else
	{
		m_bValidFile = false;
		// TODO: invalidate and disable
		ShowWindow( m_hFileOpts, SW_HIDE );
		//EnableWindow( m_hScrollPanel, FALSE );
		//EnableWindow( m_hUpdateBtn, FALSE );
		//EnableWindow( m_hResetBtn, FALSE );
		//EnableWindow( m_hResetAllBtn, FALSE );
	}

	UpdateScroll();
}


void CFileSettings::FillShuffleMask( unsigned int afMask )
{
	int r = ( afMask >> 0 ) & 3;
	int g = ( afMask >> 2 ) & 3;
	int b = ( afMask >> 4 ) & 3;
	int a = ( afMask >> 6 ) & 3;

	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_SHUFFLE_R, CB_SETCURSEL, (WPARAM)r, NULL );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_SHUFFLE_G, CB_SETCURSEL, (WPARAM)g, NULL );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_SHUFFLE_B, CB_SETCURSEL, (WPARAM)b, NULL );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_SHUFFLE_A, CB_SETCURSEL, (WPARAM)a, NULL );
}


unsigned int CFileSettings::GrabShuffleMask( void )
{
	int r = ( int )SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_SHUFFLE_R, CB_GETCURSEL, 0, NULL );
	int g = ( int )SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_SHUFFLE_G, CB_GETCURSEL, 0, NULL );
	int b = ( int )SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_SHUFFLE_B, CB_GETCURSEL, 0, NULL );
	int a = ( int )SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_SHUFFLE_A, CB_GETCURSEL, 0, NULL );

	return SHUFFLE_MASK( r, g, b, a );
}


void FormatGamma( wchar_t* buffer, Gamma_t* pgm )
{
	switch ( pgm->eGamma )
	{
	case GM_LINEAR:
		wcscpy( buffer, L"Linear" );
		break;
	case GM_SRGB:
		wcscpy( buffer, L"sRGB" );
		break;
	case GM_SPECIFY:
		FormatFloat( pgm->flGamma, buffer, 7 );
		break;
	default:
		wcscpy( buffer, L"unknown" );
	}
}


void CFileSettings::FillOriginalGamma( void )
{
	Gamma_t gmInput;
	m_pFile->GetImageInputGamma( &gmInput );
	Gamma_t gmOutput;
	m_pFile->GetImageOutputGamma( &gmOutput );
	wchar_t szInput[16];
	FormatGamma( szInput, &gmInput );
	wchar_t szOutput[16];
	FormatGamma( szOutput, &gmOutput );

	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_GAMMA_IN_ORIGINAL, WM_SETTEXT, NULL, (LPARAM)szInput );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_GAMMA_OUT_ORIGINAL, WM_SETTEXT, NULL, (LPARAM)szOutput );
}



void CFileSettings::FillChannelInfo( ChannelInfo_t* pci, int iCtrlType, int iCtrlSize, int iCtrlSizeO )
{
	wchar_t szSizeO[24];
	const wchar_t* pszSize;
	const wchar_t* pszType;
	if ( pci->flags & CHANNEL_VALID )
	{
		if ( pci->iOriginalCapacity <= 0 )
		{
			wcscpy( szSizeO, L"-" );
		}
		else
		{
			swprintf( szSizeO, L"%d-bit", pci->iOriginalCapacity );
		}

		switch ( pci->eSize )
		{
		case CS_8BIT:
			pszSize = L"8-bit";
			break;
		case CS_16BIT:
			pszSize = L"16-bit";
			break;
		case CS_32BIT:
			pszSize = L"32-bit";
			break;
		default:
			pszSize = L"?";
		}

		switch ( pci->eType )
		{
		case CT_UNORM:
			pszType = L"UNORM";
			break;
		case CT_SNORM:
			pszType = L"SNORM";
			break;
		case CT_UINT:
			pszType = L"UINT";
			break;
		case CT_SINT:
			pszType = L"SINT";
			break;
		case CT_FLOAT:
			pszType = L"FLOAT";
			break;
		default:
			pszType = L"?";
		}
	}
	else
	{
		wcscpy( szSizeO, L"-" );
		pszSize = L"-";
		pszType = L"-";
	}
	SendDlgItemMessage( m_hFileOpts, iCtrlSizeO, WM_SETTEXT, NULL, (LPARAM)szSizeO );
	SendDlgItemMessage( m_hFileOpts, iCtrlSize, WM_SETTEXT, NULL, (LPARAM)pszSize );
	SendDlgItemMessage( m_hFileOpts, iCtrlType, WM_SETTEXT, NULL, (LPARAM)pszType );
}


void CFileSettings::UpdateChannelInfo( void )
{
	m_pFile->ReadChannelInfo( m_aci );

	FillChannelInfo( &m_aci[CH_R], IDC_FILEOPT_CHINFO_R_TYPE, IDC_FILEOPT_CHINFO_R_SIZE, IDC_FILEOPT_CHINFO_R_SIZEO );
	FillChannelInfo( &m_aci[CH_G], IDC_FILEOPT_CHINFO_G_TYPE, IDC_FILEOPT_CHINFO_G_SIZE, IDC_FILEOPT_CHINFO_G_SIZEO );
	FillChannelInfo( &m_aci[CH_B], IDC_FILEOPT_CHINFO_B_TYPE, IDC_FILEOPT_CHINFO_B_SIZE, IDC_FILEOPT_CHINFO_B_SIZEO );
	FillChannelInfo( &m_aci[CH_A], IDC_FILEOPT_CHINFO_A_TYPE, IDC_FILEOPT_CHINFO_A_SIZE, IDC_FILEOPT_CHINFO_A_SIZEO );
}


void CFileSettings::Load( void )
{
	// TODO: fill with zeroes, disable input
	if ( !m_pFile->IsValidFile() )
		return;

	wchar_t buf[64];

	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_ENABLE_PROCESSING, BM_SETCHECK, (WPARAM)m_pfo->bEnableProcessing, NULL );

	FormatFloat( g_flAnalysisTopLimit, buf );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_ANALYSIS_TOP_LIMIT, WM_SETTEXT, NULL, (LPARAM)buf );
	FormatFloat( g_flAnalysisBottomLimit, buf );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_ANALYSIS_BOTTOM_LIMIT, WM_SETTEXT, NULL, (LPARAM)buf );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_ANALYSIS_SOURCE, CB_SETCURSEL, (WPARAM)g_eAnalysisSource, NULL );

	FormatFloat( m_pfo->flRangeMin, buf );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_RANGE_MIN, WM_SETTEXT, NULL, (LPARAM)buf );
	FormatFloat( m_pfo->flRangeMax, buf );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_RANGE_MAX, WM_SETTEXT, NULL, (LPARAM)buf );

	FillShuffleMask( m_pfo->afShuffleMask );

	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_ROTATE, CB_SETCURSEL, (WPARAM)m_pfo->eRotateType, NULL );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_MIRROR_LEFT_RIGHT, BM_SETCHECK, (WPARAM)m_pfo->bFlipWidth, NULL );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_MIRROR_TOP_BOTTOM, BM_SETCHECK, (WPARAM)m_pfo->bFlipHeight, NULL );

	FillOriginalGamma();
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_OVERRIDE_GAMMA, BM_SETCHECK, (WPARAM)m_pfo->bOverrideGamma, NULL );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_GAMMA_IN, CB_SETCURSEL, (WPARAM)m_pfo->gmInputGamma.eGamma, NULL );
	//swprintf( buf, L"%g", m_pfo->gmInputGamma.flGamma );
	FormatFloat( m_pfo->gmInputGamma.flGamma, buf, 7 );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_GAMMA_IN_SPECIFY, WM_SETTEXT, NULL, (LPARAM)buf );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_GAMMA_OUT, CB_SETCURSEL, (WPARAM)m_pfo->gmOutputGamma.eGamma, NULL );
	//swprintf( buf, L"%g", m_pfo->gmOutputGamma.flGamma );
	FormatFloat( m_pfo->gmOutputGamma.flGamma, buf, 7 );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_GAMMA_OUT_SPECIFY, WM_SETTEXT, NULL, (LPARAM)buf );
	UpdateOverrideGamma();
}


void CFileSettings::Store( void )
{
	wchar_t buf[64];

	m_pfo->bEnableProcessing = ( SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_ENABLE_PROCESSING, BM_GETCHECK, NULL, NULL ) != 0 );

	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_ANALYSIS_TOP_LIMIT, WM_GETTEXT, (WPARAM)sizeof(buf), (LPARAM)buf );
	g_flAnalysisTopLimit = ( float )wcstod( buf, NULL );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_ANALYSIS_BOTTOM_LIMIT, WM_GETTEXT, (WPARAM)sizeof(buf), (LPARAM)buf );
	g_flAnalysisBottomLimit = ( float )wcstod( buf, NULL );
	g_eAnalysisSource = ( int )SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_ANALYSIS_SOURCE, CB_GETCURSEL, 0, NULL );

	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_RANGE_MIN, WM_GETTEXT, (WPARAM)sizeof(buf), (LPARAM)buf );
	m_pfo->flRangeMin = ( float )wcstod( buf, NULL );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_RANGE_MAX, WM_GETTEXT, (WPARAM)sizeof(buf), (LPARAM)buf );
	m_pfo->flRangeMax = ( float )wcstod( buf, NULL );

	m_pfo->afShuffleMask = GrabShuffleMask();

	m_pfo->eRotateType = ( int )SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_ROTATE, CB_GETCURSEL, 0, NULL );
	m_pfo->bFlipWidth = ( SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_MIRROR_LEFT_RIGHT, BM_GETCHECK, NULL, NULL ) != 0 );
	m_pfo->bFlipHeight = ( SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_MIRROR_TOP_BOTTOM, BM_GETCHECK, NULL, NULL ) != 0 );

	m_pfo->bOverrideGamma = ( SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_OVERRIDE_GAMMA, BM_GETCHECK, NULL, NULL ) != 0 );
	m_pfo->gmInputGamma.eGamma = ( int )SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_GAMMA_IN, CB_GETCURSEL, 0, NULL );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_GAMMA_IN_SPECIFY, WM_GETTEXT, (WPARAM)sizeof(buf), (LPARAM)buf );
	m_pfo->gmInputGamma.flGamma = ( float )wcstod( buf, NULL );
	m_pfo->gmOutputGamma.eGamma = ( int )SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_GAMMA_OUT, CB_GETCURSEL, 0, NULL );
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_GAMMA_OUT_SPECIFY, WM_GETTEXT, (WPARAM)sizeof(buf), (LPARAM)buf );
	m_pfo->gmOutputGamma.flGamma = ( float )wcstod( buf, NULL );
}



void CFileSettings::SetPickingColor( bool bPicking )
{
	if ( bPicking )
	{
		SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_ANALYSIS_STATE, WM_SETTEXT, NULL, (LPARAM)L"RMB to cancel" );
	}
	else
	{
		SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_ANALYSIS_STATE, WM_SETTEXT, NULL, NULL );
	}
	SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_ANALYSIS_PICK, BM_SETCHECK, (WPARAM)bPicking, NULL );
	g_pTextureViewer->SetPickingColor( bPicking );
}


void FormatValue1( wchar_t* buf, float flValue, int eType, int eSize )
{
	switch ( eType )
	{
	case CT_UNORM:
		{
			unsigned int v;
			float fl = max( 0.0f, min( 1.0f, flValue ) );
			switch ( eSize )
			{
			case CS_8BIT:
				v = ( unsigned int )( fl * 255.999999 );
				break;
			case CS_16BIT:
				v = ( unsigned int )( fl * 65535.999999 );
				break;
			case CS_32BIT:
				v = ( unsigned int )( fl * 4294967295.999999 );
				break;
			default:
				v = 0;
			}
			swprintf( buf, L"%u", v );
		}
		break;
	case CT_SNORM:
		{
			int v;
			float fl = max( -1.0f, min( 1.0f, flValue ) );
			switch ( eSize )
			{
			case CS_8BIT:
				v = ( unsigned int )( fl * 127.999999 );
				break;
			case CS_16BIT:
				v = ( unsigned int )( fl * 32767.999999 );
				break;
			case CS_32BIT:
				v = ( unsigned int )( fl * 2147483647.999999 );
				break;
			default:
				v = 0;
			}
			swprintf( buf, L"%d", v );
		}
		break;
	case CT_UINT:
		{
			unsigned int v = ( unsigned int )flValue;
			swprintf( buf, L"%u", v );
		}
		break;
	case CT_SINT:
		{
			int v = ( unsigned int )flValue;
			swprintf( buf, L"%d", v );
		}
		break;
	case CT_FLOAT:
		{
			FormatFloat( flValue, buf );
		}
		break;
	default:
		buf[0] = 0;
	}
}


void CFileSettings::FillResponseValue( int iChannel, float fl, int iCtrl, bool bValid )
{
	if ( bValid && ( m_aci[iChannel].flags & CHANNEL_VALID ) )
	{
		wchar_t buf[64];
		if ( m_bUseChannelFormat )
		{
			FormatValue1( buf, fl, m_aci[iChannel].eType, m_aci[iChannel].eSize );
		}
		else
		{
			FormatFloat( fl, buf, 8, FLOAT_FIXED_WIDTH );
		}

		SendDlgItemMessage( m_hFileOpts, iCtrl, WM_SETTEXT, NULL, (LPARAM)buf );
	}
	else
	{
		SendDlgItemMessage( m_hFileOpts, iCtrl, WM_SETTEXT, NULL, (LPARAM)L"-" );
	}
}


void CFileSettings::FillResponseValues( int iChannel, float flMin, float flMax, int iCtrlMin, int iCtrlMax, bool bMin, bool bMax )
{
	FillResponseValue( iChannel, flMin, iCtrlMin, bMin );
	FillResponseValue( iChannel, flMax, iCtrlMax, bMax );
}


void CFileSettings::DoAnalysis( Rect_t* prect )
{
	if ( !m_pfo->bEnableProcessing )
	{
		return;
	}

	float aflMin[NUM_CHANNELS];
	float aflMax[NUM_CHANNELS];

	// TODO: store analysis related only
	Store();

	m_pFile->DoAnalysis( prect, 0, aflMin, aflMax );

	bool bMin = true;
	if ( prect )
	{
		if ( prect->right - prect->left == 1 && prect->bottom - prect->top == 1 )
		{
			bMin = false;
		}
	}

	FillResponseValues( CH_R, aflMin[CH_R], aflMax[CH_R], IDC_FILEOPT_ANALYSIS_RESPONSE_R_MIN, IDC_FILEOPT_ANALYSIS_RESPONSE_R_MAX, bMin, true );
	FillResponseValues( CH_G, aflMin[CH_G], aflMax[CH_G], IDC_FILEOPT_ANALYSIS_RESPONSE_G_MIN, IDC_FILEOPT_ANALYSIS_RESPONSE_G_MAX, bMin, true );
	FillResponseValues( CH_B, aflMin[CH_B], aflMax[CH_B], IDC_FILEOPT_ANALYSIS_RESPONSE_B_MIN, IDC_FILEOPT_ANALYSIS_RESPONSE_B_MAX, bMin, true );
	FillResponseValues( CH_A, aflMin[CH_A], aflMax[CH_A], IDC_FILEOPT_ANALYSIS_RESPONSE_A_MIN, IDC_FILEOPT_ANALYSIS_RESPONSE_A_MAX, bMin, true );
}


void CFileSettings::Update( void )
{
	Store();

	g_pContext->UpdateInputParams( true, true );
	g_pTextureViewer->UpdateInterface();

	Load();
}


void CFileSettings::UpdateReset( void )
{
	g_pContext->UpdateInputParams( true, true );
	g_pTextureViewer->UpdateInterface();

	Load();
}


void CFileSettings::UpdateRange( void )
{
	Update();
}


void CFileSettings::ResetRange( void )
{
	m_pfo->flRangeMin = 0.0;
	m_pfo->flRangeMax = 1.0;
	UpdateReset();
}


void CFileSettings::UpdateShuffle( void )
{
	Update();
}


void CFileSettings::ResetShuffle( void )
{
	m_pfo->afShuffleMask = DEFAULT_SHUFFLE_MASK;
	UpdateReset();
}


void CFileSettings::UpdateRotate( void )
{
	Update();
}


void CFileSettings::ResetRotate( void )
{
	m_pfo->eRotateType = RT_0;
	m_pfo->bFlipWidth = false;
	m_pfo->bFlipHeight = false;
	UpdateReset();
}


void CFileSettings::UpdateGamma( void )
{
	Update();
}


void CFileSettings::ResetGamma( void )
{
	m_pFile->GetImageInputGamma( &m_pfo->gmInputGamma );
	m_pFile->GetImageOutputGamma( &m_pfo->gmOutputGamma );
	UpdateReset();
}


bool CFileSettings::FOverrideGamma( void )
{
	return  ( SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_OVERRIDE_GAMMA, BM_GETCHECK, NULL, NULL ) != 0 );
}


void CFileSettings::UpdateOverrideGamma( void )
{
	bool bOverride = FOverrideGamma();
	EnableWindow( GetDlgItem( m_hFileOpts, IDC_FILEOPT_GAMMA_IN_TEXT ), bOverride );
	EnableWindow( GetDlgItem( m_hFileOpts, IDC_FILEOPT_GAMMA_IN ), bOverride );
	EnableWindow( GetDlgItem( m_hFileOpts, IDC_FILEOPT_GAMMA_OUT_TEXT ), bOverride );
	EnableWindow( GetDlgItem( m_hFileOpts, IDC_FILEOPT_GAMMA_OUT ), bOverride );
	UpdateGammaChoice();
}


void CFileSettings::UpdateGammaChoice( void )
{
	bool bOverride = FOverrideGamma();
	int eGamma = ( int )SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_GAMMA_IN, CB_GETCURSEL, 0, NULL );
	EnableWindow( GetDlgItem( m_hFileOpts, IDC_FILEOPT_GAMMA_IN_SPECIFY ), eGamma == GM_SPECIFY && bOverride );
	eGamma = ( int )SendDlgItemMessage( m_hFileOpts, IDC_FILEOPT_GAMMA_OUT, CB_GETCURSEL, 0, NULL );
	EnableWindow( GetDlgItem( m_hFileOpts, IDC_FILEOPT_GAMMA_OUT_SPECIFY ), eGamma == GM_SPECIFY && bOverride );
}


void CFileSettings::ApplyFileSettings( void )
{
	Update();
}


void CFileSettings::OnScrollPanelSize( int fwSizeType, int nWidth, int nHeight )
{
	m_iHeight = nHeight;
	UpdateScroll();
}


void CFileSettings::OnSize( int fwSizeType, int nWidth, int nHeight )
{
	//m_iHeight = nHeight;
	//UpdateScroll();
	RECT rect;
	GetClientRect( m_hWnd, &rect );
	//rect.bottom -= 105;

	MoveWindow( m_hScrollPanel, 0, 0, rect.right-rect.left, rect.bottom-rect.top, TRUE );

	//MoveWindow( m_hUpdateBtn, 100, rect.bottom+10, 82, 23, TRUE );
	//MoveWindow( m_hResetBtn, 10, rect.bottom+10, 82, 23, TRUE );
	//MoveWindow( m_hResetAllBtn, 10, rect.bottom+40, 82, 23, TRUE );
}


void CFileSettings::OnCommand( int iCmd, int iControl, HWND hWndCtrl )
{
	//if ( iCmd == BN_CLICKED )
	//{
	//	switch ( iControl )
	//	{
	//	case IDC_FILEOPT_UPDATE:
	//		Update();
	//		break;
	//	case IDC_FILEOPT_RESET:
	//		Reset();
	//		break;
	//	case IDC_FILEOPT_RESET_ALL:
	//		ResetAll();
	//		break;
	//	}
	//}
}


void CFileSettings::EnsureVisible( RECT* prect )
{
	if ( prect->top < -m_yScroll )
	{
		m_yScroll = -(prect->top - 10);
	}
	else if ( prect->bottom > -(m_yScroll - m_iHeight) )
	{
		m_yScroll = -(prect->bottom - m_iHeight + 10);
	}

	UpdateScroll();
}


void CFileSettings::OnScrollPanelScroll( int nScrollCode, int nPos, HWND hWndScrollBar )
{
	int iPos;
	switch( nScrollCode )
	{
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		iPos = nPos;
		break;
	case SB_LINEUP:
		iPos = GetScrollPos( m_hScrollPanel, SB_VERT ) - 21;
		break;
	case SB_LINEDOWN:
		iPos = GetScrollPos( m_hScrollPanel, SB_VERT ) + 21;
		break;
	case SB_PAGEUP:
		iPos = GetScrollPos( m_hScrollPanel, SB_VERT ) - m_iHeight;
		break;
	case SB_PAGEDOWN:
		iPos = GetScrollPos( m_hScrollPanel, SB_VERT ) + m_iHeight;
		break;
	default:
		return;
	}

	int yNewScroll = -iPos;
	int iDelta = yNewScroll - m_yScroll;
	m_yScroll = yNewScroll;

	UpdateScroll();
}


void CFileSettings::UpdateScroll( void )
{
	int iMaxScroll = m_iFileOptsHeight;
	int iPos = -m_yScroll;
	iPos = min( iMaxScroll-m_iHeight, iPos );
	iPos = max( 0, iPos );
	m_yScroll = -iPos;

	bool bDisable = iMaxScroll <= m_iHeight || !m_bValidFile;

	SCROLLINFO si;
	si.cbSize = sizeof( SCROLLINFO );
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = bDisable ? m_iHeight+1 : iMaxScroll;
	si.nPage = m_iHeight;
	si.nPos = iPos;
	si.nTrackPos = 0;
	SetScrollInfo( m_hScrollPanel, SB_VERT, &si, TRUE );

	EnableScrollBar( m_hScrollPanel, SB_VERT, bDisable ? ESB_DISABLE_BOTH : ESB_ENABLE_BOTH );

	RECT rect;
	GetWindowRect( m_hFileOpts, &rect );
	MoveWindow( m_hFileOpts, 0, m_yScroll, rect.right-rect.left, rect.bottom-rect.top, TRUE );
}


void CFileSettings::Close( void )
{
	SendMessage( g_hWnd, WM_COMMAND, CMD_FILE_SETTINGS, NULL );
}


LRESULT CALLBACK CFileSettings::WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CFileSettings* pFileSettings = ( CFileSettings* )GetWindowLong( hWnd, 0 );
	LRESULT lResult = NULL;

	switch (uMsg)
	{
    case WM_CREATE:
		pFileSettings = new CFileSettings( hWnd );
		SetWindowLong( hWnd, 0, ( LONG )pFileSettings );
        break;

    case WM_DESTROY:
		delete pFileSettings;
        break;

	case WM_SIZE:
		pFileSettings->OnSize( ( int )wParam, LOWORD( lParam ), HIWORD( lParam ) );
		break;

	//case WM_VSCROLL:
	//	pFileSettings->OnVScroll( LOWORD( wParam ), HIWORD( wParam ), ( HWND )lParam );
	//	break;
	case WM_COMMAND:
		pFileSettings->OnCommand( HIWORD(wParam), LOWORD(wParam), ( HWND )lParam );
		break;

	case WM_CLOSE:
		pFileSettings->Close();
		break;

	default:
		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	}

	return lResult;
}


void InitFileSettings( HWND hWndParent )
{
	const wchar_t* pszWindowClass = L"VCtrl_FileSettings";
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = CFileSettings::WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(void*);
	wc.hInstance = g_hInst;
	wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = pszWindowClass;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIconSm = 0;

	if ( !RegisterClassEx( &wc ) )
	{
		__DEBUG_BREAK;
	}

	HWND hWnd = CreateWindowEx(
		WS_EX_TOOLWINDOW,
		pszWindowClass,
		L"Processing",
		WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU | WS_THICKFRAME,
		g_iProcessingLeft,
		g_iProcessingTop,
		g_iProcessingWidth,
		g_iProcessingHeight,
		hWndParent,
		NULL,
		g_hInst,
		NULL
		);
}

