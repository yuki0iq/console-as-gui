#include "console.h"
#include "util.h"

#ifdef WIN32
#include <windows.h>
#include <wincon.h>
#endif // def WIN32

#include <vector>
#include <string>

#include <io.h>
#include <fcntl.h>

using namespace std;


/*****/
/*
// LINUX COLOR TEXT
//
Шаблон для использования в современных командных оболочках и языках программирования таков:
ESC[...m
\x1b[...m
\033[...m
Это ESCAPE-последовательность, где \x1b (\033) обозначает символ ESC (десятичный ASCII код 27),
а вместо "..." подставляются значения из таблицы, приведенной ниже,
причем они могут комбинироваться, тогда нужно их перечислить через точку с запятой.

атрибуты
0	нормальный режим
1	жирный
4	подчеркнутый
5	мигающий
7	инвертированные цвета
8	невидимый

цвет текста
30	черный
31	красный
32	зеленый
33	желтый
34	синий
35	пурпурный
36	голубой
37	белый

цвет фона
40	черный
41	красный
42	зеленый
43	желтый
44	синий
45	пурпурный
46	голубой
47	белый


Теперь несколько примеров. Все это можно опробовать, введя в консольном окне echo
-e "текст примера".
"\x1b[31mTest\x1b[0m"
"\x1b[37;43mTest\x1b[0m"
"\x1b[4;35mTest\x1b[0m"
"\x1b[1;31mСтрока\x1b[0m с \x1b[4;35;42mразными\x1b[0m \x1b[34;45mстилями\x1b[0m \x1b[1;33mоформления\x1b[0m"

Обратите внимание, что во всех трех случаях после слова Test идет последовательность
\x1b[0m
, которая просто сбрасывает стиль оформления на стандартный.
To reset colors to their defaults, use ESC[39;49m (not supported on some terminals), or reset all attributes with ESC[0m.

The original specification only had 8 colors (see "Linux Color Table.PNG"), and just gave them names.
The SGR parameters 30-37 selected the foreground color, while 40-47 selected the background.
Quite a few terminals implemented "bold" (SGR code 1) as a brighter color rather than a different font,
thus providing 8 additional foreground colors. Usually you could not get these as background colors,
though sometimes inverse video (SGR code 7) would allow that.
Examples: to get black letters on white background use ESC[30;47m, to get red use ESC[31m, to get bright red use ESC[31;1m.
*/
/*****/


//*****************************************************************************
//  CConsolePrivateData
//
//=============================================================================
class CConsolePrivateData
{
public:
	static CConsolePrivateData* GetInstance();

private:
	CConsolePrivateData();
public:
	~CConsolePrivateData();

public:
	// Must be as first call of this.
	bool Attach();
	// Must be as last call of this.
	bool Detach();

public:
	bool SetTitle(const string& sText);
	void Clear();

	void RestoreColors();
	void RestoreColorText();
	void RestoreColorBkgnd();

	void GetColors(COLOR &rColorText, COLOR &rColorBkgnd);
	void SetColors(COLOR colorText, COLOR colorBkgnd);

	bool GetCursorPos(int &X, int &Y);
	bool SetCursorPos(int X, int Y);

private:
#ifdef WIN32
	WORD COLORWIN_FROM_COLOR(COLOR color);
	COLOR COLOR_FROM_COLORWIN(WORD winColor);
#else // ifdef WIN32
	string COLORLINUX_FROM_COLOR(COLOR color);
#endif // def WIN32

private:
	bool m_bConsoleAllocated;
	int m_iAttachConsole;
#ifdef WIN32
	HANDLE m_hConsole;
#endif // def WIN32
	COLOR m_colorText;  // LIGHT_GRAY
	COLOR m_colorBkgnd; // BLACK
};
//
#define theCPD  ( CConsolePrivateData::GetInstance() )


//=============================================================================
CConsolePrivateData* CConsolePrivateData::GetInstance()
{
	static CConsolePrivateData consolePrivateData;
	return &consolePrivateData;
}


