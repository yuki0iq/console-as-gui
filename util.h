#ifndef UTIL_H__AC06C7F1_FFBF_4251_86EA_FC3E9DEA784A__INCLUDED
#define UTIL_H__AC06C7F1_FFBF_4251_86EA_FC3E9DEA784A__INCLUDED


#include <string>


//=============================================================================
class CUtil
{
public:
	static CUtil& GetUtil();

private:
	CUtil() {}
public:
	~CUtil() {}

public:
	// Return path of executable.
	// WINDOWS: pchArgv0 is ignored.
	// LINUX: pchArgv0 must not be 0. Return "" (empty str, not 0) if can't get path.
	std::string GetPathForExecutable(char *pchArgv0 = nullptr);
	// Return full file name of executable.
	// WINDOWS: pchArgv0 is ignored.
	// LINUX: pchArgv0 must not be 0. Return "" (empty str, not 0) if can't get full file name.
	std::string GetExecutableFileName(char *pchArgv0 = nullptr);

	int GetRandom(int iMin, int iMax);

	void StrTrim(std::string &str);
	void StrTrimLeft(std::string &str);
	void StrTrimRight(std::string &str);

	// Return false if read error or end of file.
	std::string GetLineFromFile(FILE *pFile, bool bTrimFirst, bool bTrimLast, bool &bEof);
};
//
#define theUtil  ( CUtil::GetUtil() )
//
#define GET_RANDOM(min, max)  ( theUtil.GetRandom(min, max)  )
//
#define STR_TRIM(str)    ( theUtil.StrTrim(str)      )
#define STR_TRIM_L(str)  ( theUtil.StrTrimLeft(str)  )
#define STR_TRIM_R(str)  ( theUtil.StrTrimRight(str) )


#endif // ndef UTIL_H__AC06C7F1_FFBF_4251_86EA_FC3E9DEA784A__INCLUDED
