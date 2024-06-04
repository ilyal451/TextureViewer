
#include <windows.h>
#include <stdio.h>

#include "utils.h"


void* AllocMemory(size_t iSize)
{
	void* p = malloc(iSize);

	if (p == NULL)
	{
		BADMEMHANDLER(iSize);
	}

	return p;
}


void FreeMemory(void* p)
{
	free(p);
}


void* operator new (size_t s)
{
	return AllocMemory(s);
}


void operator delete (void* p)
{
	FreeMemory(p);
}


char* AllocString(const char* pszSrc)
{
	char* psz;
	int iSize;

	iSize = (int)strlen(pszSrc) + 1;
	psz = (char*)AllocMemory(iSize);
	memcpy(psz, pszSrc, iSize);

	return psz;
}


wchar_t* AllocStringW(const wchar_t* pszSrc)
{
	wchar_t* psz;
	int iSize;

	iSize = ((int)wcslen(pszSrc) + 1) * 2;
	psz = (wchar_t*)AllocMemory(iSize);
	memcpy(psz, pszSrc, iSize);

	return psz;
}


wchar_t* AllocStringUnicode(const char* pszSrc)
{
	wchar_t* psz;
	int iSize;

	psz = NULL;

	iSize = MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, NULL, 0);
	psz = (wchar_t*)AllocMemory((iSize+1)*sizeof(wchar_t));

	MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, psz, iSize);
	
	psz[iSize] = '\0';

	return psz;
}


void FreeString(void* psz)
{
	if (psz != NULL)
	{
		FreeMemory(psz);
	}
}


int GeneralizePath(wchar_t* psz)
{
	int i, j;

	for (i = 0, j = 0; psz[i] != '\0'; i++)
	{
		wchar_t c = psz[i];

		if (IsPathSeparator(c))
		{
			if (j > 0)
			{
				if (IsPathSeparator(psz[j-1]))
				{
					continue;
				}
			}
			
			c = PATH_SEPARATOR;
		}

		psz[j++] = c;
	}

	psz[j] = '\0';

	return j;
}


wchar_t* GetExtension(const wchar_t* psz)
{
	const wchar_t* pszExt;
	int i;

	pszExt = NULL;

	for (i = (int)wcslen(psz); i > 0; --i)
	{
		if (psz[i] == '.')
		{
			pszExt = &psz[i+1];
			
			break;
		}
		else if (IsPathSeparator(psz[i]))
		{
			break;
		}
	}

	return (wchar_t*)pszExt;
}


void SetExtension(wchar_t* pszFileName, const wchar_t* pszExt)
{
	wchar_t* psz;

	psz = GetExtension(pszFileName);

	if (psz == NULL)
	{
		psz = &pszFileName[wcslen(pszFileName)];
		*psz = '.';
		psz++;
	}

	wcscpy(psz, pszExt);
}


wchar_t* GetFileName(const wchar_t* psz)
{
	const wchar_t* pszFileName;
	int i;

	pszFileName = psz;

	for (i = 0; psz[i] != '\0'; i++)
	{
		if (IsPathSeparator(psz[i]))
		{
			pszFileName = &psz[i+1];
		}
	}

	return (wchar_t*)pszFileName;
}


int FindString(const char* psz, char** apsz, int nItems)
{
	int i;

	for (i = 0; i < nItems; i++)
	{
		if (FStrEq(psz, apsz[i]))
		{
			return i;
		}
	}

	return -1;
}


int WrapAround( int iItem, int nItems )
{
	while ( true )
	{
		if ( iItem < 0 )
		{
			iItem = nItems + iItem;
		}
		else if ( iItem >= nItems )
		{
			iItem = iItem - nItems;
		}
		else
		{
			break;
		}
	}

	return iItem;
}


int CutChars(char* psz, const char* pszChars)
{
	bool bSolid;
	int i;
	int j;
	int n;

	bSolid = false;
	j = 0;

	for (i = 0; psz[i] != '\0'; i++)
	{
		if (psz[i] == '\"')
		{
			bSolid = !bSolid;
		}
		else
		{
			if (!bSolid)
			{
				for (n = 0; pszChars[n] != '\0'; n++)
				{
					if (pszChars[n] == psz[i])
					{
						goto skip;
					}
				}
			}
		}

		psz[j++] = psz[i];

skip:

		;
	}

	psz[j] = '\0';

	return j;
}