#ifdef WIN32
//=============================================================================
CConsolePrivateData::CConsolePrivateData()
	: m_bConsoleAllocated(false)
	, m_iAttachConsole(0)
	, m_hConsole(0)
	, m_colorText(LIGHT_GRAY)
	, m_colorBkgnd(BLACK)
{
}
#else // ifdef WIN32
//=============================================================================
CConsolePrivateData::CConsolePrivateData()
	: m_bConsoleAllocated(false)
	, m_iAttachConsole(0)
	, m_colorText(LIGHT_GRAY)
	, m_colorBkgnd(BLACK)
{
}
#endif // def WIN32


//=============================================================================
CConsolePrivateData::~CConsolePrivateData()
{
	Detach();
}


//=============================================================================
bool CConsolePrivateData::Attach()
{
	if(m_iAttachConsole > 0)
	{
		m_iAttachConsole++;
		return true;
	}
	// (m_iAttachConsole == 0)

#ifdef WIN32
	BOOL bAttachConsole = FALSE;
	BOOL bAllocConsole  = FALSE;

	bAttachConsole = ::AttachConsole(ATTACH_PARENT_PROCESS);
	if( ! bAttachConsole)
	{
		DWORD dwErr = ::GetLastError();
		if(ERROR_ACCESS_DENIED == dwErr) // (5) == process is already attached to a console
		{
			bAttachConsole = TRUE;
		}
		else
		{
			bAllocConsole = ::AllocConsole();
		}
	}
	if( ! bAttachConsole && ! bAllocConsole)
	{
		return false;
	}

	if(bAllocConsole)
	{
		m_bConsoleAllocated = true;

		// This magic code is needed to corrct work printf, std::cout...

		int hCrtIn = _open_osfhandle((long)::GetStdHandle(STD_INPUT_HANDLE), O_TEXT);
		*stdin = *(::_fdopen(hCrtIn, "r"));
		//stdin->token = (unsigned char)stdin;
		/*int err1 = */::setvbuf(stdin, NULL, _IONBF, 0);

		int hCrtOut = _open_osfhandle((long)::GetStdHandle(STD_OUTPUT_HANDLE), O_TEXT);
		*stdout = *(::_fdopen(hCrtOut, "w"));
		//stdout->token = (unsigned char)stdout;
		/*int err2 = */::setvbuf(stdout, NULL, _IONBF, 0);
		//
		*stderr = *(::_fdopen(hCrtOut, "w"));
		//stderr->token = (unsigned char)stderr;
		/*int err3 = */::setvbuf(stderr, NULL, _IONBF, 0);
	}

	if(0 == m_iAttachConsole)
	{
		m_hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);
		GetColors(m_colorText, m_colorBkgnd);
	}
#else // ifdef WIN32
#endif // def WIN32

	m_iAttachConsole++;

	return true;
}


//=============================================================================
bool CConsolePrivateData::Detach()
{
	if(m_iAttachConsole > 0)
	{
		m_iAttachConsole--;
#ifdef WIN32
		if(0 == m_iAttachConsole && m_bConsoleAllocated)
		{
			::FreeConsole();
		}
#else // ifdef WIN32
#endif // def WIN32
	}
	return true;
}


//=============================================================================
bool CConsolePrivateData::SetTitle(const string& sText)
{
#ifdef WIN32
	bool bRetCode = false;
	int iNumChars = ::MultiByteToWideChar(CP_ACP, 0, sText.c_str(), -1, NULL, 0);
	vector<wchar_t> buffer(iNumChars);
	int iNumCharsWritten = ::MultiByteToWideChar(CP_ACP, 0, sText.c_str(), -1, &(buffer[0]), iNumChars);
	if(iNumCharsWritten == iNumChars)
	{
		wstring wsTitle(buffer.begin(), buffer.end());
		BOOL bRes = ::SetConsoleTitleW(wsTitle.c_str());
		bRetCode = (bRes ? true : false);
	}
	return bRetCode;
#else // ifdef WIN32
	string sNewTitle = "title ";
	sNewTitle += sText;
	system(sNewTitle.c_str());
	return true;
#endif // def WIN32
}
//=============================================================================
void CConsolePrivateData::Clear()
{
#ifdef WIN32
	system("cls");
#else // ifdef WIN32
	system("clear");
#endif // def WIN32
}


