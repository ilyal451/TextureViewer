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
#include "../shared/plibclient.h"
#include "../shared/parser.h"

#include "../format.h"

#include "system.h"
#include "keyvalue.h"

#include "format.h"


// format related stuff (loads and handles the format DLLs)


//
// ofn
// 


int PutChar(bool bCount, wchar_t* buffer, int c)
{
	if (!bCount)
	{
		*buffer = c;
	}

	return 1;
}


int CatStr(bool bCount, wchar_t* buffer, const wchar_t* psz)
{
    int i;

    for (i = 0; psz[i] != '\0'; i++)
	{
        if (!bCount)
		{
			buffer[i] = psz[i];
		}
    }

	return i;
}


int CatFormatExtensions(bool bCount, bool bPack, wchar_t* buffer, int iFmt)
{
	int nExtensions;
	int iExtension;
	int j;

	nExtensions = GetFormat(iFmt)->GetNumExtensions();

	j = 0;

	if (nExtensions != 0)
	{
		for (iExtension = 0; ;)
		{
			j += PutChar(bCount, &buffer[j], '*');
			j += PutChar(bCount, &buffer[j], '.');

			j += CatStr(bCount, &buffer[j], GetFormat(iFmt)->GetExt(iExtension));

			iExtension++;

			if (iExtension >= nExtensions)
			{
				break;
			}

			j += PutChar(bCount, &buffer[j], ';');
		}
	}

    return j;
}


int CatFormat(bool bCount, bool bPack, wchar_t* buffer, int iFmt)
{
    const wchar_t* pszName;
	int j;

    j = 0;

	pszName = GetFormat(iFmt)->GetDisplayName();

    if (pszName != NULL)
	{
        j += CatStr(bCount, &buffer[j], pszName);

		j += PutChar(bCount, &buffer[j], ' ');
    }

	j += PutChar(bCount, &buffer[j], '(');

    j += CatFormatExtensions(bCount, bPack, &buffer[j], iFmt);

	j += PutChar(bCount, &buffer[j], ')');
	j += PutChar(bCount, &buffer[j], '\0');

    j += CatFormatExtensions(bCount, bPack, &buffer[j], iFmt);

	j += PutChar(bCount, &buffer[j], '\0');
	//j += PutChar(bCount, &buffer[j], '\0');

    return j;
}


int ComposeFormatFilterString(bool bCount, bool bPack, wchar_t* buffer)
{
	int nFormats;
	int iFmt;
	int j;

	nFormats = GetNumFormats();

    j = 0;

	// single formats first

    for (iFmt = 0; iFmt < nFormats; iFmt++)
	{
        j += CatFormat(bCount, bPack, &buffer[j], iFmt);
    }

	// all supported
    
	j += CatStr(bCount, &buffer[j], L"All Supported (");

    for (iFmt = 0; iFmt < nFormats; iFmt++)
	{
        j += CatFormatExtensions(bCount, bPack, &buffer[j], iFmt);
		
		if ((iFmt + 1) != nFormats)
		{
            j += PutChar(bCount, &buffer[j], ';');
        }
    }

	j += PutChar(bCount, &buffer[j], ')');
	j += PutChar(bCount, &buffer[j], '\0');

    for(iFmt = 0; iFmt < nFormats; iFmt++)
	{
        j += CatFormatExtensions(bCount, bPack, &buffer[j], iFmt);

        if((iFmt + 1) != nFormats)
		{
			j += PutChar(bCount, &buffer[j], ';');
        }
    }

	j += PutChar(bCount, &buffer[j], '\0');
	j += PutChar(bCount, &buffer[j], '\0');

	return j;
}


void InitOFN(bool bPack, OPENFILENAME* pofn)
{
	wchar_t* buffer;
	int iSize;

	// XXX
	buffer = wbuffer;

    pofn->lStructSize = sizeof(OPENFILENAME);
    pofn->hInstance = g_hInst;

	iSize = ComposeFormatFilterString(true, bPack, NULL);
	iSize *= sizeof(wchar_t);
	pofn->lpstrFilter = (wchar_t*)AllocMemory(iSize);
	ComposeFormatFilterString(false, bPack, (wchar_t*)pofn->lpstrFilter);

	pofn->nFilterIndex = GetNumFormats() + 1; // all supported
    pofn->nMaxFile = MAX_PATH;
	pofn->Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | ((bPack)? OFN_FILEMUSTEXIST: 0);
}


