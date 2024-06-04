

#define PARSER_ERROR ParserFatalError
void ParserFatalError(const char* fmt, ...);

extern const wchar_t* g_pszParserSource;
extern int g_iParserLine;

int ParseInt(char* psz);
unsigned int ParseUInt(char* psz);
unsigned int ParseHex(char* psz);
float ParseFloat(char* psz);
bool ParseBool(char* psz);
int ParseEnum(char** apszEnum, char* pszValue);

bool ParseConfig(wchar_t* pszIniFile, PFLINECALLBACK pfn);