//=============================================================================
void CConsolePrivateData::RestoreColors()
{
	SetColors(m_colorText, m_colorBkgnd);
}
//=============================================================================
void CConsolePrivateData::RestoreColorText()
{
	COLOR colorText  = LIGHT_GRAY;
	COLOR colorBkgnd = BLACK;
	GetColors(colorText, colorBkgnd);
	SetColors(m_colorText, colorBkgnd);
}
//=============================================================================
void CConsolePrivateData::RestoreColorBkgnd()
{
	COLOR colorText  = LIGHT_GRAY;
	COLOR colorBkgnd = BLACK;
	GetColors(colorText, colorBkgnd);
	SetColors(colorText, m_colorBkgnd);
}


//=============================================================================
void CConsolePrivateData::GetColors(COLOR &rColorText, COLOR &rColorBkgnd)
{
#ifdef WIN32
	if(0 != m_hConsole)
	{
		CONSOLE_SCREEN_BUFFER_INFO csbi = { 0 };
		BOOL bRes = ::GetConsoleScreenBufferInfo(m_hConsole, &csbi);
		if(bRes)
		{
			WORD winColorText  = (0x0F & csbi.wAttributes);
			WORD winColorBkgnd = (0x0F & (csbi.wAttributes >> 4));

			rColorText  = COLOR_FROM_COLORWIN(winColorText );
			rColorBkgnd = COLOR_FROM_COLORWIN(winColorBkgnd);
		}
	}
#else // ifdef WIN32
	rColorText  = m_colorText;
	rColorBkgnd = m_colorBkgnd;
#endif // def WIN32
}
//=============================================================================
void CConsolePrivateData::SetColors(COLOR colorText, COLOR colorBkgnd)
{
#ifdef WIN32
	if(0 != m_hConsole)
	{
		WORD winColorText  = COLORWIN_FROM_COLOR(colorText);
		WORD winColorBkgnd = COLORWIN_FROM_COLOR(colorBkgnd);

		WORD wAttr = /*(WORD)*/(0xF0 & (winColorBkgnd << 4));
		wAttr |= /*(WORD)*/(0x0F & winColorText);
		BOOL bRes = ::SetConsoleTextAttribute(m_hConsole, wAttr);
		int r = 0;
	}
#else // ifdef WIN32
// Using ANSI escape sequence, where ESC[colT;colBm set colT, colB: "\x1b[37;43mtext".
	string str = "\x1b[";                     // "\x1b["
	str += COLORLINUX_FROM_COLOR(colorText);  // "\x1b[colT"
	str += ";";                               // "\x1b[colT;"
	str += COLORLINUX_FROM_COLOR(colorBkgnd); // "\x1b[colT;colB"
	str += "m";                               // "\x1b[colT;colBm"
	cout << str.c_str();
	//
	m_colorText  = colorText;
	m_colorBkgnd = colorBkgnd;
#endif // def WIN32
}


//=============================================================================
bool CConsolePrivateData::GetCursorPos(int &X, int &Y)
{
#ifdef WIN32
	if(0 == m_hConsole)
	{
		return false;
	}
	CONSOLE_SCREEN_BUFFER_INFO csbi = { 0 };
	BOOL bRes = ::GetConsoleScreenBufferInfo(m_hConsole, &csbi);
	if(bRes)
	{
		X = csbi.dwCursorPosition.X;
		Y = csbi.dwCursorPosition.Y;
	}
	return (bRes ? true : false);
#else // ifdef WIN32
	return false;
#endif // def WIN32
}
//=============================================================================
bool CConsolePrivateData::SetCursorPos(int X, int Y)
{
#ifdef WIN32
	if(0 == m_hConsole)
	{
		return false;
	}
	COORD dwCursorPos = { (SHORT)X, (SHORT)Y };
	BOOL bRes = ::SetConsoleCursorPosition(m_hConsole, dwCursorPos);
	return (bRes ? true : false);
#else // ifdef WIN32
// Using ANSI escape sequence, where ESC[y;xH moves curser to row y, col x: "\x1b[Y;XHtext\n".
	string str = "\x1b["; // "\x1b["
	str += to_string(Y);  // "\x1b[Y"
	str += ";";           // "\x1b[Y;"
	str += to_string(X);  // "\x1b[Y;X"
	str += "H";           // "\x1b[Y;XH"
	cout << str.c_str();
	return true;
#endif // def WIN32
}