OPENFILENAME g_ofn;

void InitOFNs(void)
{
	InitOFN(false, &g_ofn);
}


bool ShowOpenDialog(HWND hWnd, wchar_t* pszFileNameBuffer)
{
	g_ofn.hwndOwner = hWnd;
	g_ofn.lpstrFile = pszFileNameBuffer;

	if ( GetOpenFileName(&g_ofn) )
	{
		return true;
	}

	return false;
}



//
//
//

FormatSettingEnum_t* FindSettingsEnum( FormatSettingsTemplate_t* pst, const char* pszName )
{
	FormatSettingEnum_t* p = pst->pEnumFirst;
	while ( p != NULL )
	{
		if ( FStrEq( p->pszName, pszName ) )
		{
			return p;
		}
		p = p->pNext;
	}
	return NULL;
}


void AddSettingsTemplateEnum( FormatSettingsTemplate_t* pst, FormatSettingEnum_t* pe )
{
	if ( pst->pEnumFirst == NULL )
	{
		pst->pEnumFirst = pe;
	}
	else
	{
		FormatSettingEnum_t* p = pst->pEnumFirst;
		while ( p->pNext != NULL )
		{
			p = p->pNext;
		}
		p->pNext = pe;
	}
	pe->pNext = NULL;
}


void AddSettingsTemplateSetting( FormatSettingsTemplate_t* pst, FormatSetting_t* ps )
{
	if ( pst->pSettingFirst == NULL )
	{
		pst->pSettingFirst = ps;
	}
	else
	{
		FormatSetting_t* p = pst->pSettingFirst;
		while ( p->pNext != NULL )
		{
			p = p->pNext;
		}
		p->pNext = ps;
	}
	ps->pNext = NULL;
}


int FindSettingType( const char* pszType )
{
	static struct TypeTable
	{
		int e;
		const char* psz;
	} atype[] =
	{
		{ ST_ENUM, "ENUM" },
		{ ST_BOOL, "BOOL" },
		{ ST_INT, "INT" },
		{ ST_FLOAT, "FLOAT" },
		{ ST_STRING, "STRING" },
	};
	static int nTypes = sizeof( atype ) / sizeof( TypeTable );

	for ( int i = 0; i < nTypes; i++ )
	{
		if ( FStrEq( atype[i].psz, pszType ) )
		{
			return atype[i].e;
		}
	}

	return -1;
}


