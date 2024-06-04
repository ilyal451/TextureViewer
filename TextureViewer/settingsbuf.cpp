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

#include <stdio.h>

#include "settingsbuf.h"


const wchar_t* g_pszSettings_Module;

void Settings_SetModule( const wchar_t* pszModule )
{
	g_pszSettings_Module = pszModule;
}


//
// open/close
//

#define SETTINGS_BUFFER_CHUNK_SIZE 4096

typedef struct Settings_S
{
	unsigned char* buffer;
	int iSize;
	int iPos;
	bool bRead;
	bool bWrite;
	bool bAllocated;
} Settings_t;


SETTINGS Settings_CreateRead( void* buffer, int iSize )
{
	Settings_t* pSettings = ( Settings_t* )malloc( sizeof( Settings_t ) );
	pSettings->buffer = ( unsigned char* )buffer;
	pSettings->iSize = iSize;
	pSettings->iPos = 0;
	pSettings->bRead = true;
	pSettings->bWrite = false;
	pSettings->bAllocated = false;

	return ( SETTINGS )pSettings;
}


SETTINGS Settings_CreateWrite( bool bAllowRead )
{
	Settings_t* pSettings = ( Settings_t* )malloc( sizeof( Settings_t ) );
	pSettings->buffer = ( unsigned char* )malloc( SETTINGS_BUFFER_CHUNK_SIZE );
	pSettings->iSize = SETTINGS_BUFFER_CHUNK_SIZE;
	pSettings->iPos = 0;
	pSettings->bRead = bAllowRead;
	pSettings->bWrite = true;
	pSettings->bAllocated = true;

	return ( SETTINGS )pSettings;
}


void Settings_Destroy( SETTINGS settings )
{
	Settings_t* pSettings = ( Settings_t* )settings;
	if ( pSettings->bAllocated )
	{
		free( pSettings->buffer );
	}
	free( pSettings );
}


int Settings_GetPos( SETTINGS settings )
{
	Settings_t* pSettings = ( Settings_t* )settings;
	return pSettings->iPos;
}


void* Settings_GetBuffer( SETTINGS settings )
{
	Settings_t* pSettings = ( Settings_t* )settings;
	return pSettings->buffer;
}


void Settings_Rewind( SETTINGS settings )
{
	Settings_t* pSettings = ( Settings_t* )settings;
	pSettings->iPos = 0;
}


int Settings_IsEOF( SETTINGS settings )
{
	Settings_t* pSettings = ( Settings_t* )settings;
	return ( pSettings->iPos == pSettings->iSize );
}


void Settings_FinalizeBuffer( SETTINGS settings )
{
	Settings_t* pSettings = ( Settings_t* )settings;
	pSettings->iSize = pSettings->iPos;
	//pSettings->bRead = true;
	pSettings->bWrite = false;
}


//
// read
//


void _Settings_CheckRead( Settings_t* pSettings )
{
	if ( !pSettings->bRead )
	{
		FATAL_ERROR( L"Attempting to read from a write-only settings buffer" );
	}
}


void _Settings_ReadFrom( Settings_t* pSettings, void* buffer, unsigned int iSize )
{
	if ( ( pSettings->iSize - pSettings->iPos ) < iSize )
	{
		FATAL_ERROR( L"Unexpected EOF on a settings buffer" );
	}

	memcpy( buffer, &pSettings->buffer[pSettings->iPos], iSize );
	pSettings->iPos += iSize;
}


void Settings_Read( SETTINGS settings, void* buffer, unsigned int iSize )
{
	Settings_t* pSettings = ( Settings_t* )settings;

	_Settings_CheckRead( pSettings );

	bool bPassed = true;

	unsigned char c;
	_Settings_ReadFrom( pSettings, &c, 1 );
	if ( c != -1 )
	{
		if ( c != iSize )
		{
			bPassed = false;
		}
	}
	else
	{
		unsigned int i;
		_Settings_ReadFrom( pSettings, &i, 4 );
		if ( i != iSize )
		{
			bPassed = false;
		}
	}

	if ( !bPassed )
	{
		FATAL_ERROR( L"Settings buffer read/write mismatch in %s.", g_pszSettings_Module );
	}

	_Settings_ReadFrom( pSettings, buffer, iSize );
}


unsigned char Settings_ReadByte( SETTINGS settings )
{
	unsigned char v;
	Settings_Read( settings, &v, 1 );

	return v;
}


short Settings_ReadShort( SETTINGS settings )
{
	short v;
	Settings_Read( settings, &v, 2 );

	return v;
}


int Settings_ReadInt( SETTINGS settings )
{
	int v;
	Settings_Read( settings, &v, 4 );

	return v;
}


float Settings_ReadFloat( SETTINGS settings )
{
	float v;
	Settings_Read( settings, &v, 4 );

	return v;
}



//
// write
//


void _Settings_CheckWrite( Settings_t* pSettings )
{
	if ( !pSettings->bWrite )
	{
		FATAL_ERROR( L"Attempting to write to a read-only settings buffer" );
	}
}


void _Settings_WriteTo( Settings_t* pSettings, const void* buffer, unsigned int iSize )
{
	if ( ( pSettings->iSize - pSettings->iPos ) < iSize )
	{
		// the least efficient way... (reallocating the buffers)
		int iSizeNeeded = iSize - ( pSettings->iSize - pSettings->iPos );
		int nChunksNeeded = ( iSizeNeeded + SETTINGS_BUFFER_CHUNK_SIZE - 1 ) / SETTINGS_BUFFER_CHUNK_SIZE;
		int iNewSize = pSettings->iSize + ( nChunksNeeded * SETTINGS_BUFFER_CHUNK_SIZE );
		unsigned char* newBuffer = ( unsigned char* )malloc( iNewSize );
		if ( newBuffer == NULL )
		{
			FATAL_ERROR( L"Coundn't write to the settings buffer (not enough memory)" );
		}
		memcpy( newBuffer, pSettings->buffer, pSettings->iPos );
		free( pSettings->buffer );
		pSettings->buffer = newBuffer;
		pSettings->iSize = iNewSize;
	}

	memcpy( &pSettings->buffer[pSettings->iPos], buffer, iSize );
	pSettings->iPos += iSize;
}


void Settings_Write( SETTINGS settings, const void* buffer, unsigned int iSize )
{
	Settings_t* pSettings = ( Settings_t* )settings;

	_Settings_CheckWrite( pSettings );

	bool bPassed = true;

	if ( iSize < 128 )
	{
		_Settings_WriteTo( pSettings, &iSize, 1 );
	}
	else
	{
		unsigned char c = -1;
		_Settings_WriteTo( pSettings, &c, 1 );
		_Settings_WriteTo( pSettings, &iSize, 4 );
	}

	_Settings_WriteTo( pSettings, buffer, iSize );
}


void Settings_WriteByte( SETTINGS settings, unsigned char v )
{
	Settings_Write( settings, &v, 1 );
}


void Settings_WriteShort( SETTINGS settings, short v )
{
	Settings_Write( settings, &v, 2 );
}


void Settings_WriteInt( SETTINGS settings, int v )
{
	Settings_Write( settings, &v, 4 );
}


void Settings_WriteFloat( SETTINGS settings, float v )
{
	Settings_Write( settings, &v, 4 );
}