#ifdef WIN32
//=============================================================================
WORD CConsolePrivateData::COLORWIN_FROM_COLOR(COLOR color)
{
	WORD winColor = 0;
	switch(color)
	{
		case DARK_RED:       winColor = FOREGROUND_RED;                                                             break;
		case DARK_GREEN:     winColor = FOREGROUND_GREEN;                                                           break;
		case OLIVE:          winColor = FOREGROUND_RED | FOREGROUND_GREEN;                                          break;
		case DARK_BLUE:      winColor = FOREGROUND_BLUE;                                                            break;
		case DARK_PINK:      winColor = FOREGROUND_RED | FOREGROUND_BLUE;                                           break;
		case DARK_BLUEGREEN: winColor = FOREGROUND_GREEN | FOREGROUND_BLUE;                                         break;
		case LIGHT_GRAY:     winColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;                        break;
		case GRAY:           winColor = FOREGROUND_INTENSITY;                                                       break;
		case RED:            winColor = FOREGROUND_RED | FOREGROUND_INTENSITY;                                      break;
		case GREEN:          winColor = FOREGROUND_GREEN | FOREGROUND_INTENSITY;                                    break;
		case YELLOW:         winColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;                   break;
		case BLUE:           winColor = FOREGROUND_BLUE | FOREGROUND_INTENSITY;                                     break;
		case PINK:           winColor = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;                    break;
		case BLUEGREEN:      winColor = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;                  break;
		case WHITE:          winColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
		case BLACK:
		default:             winColor = 0;                                                                          break;
	}
	return winColor;
}
//=============================================================================
COLOR CConsolePrivateData::COLOR_FROM_COLORWIN(WORD winColor)
{
	COLOR color = BLACK;
	switch(winColor)
	{
		case FOREGROUND_RED:                                                               color = DARK_RED;       break;
		case FOREGROUND_GREEN:                                                             color = DARK_GREEN;     break;
		case (FOREGROUND_RED | FOREGROUND_GREEN):                                          color = OLIVE;          break;
		case FOREGROUND_BLUE:                                                              color = DARK_BLUE;      break;
		case (FOREGROUND_RED | FOREGROUND_BLUE):                                           color = DARK_PINK;      break;
		case (FOREGROUND_GREEN | FOREGROUND_BLUE):                                         color = DARK_BLUEGREEN; break;
		case (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE):                        color = LIGHT_GRAY;     break;
		case FOREGROUND_INTENSITY:                                                         color = GRAY;           break;
		case (FOREGROUND_RED | FOREGROUND_INTENSITY):                                      color = RED;            break;
		case (FOREGROUND_GREEN | FOREGROUND_INTENSITY):                                    color = GREEN;          break;
		case (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY):                   color = YELLOW;         break;
		case (FOREGROUND_BLUE | FOREGROUND_INTENSITY):                                     color = BLUE;           break;
		case (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY):                    color = PINK;           break;
		case (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY):                  color = BLUEGREEN;      break;
		case (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY): color = WHITE;          break;
		default: break;
	}
	return color;
}
#else // ifdef WIN32
//=============================================================================
string CConsolePrivateData::COLORLINUX_FROM_COLOR(COLOR color)
{
	string linuxColor("0");
	switch(color)
	{
		case DARK_RED:       linuxColor = "FOREGROUND_RED";                                                             break;
		case DARK_GREEN:     linuxColor = "FOREGROUND_GREEN";                                                           break;
		case OLIVE:          linuxColor = "FOREGROUND_RED | FOREGROUND_GREEN";                                          break;
		case DARK_BLUE:      linuxColor = "FOREGROUND_BLUE";                                                            break;
		case DARK_PINK:      linuxColor = "FOREGROUND_RED | FOREGROUND_BLUE";                                           break;
		case DARK_BLUEGREEN: linuxColor = "FOREGROUND_GREEN | FOREGROUND_BLUE";                                         break;
		case LIGHT_GRAY:     linuxColor = "FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE";                        break;
		case GRAY:           linuxColor = "FOREGROUND_INTENSITY";                                                       break;
		case RED:            linuxColor = "FOREGROUND_RED | FOREGROUND_INTENSITY";                                      break;
		case GREEN:          linuxColor = "FOREGROUND_GREEN | FOREGROUND_INTENSITY";                                    break;
		case YELLOW:         linuxColor = "FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY";                   break;
		case BLUE:           linuxColor = "FOREGROUND_BLUE | FOREGROUND_INTENSITY";                                     break;
		case PINK:           linuxColor = "FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY";                    break;
		case BLUEGREEN:      linuxColor = "FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY";                  break;
		case WHITE:          linuxColor = "FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY"; break;
		case BLACK:
		default:             linuxColor = "0";                                                                          break;
	}
	return linuxColor;
}
#endif // def WIN32