#define MAX_ENUM_STRINGS 32
bool CFormat::SettingsTemplateLineProc( char* pszLine, void* param )
{
	FormatSettingsTemplate_t* pst = ( FormatSettingsTemplate_t* )param;
	int argc;
	char* argv[MAX_ENUM_STRINGS];

	g_iParserLine++;

	ContractChars( pszLine, "\t ", ' ' );

	if ( *pszLine != '\0' )
	{
		char* pszValue;
		argc = ParseLine( 1, argv, pszLine, " ", &pszValue );

		if ( pszValue == NULL )
		{
			PARSER_ERROR( "syntax error: %s", argv[0] );
		}

		CutChars( pszValue, "\t " );

		if ( FStrEq( argv[0], "Enum" ) )
		{
			argc = ParseLine( 2, argv, pszValue, "=", NULL );
			if ( argc != 2 )
			{
				PARSER_ERROR( "syntax error: %s", argv[0] );
			}
			const char* pszName = argv[0];
			char* pszEndPtr = NULL;
			argc = ParseLine( MAX_ENUM_STRINGS, argv, argv[1], ",", &pszEndPtr );
			if ( pszEndPtr != NULL )
			{
				PARSER_ERROR( "too many enum strings (%d max)", MAX_ENUM_STRINGS );
			}
			FormatSettingEnum_t* pe = ( FormatSettingEnum_t* )malloc( sizeof( FormatSettingEnum_t ) + (sizeof(char*)*(argc-1)) );
			pe->pNext = NULL;
			pe->pszName = AllocString( pszName );
			pe->n = argc;
			for ( int i = 0; i < argc; i++ )
			{
				pe->apsz[i] = AllocString( argv[i] );
			}
			AddSettingsTemplateEnum( pst, pe );
		}
		else if ( FStrEq( argv[0], "Setting" ) )
		{
			argc = ParseLine( 2, argv, pszValue, "=", NULL );
			if ( argc != 2 )
			{
				PARSER_ERROR( "syntax error: %s", argv[0] );
			}
			const char* pszName = argv[0];
			argc = ParseLine( 2, argv, argv[1], ",", NULL );
			if ( argc != 2 )
			{
				PARSER_ERROR( "syntax error: %s", argv[0] );
			}
			const char* pszDefault = argv[0];
			argc = ParseLine( 2, argv, argv[1], ":", NULL );
			int eType = FindSettingType( argv[0] );
			if ( eType == -1 )
			{
				PARSER_ERROR( "unknown type: %s", argv[0] );
			}
			FormatSetting_t* ps = ( FormatSetting_t* )malloc( sizeof( FormatSetting_t ) );
			ps->pNext = NULL;
			ps->pszName = AllocString( pszName );
			ps->eType = eType;
			if ( eType == ST_ENUM )
			{
				if ( argc != 2 )
				{
					PARSER_ERROR( "enum is required" );
				}
				ps->pe = FindSettingsEnum( pst, argv[1] );
				if ( ps->pe == NULL )
				{
					PARSER_ERROR( "syntax error: %s", argv[1] );
				}
			}
			ps->pszDefault = AllocString( pszDefault );
			AddSettingsTemplateSetting( pst, ps );
		}
		else
		{
			PARSER_ERROR( "syntax error: %s", argv[0] );
		}
	}

	return 1;
}


void CFormat::ParseSettingsTemplate( void )
{
	wchar_t szPath[MAX_PATH];
	wcscpy( szPath, GetFormatsDir() );
	wcscat( szPath, m_pszDLL );
	wchar_t* pszExt = GetExtension( szPath );
	wcscpy( pszExt, L"template" );

	int iSize;
	char* buffer = ReadFileToBuffer( szPath, &iSize );
	if (buffer != NULL)
	{
		iSize = CutComments( buffer, ";", NULL );
		//iSize = ContractChars( buffer, "\t " );
		if (iSize != 0)
		{
			g_pszParserSource = szPath;
			g_iParserLine = 0;

			ParseBuffer( buffer, iSize, SettingsTemplateLineProc, &m_st );
		}
		FreeMemory(buffer);
	}
}



KEYVALUEBUFFER CFormat::AllocSettingsBuffer( void )
{
	return KeyValue_DuplicateKeyValueBuffer( m_hkvbufSettings );
}


void CFormat::SetSettings( KEYVALUEBUFFER hkvbuf )
{
	KeyValue_DeleteKeyValueBuffer( m_hkvbufSettings );
	m_hkvbufSettings = KeyValue_DuplicateKeyValueBuffer( hkvbuf );
	SaveSettings();
}


void CFormat::GetSettingsFileName( wchar_t* pszFileName )
{
	wcscpy( pszFileName, GetFormatsDir() );
	wcscat( pszFileName, m_pszDLL );
	wchar_t* pszExt = GetExtension( pszFileName );
	wcscpy( pszExt, L"settings" );
}


bool CFormat::SettingsLineProc( char* pszLine, void* param )
{
	KEYVALUEBUFFER hkvbuf = ( KEYVALUEBUFFER )param;
	int argc;
	char* argv[2];

	g_iParserLine++;

	if ( *pszLine != '\0' )
	{
		argc = ParseLine( 2, argv, pszLine, "=", NULL );

		if ( argc != 2 )
		{
			PARSER_ERROR( "syntax error: %s", argv[0] );
		}

		KeyValue_AddKey( hkvbuf, argv[0], argv[1] );
	}

	return 1;
}


