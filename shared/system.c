/*
Texture Viewer, the game developer's image viewer (site: imagetools.itch.io)
Copyright (C) 2010-2024 Ilya Lyutin (lyutinilya551@gmail.com)
 
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the
use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it 
freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented; you must not claim
   that you wrote the original software. If you use this software in a
   product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include <stdio.h>
#include <stdlib.h>

#include "../format.h"
extern SystemFuncs_t* g_sys; // provided by the plugin

#include "system.h"



//
// fopen
//


int _SYS_CheckFileMode(const wchar_t *mode)
{
	int bRead = 0;
	int bBinary = 0;
	int i;

	for (i = 0; mode[i] != '\0'; i++)
	{
		switch (mode[i])
		{
		case 'r':
			bRead = 1;
			break;
		case 'b':
			bBinary = 1;
			break;
		default:
			return 0;
		}
	}

	return (bRead /* && bBinary*/);
}


HF SYS_wfopen(const wchar_t *filename, const wchar_t *mode)
{
	if (_SYS_CheckFileMode(mode))
	{ 
		HF stream = SYS_OpenFile(filename);

		// no files larger than 4GB
		FileSize_t fs;
		SYS_GetFileSize(stream, &fs);
		if ( fs.iSizeHigh == 0 )
		{
			return stream;
		}

		SYS_CloseFile( stream );
	}

	return NULL;
}


int SYS_fclose(HF stream)
{
	SYS_CloseFile(stream);

	return 0;
}


int SYS_fseek(HF stream, long offset, int whence)
{
	FileSize_t fs;
	unsigned int iPtr;
	
	if (whence == SEEK_SET)
	{
		iPtr = (unsigned int)offset;
	}
	else
	{
		if (whence == SEEK_CUR)
		{
			SYS_GetFilePointer(stream, &fs);
			iPtr = fs.iSizeLow;
		}
		else if (whence == SEEK_END)
		{
			SYS_GetFileSize(stream, &fs);
			iPtr = fs.iSizeLow;
		}

		if (offset >= 0)
		{
			iPtr += offset;
		}
		else
		{
			iPtr -= (-offset);
		}
	}

	fs.iSizeLow = iPtr;
	fs.iSizeHigh = 0;
	SYS_SetFilePointer(stream, &fs);

	return 0;
}


long SYS_ftell(HF stream)
{
	FileSize_t fs;
	SYS_GetFilePointer(stream, &fs);

	return (long)fs.iSizeLow;
}


size_t SYS_fread(void *ptr, size_t size, size_t n, HF stream)
{
	size_t iRead;
	size_t i;

	for (i = 0; i < n; i++)
	{
		void* buffer = &((char*)ptr)[i * size];
		
		iRead = (size_t)SYS_ReadFile(stream, buffer, (unsigned int)size);

		if (iRead < size)
		{
			break;
		}
	}

	return i;
}



void* SYS_MapFile(HF stream)
{
	FileSize_t fs;
	SYS_GetFileSize(stream, &fs);
	if ( fs.iSizeHigh == 0 )
	{
		void* p = SYS_malloc(fs.iSizeLow);
		if ( p != NULL )
		{
			SYS_ReadFile(stream, p, fs.iSizeLow);

			return p;
		}
	}

	return NULL;
}


void SYS_UnmapFile(void* p)
{
	SYS_free(p);
}


