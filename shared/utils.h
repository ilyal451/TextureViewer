

#ifndef _UTILS_H
#define _UTILS_H

// the utility library

#include <windows.h>

#define DWORD_ALIGNED(v) ((((unsigned int)(v)) + 3) & (-4))
#define ALIGNED(v, a) ((((v) + ((a) - 1)) / (a)) * (a))
#define FLOOR(v, a) (((v) / (a)) * (a))
#define CEIL ALIGNED
#define ROUND(v, a) ((((v) + ((a) / 2)) / (a)) * (a))

#define FStrEq(a, b) (stricmp(a, b) == 0)
#define FStrEqW(a, b) (wcsicmp(a, b) == 0)
#define FWStrEq(a, b) (wcsicmp(a, b) == 0)

#define IsInRange(v, l, u) (((v) >= (l)) && ((v) < (u)))

#ifndef BADMEMHANDLER
#define BADMEMHANDLER
#endif

void* AllocMemory(size_t iSize);
void FreeMemory(void* p);
void* operator new (size_t s);
void operator delete (void* p);
char* AllocString(const char* psz);
wchar_t* AllocStringW(const wchar_t* psz);
wchar_t* AllocStringUnicode(const char* pszSrc);
void FreeString(void* psz);

#define PATH_SEPARATOR '\\' // don't change to '\', SH will get broken
#define IsPathSeparator(c) (((c) == '\\') || ((c) == '/'))
#define IsBlankFileName(s) (((s)[0] == '.') && (((s)[1] == '\0') || (((s)[1] == '.') && ((s)[2] == '\0'))))

int GeneralizePath(wchar_t* psz);

wchar_t* GetExtension(const wchar_t* psz);
void SetExtension(wchar_t* pszFileName, const wchar_t* pszExt);
wchar_t* GetFileName(const wchar_t* psz);

bool ParseBool(char* psz);

int CutChars(char* psz, const char* pszChars);
int CutCharsW(wchar_t* psz, const wchar_t* pszChars);
int ContractChars(char* psz, const char* pszChars, int cToChar);
int ContractCharsW(wchar_t* psz, const wchar_t* pszChars, int cToChar);
int CutComments(char* psz, const char* pszCommentStart, const char* pszCommentEnd);
int FindChar(int c, const char* pszChars);
int FindCharW(int c, const wchar_t* pszChars);
#define IsChar(c,psz) (FindChar((c),(psz))!=-1)
#define IsCharW(c,psz) (FindCharW((c),(psz))!=-1)
int Strip(char* psz, const char* pszChars);
int ExpandQuote(char* psz);

int ParseLine(int argcMax, char* argv[], char* psz, const char* pszDelimiters, char** ppszEndPtr);
int ParseLineW(int argcMax, wchar_t* argv[], wchar_t* psz, const wchar_t* pszDelimiters, wchar_t** ppszEndPtr);

typedef bool (*PFLINECALLBACK)(char* pszLine, void* param);
void ParseBuffer(char* buffer, int iSize, PFLINECALLBACK pfnLineCallback, void* param);

char* ReadFileToBuffer(const wchar_t* pszFileName, int* psize);
int WriteFileFromBuffer( const wchar_t* pszFileName, void* buffer, int iSize );

typedef struct bitmap_s
{
	int iWidth;
	int iHeight;
	int nBPP;
	int iPitch;
	unsigned char* pixels;
	int nColors;
	RGBQUAD* pal;
} bitmap_t;

bool LoadBitmapFromFile(const char* pszFileName, bitmap_t* pbmp);
bool LoadBitmapFromFileW(const wchar_t* pszFileName, bitmap_t* pbmp);
bool SaveBitmap(const char* pszFileName, bitmap_t* pbmp);
bool SaveBitmapW(const wchar_t* pszFileName, bitmap_t* pbmp);
bool SaveBitmap(const char* pszFileName, int iWidth, int iHeight, int nBPP, int iPitch, unsigned char* pixels, int nColors, RGBQUAD* pal);
bool SaveBitmapW(const wchar_t* pszFileName, int iWidth, int iHeight, int nBPP, int iPitch, unsigned char* pixels, int nColors, RGBQUAD* pal);
void FreeBitmap(bitmap_t* pbmp);

int RoundToPow2(int iValue);
int RaiseToLevel(int iValue, int iLevel);

#endif