int CutCharsW(wchar_t* psz, const wchar_t* pszChars)
{
	bool bSolid;
	int i;
	int j;
	int n;

	bSolid = false;
	j = 0;

	for (i = 0; psz[i] != '\0'; i++)
	{
		if (psz[i] == '\"')
		{
			bSolid = !bSolid;
		}
		else
		{
			if (!bSolid)
			{
				for (n = 0; pszChars[n] != '\0'; n++)
				{
					if (pszChars[n] == psz[i])
					{
						goto skip;
					}
				}
			}
		}

		psz[j++] = psz[i];

skip:

		;
	}

	psz[j] = '\0';

	return j;
}


int ContractChars(char* psz, const char* pszChars, int cToChar)
{
	bool bSolid;
	int i;
	int j;
	int n;

	bSolid = false;
	j = 0;

	for (i = 0; psz[i] != '\0'; i++)
	{
		if (psz[i] == '\"')
		{
			bSolid = !bSolid;
		}
		else
		{
			if (!bSolid)
			{
				for (n = 0; pszChars[n] != '\0'; n++)
				{
					if (pszChars[n] == psz[i])
					{
						if ((j == 0) || (psz[j-1] == cToChar))
						{
							goto skip;
						}
						else
						{
							psz[i] = cToChar;
						}
					}
				}
			}
		}

		psz[j++] = psz[i];

skip:

		;
	}

	if (j > 0)
	{
		if (psz[j-1] == cToChar)
		{
			j--;
		}
	}

	psz[j] = '\0';

	return j;
}


int ContractCharsW(wchar_t* psz, const wchar_t* pszChars, int cToChar)
{
	bool bSolid;
	int i;
	int j;
	int n;

	bSolid = false;
	j = 0;

	for (i = 0; psz[i] != '\0'; i++)
	{
		if (psz[i] == '\"')
		{
			bSolid = !bSolid;
		}
		else
		{
			if (!bSolid)
			{
				for (n = 0; pszChars[n] != '\0'; n++)
				{
					if (pszChars[n] == psz[i])
					{
						if ((j == 0) || (psz[j-1] == cToChar))
						{
							goto skip;
						}
						else
						{
							psz[i] = cToChar;
						}
					}
				}
			}
		}

		psz[j++] = psz[i];

skip:

		;
	}

	if (j > 0)
	{
		if (psz[j-1] == cToChar)
		{
			j--;
		}
	}

	psz[j] = '\0';

	return j;
}


int CutComments(char* psz, const char* pszCommentStart, const char* pszCommentEnd)
{
	bool bSolid;
	bool bComment;
	char* pszCur;
	int i;
	int j;
	int iC;

	bSolid = false;
	bComment = false;
	i = 0;
	j = 0;

	for ( ; psz[i] != '\0'; i++)
	{
		if (bComment)
		{
			if (pszCommentEnd == NULL)
			{
				if (((psz[i] == '\r') && (psz[i+1] == '\n')) || (psz[i] == '\n'))
				{
					bComment = false;
				}
				else
				{
					continue;
				}
			}
			else
			{
				pszCur = &psz[i];
				iC = 0;

				for ( ; pszCommentEnd[iC] != '\0'; iC++)
				{
					if (pszCur[iC] != pszCommentEnd[iC])
					{
						goto skip_end;
					}
				}

				i += iC;
				bComment = false;

skip_end:

				continue;
			}
		}

		if (psz[i] == '\"')
		{
			bSolid = !bSolid;
		}

		if (!bSolid)
		{
			pszCur = &psz[i];
			iC = 0;

			for ( ; pszCommentStart[iC] != '\0'; iC++)
			{
				if (pszCur[iC] != pszCommentStart[iC])
				{
					goto skip_start;
				}
			}

			i += iC;
			bComment = true;

			continue;
		}

skip_start:

		psz[j++] = psz[i];
	}

	psz[j] = '\0';

	return j;
}


int FindChar(int c, const char* psz)
{
	int i;

	for (i = 0; psz[i] != '\0'; i++)
	{
		if (psz[i] == c)
		{
			return i;
		}
	}

	return -1;
}


int FindCharW(int c, const wchar_t* psz)
{
	int i;

	for (i = 0; psz[i] != '\0'; i++)
	{
		if (psz[i] == c)
		{
			return i;
		}
	}

	return -1;
}


int Strip(char* psz, const char* pszChars)
{
	int i;
	int j;

	for (i = 0; psz[i] != '\0'; i++)
	{
		if (!IsChar(psz[i], pszChars))
		{
			break;
		}
	}

	for (j = 0; psz[i] != '\0'; i++, j++)
	{
		psz[j] = psz[i];
	}

	for ( ; j > 0; j--)
	{
		if (!IsChar(psz[j-1], pszChars))
		{
			break;
		}
	}

	psz[j] = '\0';

	return j;
}


