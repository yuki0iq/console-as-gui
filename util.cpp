#include "util.h"

#ifdef _WIN32
#include "Windows.h"
#else // ifdef _WIN32
#endif // def _WIN32

#include <stdlib.h>
#include <time.h>
#include <math.h>

using namespace std;


//=============================================================================
#ifdef _WIN32
  #define CHAR_SLASH     '\\'
  #define STR_SLASH      "\\"
  #define STR_DBLSLASH   "\\\\"
  #define STR_CURDIR     "\\.\\"
  #define STR_PARENTDIR  "\\..\\"
#else // ifdef _WIN32
  #define CHAR_SLASH     '/'
  #define STR_SLASH      "/"
  #define STR_DBLSLASH   "//"
  #define STR_CURDIR     "/./"
  #define STR_PARENTDIR  "/../"
#endif // def _WIN32


//=============================================================================
CUtil& CUtil::GetUtil()
{
	static CUtil util;
	return util;
}


//=============================================================================
string CUtil::GetPathForExecutable(char *pchArgv0/* = nullptr*/)
{
	string exeFileName = GetExecutableFileName(pchArgv0);

	string::size_type SlInd = exeFileName.find_last_of(CHAR_SLASH);
	if(SlInd != string::npos)
	{
		exeFileName.erase(SlInd+1);
	}
	else
	{
		exeFileName += CHAR_SLASH;
	}

	return exeFileName;
}


//=============================================================================
#ifdef _WIN32
//
//=============================================================================
string CUtil::GetExecutableFileName(char* /*pchArgv0 = 0*/)
{
	char chModuleFileName[MAX_PATH];
	memset(chModuleFileName, 0, MAX_PATH*sizeof(char));
	::GetModuleFileName(0, chModuleFileName, MAX_PATH);
	return chModuleFileName;
}
//
#else // ifdef _WIN32
//
//=============================================================================
#endif // def _WIN32


//=============================================================================
int CUtil::GetRandom(int iMin, int iMax)
{
	static bool bFirst = true;
	if(bFirst)
	{
		bFirst = false;
		srand( (unsigned)time(0) );
	}
	int iVal = (int)round( (double)rand() / (RAND_MAX + 1) * (iMax - iMin) + iMin );
	return iVal;
}


//=============================================================================
void CUtil::StrTrim(string &str)
{
	StrTrimLeft(str);
	StrTrimRight(str);
}

//=============================================================================
void CUtil::StrTrimLeft(string &str)
{
	const char *pchStr = str.c_str();
	int iLen = strlen(pchStr);
	for(int k = 0; k < iLen; k++)
	{
		if(' ' != *pchStr &&
			'\t' != *pchStr)
		{
			break;
		}
		pchStr++;
	}
	str = pchStr;
}


//=============================================================================
void CUtil::StrTrimRight(string &str)
{
	const char *pchStr = str.c_str();
	int iLen = strlen(pchStr);
	for(int k = iLen-1; k >= 0; k--)
	{
		if(' ' != *(pchStr+k) &&
			'\t' != *(pchStr+k))
		{
			break;
		}
		str[k] = '\0';
	}
}


//=============================================================================
string CUtil::GetLineFromFile(FILE *pFile, bool bTrimFirst, bool bTrimLast, bool &bEof)
{
	string sLine;

const int iBufSize = 1024;

	int iLen = 0;

	for(;;)
	{
		bEof = ( ! feof(pFile) ? false : true);
		if(bEof)
		{
			break;
		}

		char chBuf[iBufSize];
		memset(chBuf, 0, iBufSize);
		fgets(chBuf, iBufSize-1, pFile);
		iLen = strlen(chBuf);
		if(0 == iLen)
		{
			break;
		}
		bool bNewLine = false;
		if('\n' == chBuf[iLen-1])
		{
			chBuf[iLen-1] = '\0';
			bNewLine = true;
		}
		iLen = strlen(chBuf);
		if(0 == iLen)
		{
			break;
		}
		sLine += chBuf;
		if(bNewLine)
		{
			break;
		}
	}

	// Trim whitespaces, tabs.
	if(bTrimFirst)
	{
		STR_TRIM_L(sLine);
	} // if(bTrimFirst)
	if(bTrimLast)
	{
		STR_TRIM_R(sLine);
	} // if(bTrimLast)

	return sLine;
}
