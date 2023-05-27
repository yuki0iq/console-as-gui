#ifndef CONSOLE_H__INCLUDED
#define CONSOLE_H__INCLUDED


#include <iostream>

//INCLUDE util.cpp to project!!!

//=============================================================================
enum COLOR
{
	BLACK          = 0,
	DARK_RED       = 1,
	DARK_GREEN     = 2,
	OLIVE          = 3,
	DARK_BLUE      = 4,
	DARK_PINK      = 5,
	DARK_BLUEGREEN = 6,
	LIGHT_GRAY     = 7,
	GRAY           = 8,
	RED            = 9,
	GREEN          = 10,
	YELLOW         = 11,
	BLUE           = 12,
	PINK           = 13,
	BLUEGREEN      = 14,
	WHITE          = 15
};


//=============================================================================
// TODO:
// LINUX: GetCursorPos, SetColors
//
class CConsole
{
public:
	CConsole();
	~CConsole();

public:
	bool SetTitle(const std::string& sText);
	void Clear();

	void RestoreColors();
	void RestoreColorText();
	void RestoreColorBkgnd();

	void GetColors(COLOR &rColorText, COLOR &rColorBkgnd);
	COLOR GetColorText();
	COLOR GetColorBkgnd();

	void SetColors(COLOR colorText, COLOR colorBkgnd);
	void SetColorText(COLOR colorText);
	void SetColorBkgnd(COLOR colorBkgnd);

	void OutputColorText(const std::string& sText, COLOR colorText, COLOR colorBkgnd, bool bRestore);

	// trim leading and trailing whitespaces and tabs
	std::string GetTrimmedLine();
	// bTrimFirst - trim leading whitespaces and tabs.
	// bTrimLast -  trim trailing whitespaces and tabs.
	std::string GetLine(bool bTrimFirst, bool bTrimLast);

	bool GetCursorPos(int &X, int &Y);
	bool SetCursorPos(int X, int Y);
};


#endif // ndef CONSOLE_H__INCLUDED