int ExpandQuote(char* psz)
{
	int i = (int)strlen(psz);
	int j;

	if (i >= 2)
	{
		if ((psz[0] == '\"') && (psz[i-1] == '\"'))
		{
			i -= 2;

			for (j = i; j > 0; j--)
			{
				psz[j-1] = psz[j];
			}

			psz[i] = '\0';
		}
	}

	return i;
}


int ParseLine(int argcMax, char* argv[], char* psz, const char* pszDelimiters, char** ppszEndPtr)
{
	int argc;
	char* pszIdent;
	char* pszQuoteStart;
	char* pszQuoteEnd;
	bool bSolid;
	bool bMoreArgs;
	int nQuotes;
	int c;
	int i;
	//int n;

	argc = 0;
	pszIdent = psz;
	pszQuoteStart = NULL;
	pszQuoteEnd = NULL;
	bSolid = false;
	bMoreArgs = false;
	nQuotes = 0;

	for (i = 0; ; i++)
	{
		c = psz[i];

		if (c == '\"')
		{
			if (!bSolid)
			{
				pszQuoteStart = &psz[i];
				bSolid = TRUE;
			}
			else
			{
				pszQuoteEnd = &psz[i];
				bSolid = FALSE;
				nQuotes++;
			}

			continue;
		}

		if (!bSolid)
		{
			if ((c == '\0') || IsChar(c, pszDelimiters))
			{
				if (argc < argcMax)
				{
					psz[i] = '\0';

					// cut quotes
					if ((pszQuoteStart == pszIdent) && (pszQuoteEnd == &psz[i-1]))
					{
						if (nQuotes == 1)   // filter multiple quotes
						{
							pszIdent++;
							*pszQuoteStart = '\0';
							*pszQuoteEnd = '\0';
						}
					}

					argv[argc] = pszIdent;
					pszIdent = &psz[i+1];
				}
				else
				{
					if (argcMax != 0)
					{
						bMoreArgs = true;
						break;
					}
				}

				argc++;

				nQuotes = 0;
			}
		}

		if (c == '\0')
		{
			break;
		}
	}

	if (ppszEndPtr != NULL)
	{
		if (bMoreArgs)
		{
			(*ppszEndPtr) = pszIdent;
		}
		else
		{
			(*ppszEndPtr) = NULL;
		}
	}

	return argc;
}


int ParseLineW(int argcMax, wchar_t* argv[], wchar_t* psz, const wchar_t* pszDelimiters, wchar_t** ppszEndPtr)
{
	int argc;
	wchar_t* pszIdent;
	wchar_t* pszQuoteStart;
	wchar_t* pszQuoteEnd;
	bool bSolid;
	bool bMoreArgs;
	int nQuotes;
	int c;
	int i;
	//int n;

	argc = 0;
	pszIdent = psz;
	pszQuoteStart = NULL;
	pszQuoteEnd = NULL;
	bSolid = false;
	bMoreArgs = false;
	nQuotes = 0;

	for (i = 0; ; i++)
	{
		c = psz[i];

		if (c == '\"')
		{
			if (!bSolid)
			{
				pszQuoteStart = &psz[i];
				bSolid = TRUE;
			}
			else
			{
				pszQuoteEnd = &psz[i];
				bSolid = FALSE;
				nQuotes++;
			}

			continue;
		}

		if (!bSolid)
		{
			if ((c == '\0') || IsCharW(c, pszDelimiters))
			{
				if (argc < argcMax)
				{
					psz[i] = '\0';

					// cut quotes
					if ((pszQuoteStart == pszIdent) && (pszQuoteEnd == &psz[i-1]))
					{
						if (nQuotes == 1)   // filter multiple quotes
						{
							pszIdent++;
							*pszQuoteStart = '\0';
							*pszQuoteEnd = '\0';
						}
					}

					argv[argc] = pszIdent;
					pszIdent = &psz[i+1];
				}
				else
				{
					if (argcMax != 0)
					{
						bMoreArgs = true;
						break;
					}
				}

				argc++;

				nQuotes = 0;
			}
		}

		if (c == '\0')
		{
			break;
		}
	}

	if (ppszEndPtr != NULL)
	{
		if (bMoreArgs)
		{
			(*ppszEndPtr) = pszIdent;
		}
		else
		{
			(*ppszEndPtr) = NULL;
		}
	}

	return argc;
}


