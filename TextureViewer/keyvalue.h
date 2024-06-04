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

typedef void *KEYVALUEKEY;
typedef void *KEYVALUEBUFFER;

KEYVALUEBUFFER KeyValue_CreateKeyValueBuffer( void );
KEYVALUEBUFFER KeyValue_DuplicateKeyValueBuffer( KEYVALUEBUFFER hkvbufCopyOf );
void KeyValue_DeleteKeyValueBuffer( KEYVALUEBUFFER hkvbuf );
KEYVALUEKEY KeyValue_FindKey( KEYVALUEBUFFER hkvbuf, const char* pszKey );
KEYVALUEKEY KeyValue_AddKey( KEYVALUEBUFFER hkvbuf, const char* pszKey, const char* pszValueInit );
KEYVALUEKEY KeyValue_KeyFirst( KEYVALUEBUFFER hkvbuf );
KEYVALUEKEY KeyValue_KeyNext( KEYVALUEKEY hkvkey );
const char* KeyValue_GetKeyName( KEYVALUEKEY hkvkey );
const char* KeyValue_GetKeyValue( KEYVALUEKEY hkvkey );
void KeyValue_SetKeyValue( KEYVALUEKEY hkvkey, const char* pszValue );

