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

#include "memory.h"


#ifndef __cplusplus
typedef unsigned char bool;
#define true 1
#define false 0
#endif


#define PAGE_SIZE 0x1000
#define MAX_MEM 0x80000000

typedef unsigned int chunk_word_t;
#define CHUNK_WORD_BITS (sizeof(chunk_word_t) * 8)
#define NUM_WORDS_CHUNK 8
#define NUM_CHUNK_PAGES (NUM_WORDS_CHUNK * CHUNK_WORD_BITS)
#define CHUNK_SIZE (NUM_CHUNK_PAGES * PAGE_SIZE)
#define MAX_CHUNKS (MAX_MEM / CHUNK_SIZE)

/*
currently:
CHUNK_WORD_BITS = 32
NUM_CHUNK_PAGES = 256
CHUNK_SIZE = 0x100000 (1MB)
MAX_CHUNKS = 0x800 (2048)

*/

typedef struct mem_chunk_s
{
	void* pAddress;
	chunk_word_t afPageBusy[NUM_WORDS_CHUNK];
} mem_chunk_t;

mem_chunk_t* g_apchunk[MAX_CHUNKS];



mem_chunk_t* _S_AllocChunk(void)
{
	mem_chunk_t* pchunk;

	// XXX: fragment?
	pchunk = (mem_chunk_t*)malloc(sizeof(mem_chunk_t));

	if (pchunk != NULL)
	{
		pchunk->pAddress = VirtualAlloc(NULL, CHUNK_SIZE, MEM_RESERVE | MEM_TOP_DOWN, PAGE_READWRITE);

		if (pchunk->pAddress != NULL)
		{
			int i;
			
			for (i = 0; i < NUM_WORDS_CHUNK; i++)
			{
				pchunk->afPageBusy[i] = 0;
			}

			return pchunk;
		}

		free(pchunk);
	}

	return NULL;
}


void _S_FreeChunk(mem_chunk_t* pchunk)
{
	VirtualFree(pchunk->pAddress, 0, MEM_RELEASE);
	
	free(pchunk);
}


bool _S_CanFreeChunk(mem_chunk_t* pchunk)
{
	int i;
	
	for (i = 0; i < NUM_WORDS_CHUNK; i++)
	{
		if (pchunk->afPageBusy[i] != 0)
		{
			return false;
		}
	}

	return true;
}


bool _S_HasFreeSpace(mem_chunk_t* pchunk)
{
	int i;
	
	for (i = 0; i < NUM_WORDS_CHUNK; i++)
	{
		if (pchunk->afPageBusy[i] != -1)
		{
			return true;
		}
	}

	return false;
}


void* _S_ChunkAllocPages(mem_chunk_t* pchunk, int iStart, int n)
{
	size_t iOffset;
	size_t iSize;
	void* pStartAddress;
	void* p;

	iOffset = iStart * PAGE_SIZE;
	iSize = n * PAGE_SIZE;
	pStartAddress = (void*)((size_t)pchunk->pAddress + iOffset);

	p = VirtualAlloc(pStartAddress, iSize, MEM_COMMIT, PAGE_READWRITE);

	if (p != NULL)
	{
		int iEnd;
		int i;

		iEnd = iStart + n;

		// XXX: _S_ChangePagesBusy
		// mark pages are busy
		for (i = iStart; i < iEnd; i++)
		{
			int iByte = i / CHUNK_WORD_BITS;
			int iBit = i % CHUNK_WORD_BITS;
			unsigned int mask = 1 << iBit;
			
			pchunk->afPageBusy[iByte] |= mask;
		}
	}

	return p;
}


void _S_ChunkFreePages(mem_chunk_t* pchunk, int iStart, int n)
{
	size_t iOffset;
	size_t iSize;
	void* pStartAddress;
	int iEnd;
	int i;

	iEnd = iStart + n;

	for (i = iStart; i < iEnd; i++)
	{
		int iByte = i / CHUNK_WORD_BITS;
		int iBit = i % CHUNK_WORD_BITS;
		unsigned int mask = 1 << iBit;
		
		pchunk->afPageBusy[iByte] &= ~mask;
	}

	iOffset = iStart * PAGE_SIZE;
	iSize = n * PAGE_SIZE;
	pStartAddress = (void*)((size_t)pchunk->pAddress + iOffset);

	VirtualFree(pStartAddress, iSize, MEM_DECOMMIT);
}


void* _S_ChunkAllocPiece(mem_chunk_t* pchunk, size_t iSize)
{
	int iStart;
	int nPages;
	int i;

	nPages = 0;

	for (i = 0; i < NUM_CHUNK_PAGES; i++)
	{
		int iByte = i / CHUNK_WORD_BITS;
		int iBit = i % CHUNK_WORD_BITS;
		unsigned int mask = 1 << iBit;

		if (!(pchunk->afPageBusy[iByte] & mask))
		{
			if (nPages == 0)
			{
				iStart = i;
			}
			
			nPages++;

			if ((nPages * PAGE_SIZE) >= iSize)
			{
				return _S_ChunkAllocPages(pchunk, iStart, nPages);
			}
		}
		else
		{
			nPages = 0;
		}
	}

	return NULL;
}


void _S_ChunkFreePiece(mem_chunk_t* pchunk, void* p, size_t iSize)
{
	int iStart;
	int nPages;

	iStart = ((size_t)p - (size_t)pchunk->pAddress) / PAGE_SIZE;
	nPages = (iSize + (PAGE_SIZE - 1)) / PAGE_SIZE;
	
	_S_ChunkFreePages(pchunk, iStart, nPages);
}