void ParseBuffer(char* buffer, int iSize, PFLINECALLBACK pfnLineCallback, void* param)
{
	char* pszLine;
	int i;

	pszLine = buffer;

	for (i = 0; i <= iSize; i++)
	{
		if ((buffer[i] == '\n') || (buffer[i] == '\0'))
		{
			buffer[i] = '\0';

			if (!pfnLineCallback(pszLine, param))
			{
				break;
			}

			pszLine = &buffer[i+1];
		}
		else if (buffer[i] == '\r')
		{
			buffer[i] = '\0';
		}
	}
}


char* ReadFileToBuffer(const wchar_t* pszFileName, int* piSize)
{
	char* buffer = NULL;
	int iSize;
	HANDLE hFile;

	hFile = CreateFileW(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		iSize = GetFileSize(hFile, NULL);

		buffer = (char*)AllocMemory(iSize + 1);

		if (buffer != NULL)
		{
			DWORD iRead = 0;

			ReadFile(hFile, buffer, iSize, &iRead, NULL);
			
			if (iRead == iSize)
			{
				buffer[iSize] = '\0';
			}
			else
			{
				FreeMemory(buffer);
				buffer = NULL;
			}
		}

		*piSize = iSize;

		CloseHandle(hFile);
	}

	return buffer;
}


int WriteFileFromBuffer( const wchar_t* pszFileName, void* buffer, int iSize )
{
	HANDLE hFile = CreateFileW( pszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		DWORD iWritten = 0;
		WriteFile( hFile, buffer, iSize, &iWritten, NULL );

		CloseHandle( hFile );

		if ( iWritten == iSize )
		{
			return 1;
		}
	}

	return 0;
}


// CutComments(buffer, "/*", "*/");
// CutComments(buffer, "//", NULL);
// CutComments(buffer, ";", NULL);
// ContractChars(buffer, "\t ", ' ');

bool ParseFile(const wchar_t* pszFileName, PFLINECALLBACK pfnLineCallback, void* param)
{
	char* buffer;
	int iSize;

	buffer = ReadFileToBuffer(pszFileName, &iSize);

	if (buffer != NULL)
	{
		ParseBuffer(buffer, iSize, pfnLineCallback, param);

		FreeMemory(buffer);

		return true;
	}

	return false;
}


bool LoadBitmapFromFile(const char* pszFileName, bitmap_t* pbmp)
{
	FILE* stream;
	bool bSuccess = false;
	int i;

	memset(pbmp, 0, sizeof(bitmap_t));

	stream = fopen(pszFileName, "rb");
	if (stream != NULL)
	{
		BITMAPFILEHEADER bmf;
		BITMAPINFOHEADER bmi;

		fread(&bmf, sizeof(BITMAPFILEHEADER), 1, stream);
		if (bmf.bfType == 'MB')
		{
			fread(&bmi, sizeof(BITMAPINFOHEADER), 1, stream);
			if (bmi.biCompression == BI_RGB)
			{
				if ((bmi.biBitCount == 8) || (bmi.biBitCount == 24) || (bmi.biBitCount == 32))
				{
					pbmp->iWidth = bmi.biWidth;
					pbmp->iHeight = (bmi.biHeight > 0)? bmi.biHeight: -bmi.biHeight;
					pbmp->nBPP = bmi.biBitCount / 8;
					pbmp->iPitch = DWORD_ALIGNED(pbmp->iWidth * pbmp->nBPP);
					pbmp->pixels = (unsigned char*)AllocMemory(pbmp->iPitch * pbmp->iHeight);

					if (pbmp->nBPP == 1)
					{
						pbmp->nColors = (bmi.biClrUsed == 0)? 256: bmi.biClrUsed;
						pbmp->pal = (RGBQUAD*)AllocMemory(pbmp->nColors * sizeof(RGBQUAD));
						fread(pbmp->pal, pbmp->nColors * sizeof(RGBQUAD), 1, stream);
					}

					fseek(stream, bmf.bfOffBits, SEEK_SET);

					if (bmi.biHeight > 0)
					{
						for (i = 0; i < pbmp->iHeight; i++)
						{
							fread(&pbmp->pixels[((pbmp->iHeight - 1) - i) * pbmp->iPitch], pbmp->iPitch, 1, stream);
						}
					}
					else
					{
						for (i = 0; i < pbmp->iHeight; i++)
						{
							fread(&pbmp->pixels[i * pbmp->iPitch], pbmp->iPitch, 1, stream);
						}
					}

					bSuccess = true;
				}
			}
		}

		fclose(stream);
	}

	return bSuccess;
}

