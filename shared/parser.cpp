
#include <stdio.h>
#include "..\\shared\\utils.h"

#include "parser.h"


int ParseInt(char* psz)
{
	int iValue;
	char* pszEndPtr;

	iValue = strtol(psz, &pszEndPtr, 10);

	if (*pszEndPtr != '\0')
	{
		PARSER_ERROR("a number is expected: %s", psz);
	}
	
	return iValue;
}


unsigned int ParseUInt(char* psz)
{
	unsigned int iValue;
	char* pszEndPtr;

	iValue = strtoul(psz, &pszEndPtr, 10);

	if (*pszEndPtr != '\0')
	{
		PARSER_ERROR("an unsigned number is expected: %s", psz);
	}
	
	return iValue;
}


unsigned int ParseHex(char* psz)
{
	unsigned int iValue;
	char* pszEndPtr;

	iValue = strtoul(psz, &pszEndPtr, 16);

	if (*pszEndPtr != '\0')
	{
		PARSER_ERROR("an unsigned hexadecimal number is expected: %s", psz);
	}
	
	return iValue;
}


float ParseFloat(char* psz)
{
	return (float)atof(psz);
}


bool ParseBool(char* psz)
{
	if (FStrEq(psz, "true"))
	{
		return true;
	}
	else if (FStrEq(psz, "false"))
	{
		return false;
	}
	else
	{
		return (ParseInt(psz) != 0);
	}
}


int ParseEnum(char** apszEnum, char* pszValue)
{
	int i;

	for (i = 0; apszEnum[i] != NULL; i++)
	{
		if (FStrEq(apszEnum[i], pszValue))
		{
			return i;
		}
	}

	return 0;
}


bool ParseConfig(wchar_t* pszIniFile, PFLINECALLBACK pfn)
{
	char* buffer;
	int iSize;

	buffer = ReadFileToBuffer(pszIniFile, &iSize);

	if (buffer != NULL)
	{
		iSize = CutComments(buffer, ";", NULL);
		iSize = CutChars(buffer, "\t ");
		
		if (iSize != 0)
		{
			g_pszParserSource = pszIniFile;
			g_iParserLine = 0;

			ParseBuffer(buffer, iSize, pfn, NULL);
		}

		return true;
	}

	return false;
}