void* _S_AllocPiece(size_t iSize)
{
	void* p;
	int i;

	p = NULL;

	// XXX: performance?
	for (i = 0; i < MAX_CHUNKS; i++)
	{
		if (g_apchunk[i] != NULL)
		{
			if (_S_HasFreeSpace(g_apchunk[i])) // optimization
			{
				p = _S_ChunkAllocPiece(g_apchunk[i], iSize);

				if (p != NULL)
				{
					break;
				}
			}
		}
	}

	if (p == NULL)
	{
		// well, alloc a new chunk
		
		// get the first empty item
		for (i = 0; g_apchunk[i] != NULL; i++)
		{
			// nothing
		}

		g_apchunk[i] = _S_AllocChunk();

		p = _S_ChunkAllocPiece(g_apchunk[i], iSize);
	}

	return p;
}


bool _S_IsChunkAddress(mem_chunk_t* pchunk, void* p)
{
	return (((size_t)p >= (size_t)pchunk->pAddress) && ((size_t)p < ((size_t)pchunk->pAddress + CHUNK_SIZE)));
}


void _S_FreePiece(void* p, size_t iSize)
{
	int i;

	// XXX: performance?
	for (i = 0; i < MAX_CHUNKS; i++)
	{
		if (g_apchunk[i] != NULL)
		{
			if (_S_IsChunkAddress(g_apchunk[i], p))
			{
				_S_ChunkFreePiece(g_apchunk[i], p, iSize);
				
				if (_S_CanFreeChunk(g_apchunk[i]))
				{
					_S_FreeChunk(g_apchunk[i]);
					g_apchunk[i] = NULL;
				}

				break;
			}
		}
	}

}


typedef struct piece_s
{
	void* p;
	size_t iSize;
} piece_t;


typedef struct mem_s
{
	size_t iSize;
	int nPieces;
	piece_t apiece[1];
} mem_t, *HMEMOBJ;


mem_t* _S_AllocStruct(int nPieces)
{
	mem_t* p;
	int i;

	p = (mem_t*)malloc(sizeof(mem_t) + ((nPieces - 1) * sizeof(piece_t)));

	if (p != NULL)
	{
		p->iSize = 0;
		p->nPieces = nPieces;

		for (i = 0; i < nPieces; i++)
		{
			p->apiece[i].p = NULL;
			p->apiece[i].iSize = 0;
		}

		return p;
	}

	return NULL;
}


void _S_FreeStruct(mem_t* p)
{
	free(p);
}


mem_t* _S_AllocMemory(size_t iSize)
{
	if (iSize != 0)
	{
		int nPieces;
		mem_t* p;

		nPieces = (iSize + (CHUNK_SIZE - 1)) / CHUNK_SIZE;

		p = _S_AllocStruct(nPieces);

		if (p != NULL)
		{
			size_t iOffset;
			int i, j;

			p->iSize = iSize;
			p->nPieces = nPieces;

			iOffset = 0;

			for (i = 0; i < nPieces; i++)
			{
				size_t iPieceSize = min(CHUNK_SIZE, iSize - iOffset);

				p->apiece[i].p = _S_AllocPiece(iPieceSize);

				if (p->apiece[i].p == NULL)
				{
					goto skip;
				}

				p->apiece[i].iSize = iPieceSize;

				iOffset += iPieceSize;
			}

			if ((iOffset != iSize) || (i != nPieces))
			{
				__asm int 3;
			}

			return p;

skip:
			// free all we have already allocated
			for (j = 0; j < i; j++)
			{
				_S_FreePiece(p->apiece[j].p, p->apiece[j].iSize);
			}

			_S_FreeStruct(p);
		}
	}

	return NULL;
}


void _S_FreeMemory(mem_t* p)
{
	int i;

	for (i = 0; i < p->nPieces; i++)
	{
		_S_FreePiece(p->apiece[i].p, p->apiece[i].iSize);
	}

	_S_FreeStruct(p);
}


size_t _S_GetMemSize(mem_t* p)
{
	return p->iSize;
}


size_t _S_GetSizeAllocated(mem_t* p)
{
	return (p->iSize + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
}


void _S_ReadMemory(mem_t* p, size_t iOffset, size_t iSize, void* buffer)
{
	int iPiece;
	size_t iPieceOffset;
	size_t iSizeRead;

	iPiece = iOffset / CHUNK_SIZE;
	iPieceOffset = iOffset % CHUNK_SIZE;
	iSizeRead = 0;

	while (iSizeRead < iSize)
	{
		if (iPiece < p->nPieces)
		{
			size_t iSizeToRead = min((CHUNK_SIZE - iPieceOffset), (iSize - iSizeRead));

			memcpy(&((char*)buffer)[iSizeRead], &((char*)p->apiece[iPiece].p)[iPieceOffset], iSizeToRead);

			iSizeRead += iSizeToRead;

			iPiece++;
			iPieceOffset = 0;
		}
		else
		{
			__asm int 3;
		}
	}
}


void _S_WriteMemory(mem_t* p, size_t iOffset, size_t iSize, void* buffer)
{
	int iPiece;
	size_t iPieceOffset;
	size_t iSizeRead;

	iPiece = iOffset / CHUNK_SIZE;
	iPieceOffset = iOffset % CHUNK_SIZE;
	iSizeRead = 0;

	while (iSizeRead < iSize)
	{
		if (iPiece < p->nPieces)
		{
			size_t iSizeToRead = min((CHUNK_SIZE - iPieceOffset), (iSize - iSizeRead));

			memcpy(&((char*)p->apiece[iPiece].p)[iPieceOffset], &((char*)buffer)[iSizeRead], iSizeToRead);

			iSizeRead += iSizeToRead;

			iPiece++;
			iPieceOffset = 0;
		}
		else
		{
			__asm int 3;
		}
	}
}