//*****************************************************************************
//  CConsole
//
//=============================================================================
CConsole::CConsole()
{
	theCPD->Attach();
}


//=============================================================================
CConsole::~CConsole()
{
	theCPD->Detach();
}


//=============================================================================
bool CConsole::SetTitle(const string& sText)
{
	return theCPD->SetTitle(sText);
}


//=============================================================================
void CConsole::Clear()
{
	return theCPD->Clear();
}


//=============================================================================
void CConsole::RestoreColors()
{
	theCPD->RestoreColors();
}
//=============================================================================
void CConsole::RestoreColorText()
{
	theCPD->RestoreColorText();
}
//=============================================================================
void CConsole::RestoreColorBkgnd()
{
	theCPD->RestoreColorBkgnd();
}


//=============================================================================
void CConsole::GetColors(COLOR &rColorText, COLOR &rColorBkgnd)
{
	theCPD->GetColors(rColorText, rColorBkgnd);
}
//=============================================================================
COLOR CConsole::GetColorText()
{
	COLOR colorText  = LIGHT_GRAY;
	COLOR colorBkgnd = BLACK;
	GetColors(colorText, colorBkgnd);
	return colorText;
}
//=============================================================================
COLOR CConsole::GetColorBkgnd()
{
	COLOR colorText  = LIGHT_GRAY;
	COLOR colorBkgnd = BLACK;
	GetColors(colorText, colorBkgnd);
	return colorBkgnd;
}


//=============================================================================
void CConsole::SetColors(COLOR colorText, COLOR colorBkgnd)
{
	theCPD->SetColors(colorText, colorBkgnd);
}
//=============================================================================
void CConsole::SetColorText(COLOR colorText)
{
	COLOR colorBkgnd = GetColorBkgnd();
	SetColors(colorText, colorBkgnd);
}
//=============================================================================
void CConsole::SetColorBkgnd(COLOR colorBkgnd)
{
	COLOR colorText = GetColorText();
	SetColors(colorText, colorBkgnd);
}


//=============================================================================
void CConsole::OutputColorText(const string& sText, COLOR colorText, COLOR colorBkgnd, bool bRestore)
{
	COLOR oldColorText  = LIGHT_GRAY;
	COLOR oldColorBkgnd = BLACK;
	if(bRestore)
	{
		oldColorText  = GetColorText();
		oldColorBkgnd = GetColorBkgnd();
	}

	SetColorText(colorText);
	SetColorBkgnd(colorBkgnd);

	cout << sText.c_str();

	if(bRestore)
	{
		SetColorText(oldColorText);
		SetColorBkgnd(oldColorBkgnd);
	}
}


//=============================================================================
string CConsole::GetTrimmedLine()
{
	return GetLine(true, true);
}
//
//=============================================================================
string CConsole::GetLine(bool bTrimFirst, bool bTrimLast)
{
	string sRetStr;

const int iBufSize = 1024;

	int iLen = 0;

	while(true)
	{
		char chBuf[iBufSize];
		memset(chBuf, 0, iBufSize);
		fgets(chBuf, iBufSize-1, stdin);
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
		sRetStr += chBuf;
		if(bNewLine)
		{
			break;
		}
	}

	// Trim whitespaces, tabs.
	if(bTrimFirst)
	{
		STR_TRIM_L(sRetStr);
	}
	if(bTrimLast)
	{
		STR_TRIM_R(sRetStr);
	} // if(bTrimLast)

	return sRetStr;
}


//=============================================================================
bool CConsole::GetCursorPos(int &X, int &Y)
{
	return theCPD->GetCursorPos(X, Y);
}
//=============================================================================
bool CConsole::SetCursorPos(int X, int Y)
{
	return theCPD->SetCursorPos(X, Y);
}
