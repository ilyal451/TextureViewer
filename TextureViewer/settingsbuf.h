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

typedef void *SETTINGS;

void Settings_SetModule( const wchar_t* pszModule );

SETTINGS Settings_CreateRead( void* buffer, int iSize );
SETTINGS Settings_CreateWrite( bool bAllowRead );
void Settings_Destroy( SETTINGS settings );
int Settings_GetPos( SETTINGS settings );
void* Settings_GetBuffer( SETTINGS settings );
void Settings_Rewind( SETTINGS settings );
void Settings_FinalizeBuffer( SETTINGS settings );

int Settings_IsEOF( SETTINGS settings );

void Settings_Read( SETTINGS settings, void* buffer, unsigned int iSize );
unsigned char Settings_ReadByte( SETTINGS settings );
short Settings_ReadShort( SETTINGS settings );
int Settings_ReadInt( SETTINGS settings );
float Settings_ReadFloat( SETTINGS settings );
//int Settings_ReadString( SETTINGS settings, char* buffer, int iBufferMax );
//int Settings_ReadStringW( SETTINGS settings, wchar_t* buffer, int iBufferMax );

void Settings_Write( SETTINGS settings, const void* buffer, unsigned int iSize );
void Settings_WriteByte( SETTINGS settings, unsigned char v );
void Settings_WriteShort( SETTINGS settings, short v );
void Settings_WriteInt( SETTINGS settings, int v );
void Settings_WriteFloat( SETTINGS settings, float v );
//void Settings_WriteString( SETTINGS settings, char* psz );
//void Settings_WriteStringW( SETTINGS settings, wchar_t* psz );

