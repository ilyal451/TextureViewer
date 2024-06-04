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

#include <windows.h>
#include <stdio.h>

#include "../shared/utils.h"

#include "keyvalue.h"


typedef struct KeyValueKey
{
	struct KeyValueKey* pPrev;
	struct KeyValueKey* pNext;
	char* pszKey;
	char* pszValue;
} KeyValueKey_t;

typedef struct KeyValueBuffer
{
	KeyValueKey_t* pKeyFirst;
	KeyValueKey_t* pKeyLast;
} KeyValueBuffer_t;


KEYVALUEBUFFER KeyValue_CreateKeyValueBuffer( void )
{
	KeyValueBuffer_t* pkvbuf = ( KeyValueBuffer_t* )malloc( sizeof( KeyValueBuffer_t ) );

	if ( pkvbuf != NULL )
	{
		pkvbuf->pKeyFirst = NULL;
		pkvbuf->pKeyLast = NULL;
	}

	return ( KEYVALUEBUFFER )pkvbuf;
}


KEYVALUEBUFFER KeyValue_DuplicateKeyValueBuffer( KEYVALUEBUFFER hkvbufCopyOf )
{
	KEYVALUEBUFFER hkvbuf = KeyValue_CreateKeyValueBuffer();
	KEYVALUEKEY hkvkey = KeyValue_KeyFirst( hkvbufCopyOf );
	while ( hkvkey != NULL )
	{
		KeyValue_AddKey( hkvbuf, KeyValue_GetKeyName( hkvkey ), KeyValue_GetKeyValue( hkvkey ) );
		hkvkey = KeyValue_KeyNext( hkvkey );
	}
	return hkvbuf;
}


void KeyValue_DeleteKeyValueBuffer( KEYVALUEBUFFER hkvbuf )
{
	KeyValueBuffer_t* pkvbuf = ( KeyValueBuffer_t* )hkvbuf;
	KeyValueKey_t* pkvkey = pkvbuf->pKeyFirst;

	while ( pkvkey != NULL )
	{
		KeyValueKey_t* pkvThis = pkvkey;

		pkvkey = pkvkey->pNext;

		free( pkvThis );
	}

	free ( pkvbuf );
}


KEYVALUEKEY KeyValue_FindKey( KEYVALUEBUFFER hkvbuf, const char* pszKey )
{
	KeyValueBuffer_t* pkvbuf = ( KeyValueBuffer_t* )hkvbuf;
	KeyValueKey_t* pkvkey = pkvbuf->pKeyFirst;

	while ( pkvkey != NULL )
	{
		if ( FStrEq( pkvkey->pszKey, pszKey ) )
		{
			return ( KEYVALUEKEY )pkvkey;
		}

		pkvkey = pkvkey->pNext;
	}

	return NULL;
}


KeyValueKey_t* _KeyValue_CreateKey( const char* pszKey, const char* pszInitValue )
{
	KeyValueKey_t* pkvkey = ( KeyValueKey_t* )malloc( sizeof( KeyValueKey_t ) );

	if ( pkvkey != NULL )
	{
		pkvkey->pPrev = NULL;
		pkvkey->pNext = NULL;
		pkvkey->pszKey = AllocString( pszKey );
		pkvkey->pszValue = AllocString( pszInitValue );

		return pkvkey;
	}

	return NULL;
}


void _KeyValue_DeleteKey( KeyValueKey_t* pkvkey )
{
	FreeString( pkvkey->pszKey );
	FreeString( pkvkey->pszValue );
	free( pkvkey );
}


KEYVALUEKEY KeyValue_AddKey( KEYVALUEBUFFER hkvbuf, const char* pszKey, const char* pszInitValue )
{
	if ( KeyValue_FindKey( hkvbuf, pszKey ) != NULL )
	{
		// already exists
		return NULL;
	}

	KeyValueBuffer_t* pkvbuf = ( KeyValueBuffer_t* )hkvbuf;
	KeyValueKey_t* pkvkey = _KeyValue_CreateKey( pszKey, pszInitValue );
	if ( pkvkey )
	{
		if ( pkvbuf->pKeyFirst == NULL )
		{
			pkvbuf->pKeyFirst = pkvkey;
		}
		else
		{
			pkvbuf->pKeyLast->pNext = pkvkey;
			pkvkey->pPrev = pkvbuf->pKeyLast;
		}
		pkvbuf->pKeyLast = pkvkey;

		return pkvkey;
	}

	return NULL;
}


KEYVALUEKEY KeyValue_KeyFirst( KEYVALUEBUFFER hkvbuf )
{
	KeyValueBuffer_t* pkvbuf = ( KeyValueBuffer_t* )hkvbuf;
	return ( KEYVALUEKEY )pkvbuf->pKeyFirst;
}


KEYVALUEKEY KeyValue_KeyNext( KEYVALUEKEY hkvkey )
{
	KeyValueKey_t* pkvkey = ( KeyValueKey_t* )hkvkey;
	return ( KEYVALUEKEY )pkvkey->pNext;
}


const char* KeyValue_GetKeyName( KEYVALUEKEY hkvkey )
{
	KeyValueKey_t* pkvkey = ( KeyValueKey_t* )hkvkey;
	return pkvkey->pszKey;
}


const char* KeyValue_GetKeyValue( KEYVALUEKEY hkvkey )
{
	KeyValueKey_t* pkvkey = ( KeyValueKey_t* )hkvkey;
	return pkvkey->pszValue;
}


void KeyValue_SetKeyValue( KEYVALUEKEY hkvkey, const char* pszValue )
{
	KeyValueKey_t* pkvkey = ( KeyValueKey_t* )hkvkey;
	FreeString( pkvkey->pszValue );
	pkvkey->pszValue = AllocString( pszValue );
}



