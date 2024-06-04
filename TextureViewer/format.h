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


typedef struct FormatSettingEnum
{
	struct FormatSettingEnum* pNext;
	const char* pszName;
	int n;
	const char* apsz[1];
} FormatSettingEnum_t;

enum FormatSettingTypes
{
	ST_ENUM,
	ST_BOOL,
	ST_INT,
	ST_FLOAT,
	ST_STRING,
};

typedef struct FormatSetting
{
	struct FormatSetting* pNext;
	const char* pszName;
	int eType;
	FormatSettingEnum_t* pe;
	const char* pszDefault;
} FormatSetting_t;

typedef struct FormatSettingsTemplate
{
	FormatSettingEnum_t* pEnumFirst;
	FormatSetting_t* pSettingFirst;
} FormatSettingsTemplate_t;


class CFormat
{

public:
	CFormat(const char* pszDisplayName, const char* pszDLL, int nExtensions, char* apszExt[], int iId);
	~CFormat();
	const wchar_t* GetDisplayName(void);
	const wchar_t* GetDLL(void);
	int GetNumExtensions(void);
	const wchar_t* GetExt(int i);
	int GetId(void);
	const FormatFuncs_t* GetFuncs(void);
	bool IsExt(const wchar_t* pszExt);
	//bool IsProjectionSupported(int eProjection);

	void LoadDLL(void);

	KEYVALUEBUFFER AllocSettingsBuffer( void );
	void SetSettings( KEYVALUEBUFFER hkvbuf );

private:
	wchar_t* m_pszDisplayName;
	wchar_t* m_pszDLL;
	int m_nExtensions;
	wchar_t** m_apszExt;
	int m_iId;
	HMODULE m_hDLL;
	bool m_bDllLoadAttempted;
	const FormatFuncs_t* m_pfmt;
	KEYVALUEBUFFER m_hkvbufSettings;
	FormatSettingsTemplate_t m_st;

	static bool SettingsTemplateLineProc( char* pszLine, void* param );
	void ParseSettingsTemplate( void );
	void GetSettingsFileName( wchar_t* pszFileName );
	static bool SettingsLineProc( char* pszLine, void* param );
	void LoadSettings( void );
	void SaveSettings( void );
};


int GetNumFormats(void);
CFormat* GetFormat(int iFormat);
int FindFormatFor(const wchar_t* pszExt);

void InitFormats(void);