void CFormat::LoadSettings( void )
{
	wchar_t szPath[MAX_PATH];
	GetSettingsFileName( szPath );

	int iSize;
	char* buffer = ReadFileToBuffer( szPath, &iSize );
	if (buffer != NULL)
	{
		iSize = CutComments( buffer, ";", NULL );
		iSize = CutChars( buffer, "\t " );
		if (iSize != 0)
		{
			g_pszParserSource = szPath;
			g_iParserLine = 0;

			ParseBuffer( buffer, iSize, SettingsLineProc, m_hkvbufSettings );
		}
		FreeMemory(buffer);
	}	
}


void CFormat::SaveSettings( void )
{
	wchar_t szPath[MAX_PATH];
	GetSettingsFileName( szPath );

	FILE* stream = _wfopen( szPath, L"wt" );
	if ( stream )
	{
		KEYVALUEKEY hkvkey = KeyValue_KeyFirst( m_hkvbufSettings );
		while ( hkvkey != NULL )
		{
			fprintf( stream, "%s=%s\n", KeyValue_GetKeyName( hkvkey ), KeyValue_GetKeyValue( hkvkey ) );
			hkvkey = KeyValue_KeyNext( hkvkey );
		}
		fclose( stream );
	}
}



bool IsFileExt(const wchar_t* pszExt, const wchar_t* pszFileName)
{
	wchar_t* pszFileExt = GetExtension(pszFileName);
	if ( *pszFileExt == '.' )
		return false;

	return FWStrEq(pszExt, pszFileExt);
}



//
// CFormat
//



CFormat::CFormat(const char* pszDisplayName, const char* pszDLL, int nExtensions, char* apszExt[], int iId)
{
	int i;

	m_pszDisplayName = AllocStringUnicode(pszDisplayName);
	m_pszDLL = AllocStringUnicode(pszDLL);
	m_nExtensions = nExtensions;
	m_apszExt = (wchar_t**)AllocMemory(nExtensions * sizeof(wchar_t*));
	for (i = 0; i < nExtensions; i++)
	{
		m_apszExt[i] = AllocStringUnicode(apszExt[i]);
	}
	m_iId = iId;
	m_hDLL = NULL;
	m_bDllLoadAttempted = false;
	m_hkvbufSettings = KeyValue_CreateKeyValueBuffer();
	m_st.pEnumFirst = NULL;
	m_st.pSettingFirst = NULL;
}


CFormat::~CFormat(void)
{
	int i;

	FreeMemory(m_pszDisplayName);
	
	FreeMemory(m_pszDLL);

	for (i = 0; i < m_nExtensions; i++)
	{
		FreeMemory(m_apszExt[i]);
	}
	FreeMemory(m_apszExt);

	if (m_hDLL)
	{
		FreeLibrary(m_hDLL);
	}

	KeyValue_DeleteKeyValueBuffer( m_hkvbufSettings );

	// TODO: template
}


const wchar_t* CFormat::GetDisplayName(void)
{
	return m_pszDisplayName;
}


const wchar_t* CFormat::GetDLL(void)
{
	return m_pszDLL;
}


int CFormat::GetNumExtensions(void)
{
	return m_nExtensions;
}


const wchar_t* CFormat::GetExt(int i)
{
	return m_apszExt[i];
}


int CFormat::GetId(void)
{
	return m_iId;
}


const FormatFuncs_t* CFormat::GetFuncs(void)
{
	return m_pfmt;
}


bool CFormat::IsExt(const wchar_t* pszExt)
{
	int i;

	for (i = 0; i < m_nExtensions; i++)
	{
		if ( FWStrEq(m_apszExt[i], pszExt) )
		{
			return true;
		}
	}

	return false;
}

/*
bool CFormat::IsProjectionSupported(int eProjection)
{
	switch (eProjection)
	{
	case PROJ_XY:
		return (m_pfmt->pfnGetMIPMapData != NULL);
	case PROJ_ZY:
		return (m_pfmt->pfnGetMIPMapDataZY != NULL);
	case PROJ_XZ:
		return (m_pfmt->pfnGetMIPMapDataXZ != NULL);
	default:
		__DEBUG_BREAK;
	}

	return true;
}
*/