/*
void GenerateGrayscalePal(RGBQUAD* pa)
{
	int i;

	for (i = 0; i < 256; i++)
	{
		pa[i].rgbRed = pa[i].rgbGreen = pa[i].rgbRed = (unsigned)i;
		pa[i].rgbReserved = 255;
	}
}
*/


void _SaveBitmap(FILE* stream, bitmap_t* pbmp)
{
	BITMAPFILEHEADER bmf;
	BITMAPINFOHEADER bmi;
	int iDataSize;
	int iPaletteSize;
	int iHeaderSize;
	int iFileSize;
	int i;

	iPaletteSize = (pbmp->nBPP == 1)? pbmp->nColors * sizeof(RGBQUAD): 0;
	iHeaderSize = DWORD_ALIGNED(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + iPaletteSize);
	iDataSize = pbmp->iPitch * pbmp->iHeight;
	iFileSize = iHeaderSize + iDataSize;

	memset(&bmf, 0, sizeof(BITMAPFILEHEADER));
	bmf.bfType = 'MB';
	bmf.bfSize = iFileSize;
	bmf.bfOffBits = iHeaderSize;
	fwrite(&bmf, sizeof(BITMAPFILEHEADER), 1, stream);

	memset(&bmi, 0, sizeof(BITMAPINFOHEADER));
	bmi.biSize = sizeof(BITMAPINFOHEADER);
	bmi.biWidth = pbmp->iWidth;
	bmi.biHeight = -pbmp->iHeight;
	bmi.biPlanes = 1;
	bmi.biBitCount = pbmp->nBPP * 8;
	bmi.biClrUsed = (pbmp->nColors == 256)? 0: pbmp->nColors;
	fwrite(&bmi, sizeof(BITMAPINFOHEADER), 1, stream);

	if (pbmp->nBPP == 1)
	{
		// if (pbmp->pal != NULL)
		fwrite(pbmp->pal, iPaletteSize, 1, stream);
	}

	fseek(stream, iHeaderSize, SEEK_SET);
	for (i = 0; i < pbmp->iHeight; i++)
	{
		fwrite(&pbmp->pixels[i * pbmp->iPitch], pbmp->iPitch, 1, stream);
	}
}


bool SaveBitmap(const char* pszFileName, bitmap_t* pbmp)
{
	FILE* stream;

	stream = fopen(pszFileName, "wb");
	if (stream != NULL)
	{
		_SaveBitmap( stream, pbmp );
		
		fclose(stream);

		return true;
	}

	return false;
}


bool SaveBitmapW(const wchar_t* pszFileName, bitmap_t* pbmp)
{
	FILE* stream;

	stream = _wfopen(pszFileName, L"wb");
	if (stream != NULL)
	{
		_SaveBitmap( stream, pbmp );
		
		fclose(stream);

		return true;
	}

	return false;
}


void FreeBitmap(bitmap_t* pbmp)
{
	if (pbmp->pixels != NULL)
	{
		FreeMemory(pbmp->pixels);
	}

	if (pbmp->nBPP == 1)
	{
		if (pbmp->pal != NULL)
		{
			FreeMemory(pbmp->pal);
		}
	}
}


bool SaveBitmap(const char* pszFileName, int iWidth, int iHeight, int nBPP, int iPitch, unsigned char* pixels, int nColors, RGBQUAD* pal)
{
	bitmap_t bmp =
	{
		iWidth,
		iHeight,
		nBPP,
		iPitch,
		pixels,
		nColors,
		pal
	};

	return SaveBitmap(pszFileName, &bmp);
}


bool SaveBitmapW(const wchar_t* pszFileName, int iWidth, int iHeight, int nBPP, int iPitch, unsigned char* pixels, int nColors, RGBQUAD* pal)
{
	bitmap_t bmp =
	{
		iWidth,
		iHeight,
		nBPP,
		iPitch,
		pixels,
		nColors,
		pal
	};

	return SaveBitmapW(pszFileName, &bmp);
}



int RoundToPow2(int iValue)
{
	int iResult = 1;

	while (iResult < iValue)
	{
		iResult = iResult * 2;
	}

	return iResult;
}


int RaiseToLevel(int iValue, int iLevel)
{
	int iResult;

	if (iLevel > 0)
	{
		iResult = iValue << iLevel;
	}
	else if (iLevel < 0)
	{
		iResult = iValue >> (-iLevel);
	}
	else
	{
		iResult = iValue;
	}

	return iResult;
}

