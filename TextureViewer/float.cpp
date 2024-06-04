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
#include <math.h>

#include "float.h"


int FormatFloat( float fl, wchar_t* buffer, int width, int options )
{
	union
	{
		float fl;
		unsigned int i;
	} aa;

	aa.fl = fl;
   
	int sign = ( aa.i & 0x80000000 );
	int exp = ( aa.i >> 23 ) & 0xFF;
	if ( exp == 0 )
	{
		// zero or denorm
		// we treat denorms as zero
		int n = swprintf( buffer, sign ? L"-0.0" : L"0.0" );
		if ( options & FLOAT_FIXED_WIDTH )
		{
			int nDigits = n;
			if ( *buffer == '-' )
			{
				nDigits--;
			}
			for ( ; nDigits < width; nDigits++ )
			{
				buffer[n++] = '0';
			}
			buffer[n] = '\0';
		}
		return n;
	}
	else if ( exp == 0xFF )
	{
		// inf or nan
		int n;
		int man = ( aa.i & 0x007FFFFF );
		if ( man == 0 )
		{
			n = swprintf( buffer, sign ? L"-INF" : L"+INF" );
		}
		else
		{
			n = swprintf( buffer, L"NAN" );
		}

		return n;
	}

	exp -= 127; // unbias

	float dexp = exp * 0.30103f; // log10( 2.0f );

	//float flM = log10( fl );
	int iOffset = 7-(int)dexp;
	float flD = pow( 10.0f, iOffset );
	float flS = fl * flD;
	// float flM = log10( flS );


	int n = swprintf( buffer, L"%d", (int)flS );
	int nDigits = n;
	if ( buffer[0] == '-' )
	{
		nDigits--;
	}

	// bring it all to a single form
	if ( iOffset >= nDigits ) // have to pad
	{
		int iPad = 2 + iOffset - nDigits;	// including '0.'
		int i = n;
		n += iPad;
		buffer[n] = '\0';
		int iCount = nDigits;
		for ( ; iCount > 0; iCount-- )
		{
			buffer[i-1+iPad] = buffer[i-1];
			iOffset--;
			i--;
		}
		buffer[i++] = '0';
		buffer[i++] = '.';
		for ( iCount = 2; iCount < iPad; iCount++ )
		{
			buffer[i++] = '0';
			iOffset--;
		}
		nDigits += iPad;
	}
	else if ( iOffset > 0 ) // find dot
	{
		int i = n++;
		buffer[n] = '\0';
		for ( int iCount = nDigits; iCount > 0; iCount-- )
		{
			buffer[i] = buffer[i-1];
			i--;
			iOffset--;
			if ( !iOffset )
			{
				break;
			}
		}
		buffer[i] = '.';
		nDigits += 1; // the dot
	}
	else if ( iOffset == 0 ) // exact
	{
		buffer[n++] = '.';
		buffer[n++] = '0';
		buffer[n] = '\0';
		nDigits += 2; // 0.
	}
	else // add zeroes
	{
		for ( ; iOffset < 0; iOffset++ )
		{
			nDigits++;
			buffer[n++] = '0';
		}
		buffer[n++] = '.';
		buffer[n++] = '0';
		buffer[n] = '\0';
		nDigits += 2; // .0
	}

	// cut zeroes until the dot
	for ( ; nDigits > 1; )
	{
		if ( buffer[n-1] != '0' )
		{
			break;
		}
		if ( buffer[n-2] == '.' )
		{
			break;
		}
		n--;
		buffer[n] = '\0';
		nDigits--;
	}

	// check if we have to cut even more
	if ( nDigits > width )
	{
		// cut the digits after the point
		for ( ; nDigits > width; )
		{
			if ( buffer[n-2] == '.' )
			{
				break;
			}
			n--;
			// TODO: proper carry.....
			if ( buffer[n] > 5 )
			{
				for ( int i = n-1; ; i-- )
				{
					if ( buffer[i] == '.' )
					{
						continue;
					}
					int a = buffer[i] - '0';
					a++;
					if ( a > 9 )
					{
						buffer[i] = '0';
					}
					else
					{
						buffer[i] = '0' + a;
						break;
					}
					if ( i == 0 || buffer[i-1] == '-' )
					{
						// pad
						for ( int j = n; j >= i; j-- )
						{
							buffer[j+1] = buffer[j];
						}
						buffer[i] = '1';
						n++;
						nDigits++;
						break;
					}
				}
			}
			buffer[n] = '\0';
			nDigits--;
		}

		// cut zeroes again
		for ( ; nDigits > 1; )
		{
			if ( buffer[n-1] != '0' )
			{
				break;
			}
			if ( buffer[n-2] == '.' )
			{
				break;
			}
			n--;
			buffer[n] = '\0';
			nDigits--;
		}

		// if still wasn't enough
		if ( nDigits > width )
		{
			// do the exponential form
			// calculate num digits to cut away
			int nServiceDigits = 2 + 2; // the 'e+/-' expr, assume the exponent is 2 digits
			int nTargetDigits = width - nServiceDigits;
			if ( nTargetDigits < 3 )
			{
				// not enough space to write it
				// give up
				wcscpy( buffer, ( buffer[0] == '-' ) ? L"-INF" : L"+INF" );
			}
			else
			{
				// here we assume that buffer[n-2] is the dot (see above)
				// eat digits and move the dot
				iOffset = 0;
				for ( ; nDigits > nTargetDigits; nDigits-- )
				{
					n--;
					buffer[n] = '\0';
					buffer[n-1] = buffer[n-2];
					buffer[n-2] = '.';
					iOffset++;
				}

				// put the exponent
				n += swprintf( &buffer[n], L"e+%.2d", iOffset );
			}
			return n; // return here
		}
	}

	if ( nDigits < width )
	{
		if ( options & FLOAT_FIXED_WIDTH )
		{
			for ( ; nDigits < width; nDigits++ )
			{
				buffer[n++] = '0';
			}
			buffer[n] = '\0';
		}
	}

	return n;
}