extern PLibClientFuncs_t* g_plibclientfuncs;
void* _GetPLibClientFuncs( void )
{
	return g_plibclientfuncs;
}


const SystemFuncs_t g_SystemFuncs =
{
	SYS_OpenFile,
	SYS_CloseFile,
	SYS_GetFileSize,
	SYS_SetFilePointer,
	SYS_GetFilePointer,
	SYS_ReadFile,

	SYS_AllocMemory,
	SYS_FreeMemory,

	SYS_AllocStreamMemory,
	SYS_FreeStreamMemory,
	SYS_ReadStreamMemory,
	SYS_WriteStreamMemory,

	SYS_GetNumThreads,
	SYS_CallThread,
	SYS_WaitForAllThreads,

	KeyValue_FindKey,
	KeyValue_AddKey,
	KeyValue_GetKeyValue,
	KeyValue_SetKeyValue,

	_GetPLibClientFuncs,
};


#define GETINTERFACEVERSION "GetInterfaceVersion"
#define LOADDLL "LoadDll"

void CFormat::LoadDLL(void)
{
	if ( m_bDllLoadAttempted )
	{
		// don't try to load it multiple times
		return;
	}

	m_bDllLoadAttempted = true;

	wchar_t pszDLLPath[MAX_PATH];
	wcscpy(pszDLLPath, GetFormatsDir());
	wcscat(pszDLLPath, m_pszDLL);

	m_hDLL = LoadLibrary(pszDLLPath);
	if (m_hDLL == NULL)
	{
		NONFATAL_ERROR(L"cannot load library %s", pszDLLPath);
		return;
	}

	PFNGETFORMATINTERFACEVERSION pfnGetInterfaceVersion;
	pfnGetInterfaceVersion = (PFNGETFORMATINTERFACEVERSION)GetProcAddress(m_hDLL, GETINTERFACEVERSION);
	if (pfnGetInterfaceVersion == NULL)
	{
		NONFATAL_ERROR(L"%S not found in %s", GETINTERFACEVERSION, m_pszDLL);
		return;
	}

	int iInterfaceVersion = pfnGetInterfaceVersion( FORMAT_INTERFACE_VERSION );
	if ( iInterfaceVersion != FORMAT_INTERFACE_VERSION )
	{
		NONFATAL_ERROR(L"%s unsupported interface version %d (%d is required)", m_pszDLL, iInterfaceVersion, FORMAT_INTERFACE_VERSION);
		return;
	}

	PFNLOADFORMATDLL pfnLoadDll;
	pfnLoadDll = (PFNLOADFORMATDLL)GetProcAddress(m_hDLL, LOADDLL);
	if (pfnLoadDll == NULL)
	{
		NONFATAL_ERROR(L"%S not found in %s", LOADDLL, m_pszDLL);
		return;
	}

	m_pfmt = pfnLoadDll(&g_SystemFuncs, g_hWnd);

	if (m_pfmt == NULL)
	{
		NONFATAL_ERROR(L"format dll error %s (returned NULL)", m_pszDLL);
		return;
	}

	// OK here

	ParseSettingsTemplate();
	LoadSettings();
}



//
// Globals
//


#define MAX_FORMATS 128
CFormat* g_apFormat[MAX_FORMATS];
int g_nFormats;


int GetNumFormats(void)
{
	return g_nFormats;
}


CFormat* GetFormat(int iFmt)
{
	return g_apFormat[iFmt];
}


int FindFormatFor(const wchar_t* pszExt)
{
	int i;
	
	for (i = 0; i < g_nFormats; i++)
	{
		if (GetFormat(i)->IsExt(pszExt))
		{
			return i;
		}
	}

	return -1;
}


#define MAX_EXTENSIONS 16

typedef struct format_info_s
{
	char* pszDisplayName;
	bool bActive;
	char* pszDLL;
	int nExtensions;
	char* apszExt[MAX_EXTENSIONS];
} format_info_t;


int AddFormat(format_info_t* pfi)
{
	if (g_nFormats < MAX_FORMATS)
	{
		wchar_t szExt[MAX_PATH];
		// check for extension conflicts
		for ( int i = 0; i < pfi->nExtensions; i++ )
		{
			MultiByteToWideChar(CP_ACP, 0, pfi->apszExt[i], -1, szExt, MAX_PATH);

			int iFormat = FindFormatFor( szExt );
			if ( iFormat != -1 )
			{
				CFormat* pFormat = GetFormat( iFormat );
				FATAL_ERROR(L"Extension conflict detected for \'%s\' (%s, %S).", szExt, pFormat->GetDLL(), pfi->pszDLL );
			}
		}

		int iId = g_nFormats++;
		g_apFormat[iId] = new CFormat(pfi->pszDisplayName, pfi->pszDLL, pfi->nExtensions, pfi->apszExt, iId);

		return iId;
	}
	else
	{
		FATAL_ERROR(L"too many formats (%d max)", MAX_FORMATS);
	}

	return -1;
}


bool DllConfigLineProc(char* pszLine, void* param)
{
	format_info_t* pfi = (format_info_t*)param;
	int argc;
	char* argv[2];

	g_iParserLine++;

	if (*pszLine != '\0')
	{
		argc = ParseLine(2, argv, pszLine, "=", NULL);

		if (argc != 2)
		{
			PARSER_ERROR("syntax error: %s", argv[0]);
		}

		if (FStrEq(argv[0], "DisplayName"))
		{
			pfi->pszDisplayName = argv[1];
		}
		else if (FStrEq(argv[0], "Active"))
		{
			pfi->bActive = ( atoi( argv[1] ) != 0 );
		}
		else if (FStrEq(argv[0], "Dll"))
		{
			pfi->pszDLL = argv[1];
		}
		else if (FStrEq(argv[0], "Extensions"))
		{
			pfi->nExtensions = ParseLine(MAX_EXTENSIONS, pfi->apszExt, argv[1], ",", NULL);
		}
		else
		{
			PARSER_ERROR("syntax error: %s", argv[0]);
		}
	}

	return 1;
}


int ParseDllConfigFile(wchar_t* pszIniFile, format_info_t* pfi)
{
	char* buffer;
	int iSize;

	pfi->pszDisplayName = NULL;
	pfi->bActive = false;
	pfi->pszDLL = NULL;
	pfi->nExtensions = 0;
	
	buffer = ReadFileToBuffer(pszIniFile, &iSize);

	int iId = -1;

	if (buffer != NULL)
	{
		iSize = CutComments(buffer, ";", NULL);
		iSize = CutChars(buffer, "\t ");
		
		if (iSize != 0)
		{
			g_pszParserSource = pszIniFile;
			g_iParserLine = 0;

			ParseBuffer(buffer, iSize, DllConfigLineProc, pfi);

			// don't move it down
			if ((pfi->pszDisplayName != NULL) && (pfi->pszDLL != NULL) && (pfi->nExtensions != 0))
			{
				if (pfi->bActive)
				{
					iId = AddFormat(pfi);
				}
			}
			else
			{
				FATAL_ERROR(L"plugin config file error %s", pszIniFile);
			}
		}

		FreeMemory(buffer);
	}

	return iId;
}


void InitFormats( void )
{
	wchar_t szSearchPath[MAX_PATH];
	wchar_t* pszFileName;
    HANDLE hFind;
    WIN32_FIND_DATA fd;
	format_info_t fi;
	int i;

	wcscpy(szSearchPath, GetFormatsDir());
	wcscat(szSearchPath, L"*.ini");
	pszFileName = GetFileName(szSearchPath);

    hFind = FindFirstFile(szSearchPath, &fd);
    if (hFind != INVALID_HANDLE_VALUE)
	{
        do
		{
            // skip .. and .
            for (i = 0; fd.cFileName[i] == '.'; i++)
			{
                // do nothing
            }
            if (fd.cFileName[i] == '\0')
			{
                continue;
            }

            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
                wcscpy(pszFileName, fd.cFileName);

                int iFmt = ParseDllConfigFile(szSearchPath, &fi);
            }
        }
        while (FindNextFile(hFind, &fd));
        
		FindClose(hFind);
    }

	InitOFNs();
}


