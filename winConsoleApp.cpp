#include <windows.h>
#include <iostream>
#include <string>

#include "winConsoleApp.h"

using namespace std;


//*****************************************************************************
SControlKeyState::SControlKeyState(DWORD dwControlKeyState)
{
	bCapsLock   = (CAPSLOCK_ON   & dwControlKeyState ? true : false);
	bNumLock    = (NUMLOCK_ON    & dwControlKeyState ? true : false);
	bScrollLock = (SCROLLLOCK_ON & dwControlKeyState ? true : false);
	//
	bEnhancedKey = (ENHANCED_KEY & dwControlKeyState ? true : false);
	//
	bAltLeft  = (LEFT_ALT_PRESSED  & dwControlKeyState ? true : false);
	bAltRight = (RIGHT_ALT_PRESSED & dwControlKeyState ? true : false);
	//
	bCtrlLeft  = (LEFT_CTRL_PRESSED  & dwControlKeyState ? true : false);
	bCtrlRight = (RIGHT_CTRL_PRESSED & dwControlKeyState ? true : false);
	//
	bShift = (SHIFT_PRESSED & dwControlKeyState ? true : false);
}


//*****************************************************************************
// CWinConsoleApp

//=============================================================================
CWinConsoleApp& CWinConsoleApp::GetInstance()
{
	static CWinConsoleApp app;
	return app;
}


//=============================================================================
CWinConsoleApp::CWinConsoleApp()
	: m_bInitDone(false)
	, m_hStdin(0)
	, m_dwInConsoleModeOld(0)
	, m_dwInConsoleMode(ENABLE_WINDOW_INPUT)
	, m_hStdout(0)
	, m_dwOutConsoleModeOld(0)
	, m_bCursorVisibleOld(TRUE)
	, m_bInsideInputBeginEnd(false)
	, m_pObserver(nullptr)
	, m_dwTimeout(INFINITE)
{
}


//=============================================================================
CWinConsoleApp::~CWinConsoleApp()
{
	if(m_bInitDone)
	{
		// Restore input mode on exit.
		InputBegin();
		m_dwInConsoleModeOld = 0;
		m_hStdin = 0;
		m_dwOutConsoleModeOld = 0;
		m_hStdout = 0;
		m_bInitDone = false;
	}
}


//=============================================================================
bool CWinConsoleApp::Init()
{
	if( ! m_bInitDone)
	{
		m_hStdin  = ::GetStdHandle(STD_INPUT_HANDLE );
		m_hStdout = ::GetStdHandle(STD_OUTPUT_HANDLE);

		// Save the current modes, to be restored on exit.
		::GetConsoleMode(m_hStdin,  &m_dwInConsoleModeOld );
		::GetConsoleMode(m_hStdout, &m_dwOutConsoleModeOld);
		//
		CONSOLE_CURSOR_INFO ci = {};
		::GetConsoleCursorInfo(m_hStdout, &ci);
		m_bCursorVisibleOld = ci.bVisible;

		m_bInitDone = true;
	}
	return m_bInitDone;
}


//=============================================================================
bool CWinConsoleApp::SetSize(int iCol, int iLines)
{
//system("mode con COLS=80 LINES=25");

HANDLE hStdout = ::GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO sbi = { };
	::GetConsoleScreenBufferInfo(hStdout, &sbi);

	SMALL_RECT rcWnd = { 0, 0, 1, 1 };
	::SetConsoleWindowInfo(hStdout, TRUE, &rcWnd);

	COORD cBuff = { iCol, iLines };
	::SetConsoleScreenBufferSize(hStdout, cBuff);

	rcWnd = { 0, 0, iCol-1, iLines-1 };
	::SetConsoleWindowInfo(hStdout, TRUE, &rcWnd);

	sbi = { };
	::GetConsoleScreenBufferInfo(hStdout, &sbi);

	return true;
}


//=============================================================================
void CWinConsoleApp::SetEnableInput(bool bKey, bool bMouse)
{
	if(bKey)
	{
		m_dwInConsoleMode |= ENABLE_WINDOW_INPUT;
	}
	else
	{
		m_dwInConsoleMode &= ~ENABLE_WINDOW_INPUT;
	}
	if(bMouse)
	{
		m_dwInConsoleMode |= ENABLE_MOUSE_INPUT;
	}
	else
	{
		m_dwInConsoleMode &= ~ENABLE_MOUSE_INPUT;
	}
	if( ! m_bInsideInputBeginEnd)
	{
		// Enable the [window], [mouse] input events.
		::SetConsoleMode(m_hStdin, m_dwInConsoleMode);
	}
}
//=============================================================================
void CWinConsoleApp::GetEnableInput(bool &bKey, bool &bMouse)
{
	bKey   = (m_dwInConsoleMode & ENABLE_WINDOW_INPUT ? true : false);
	bMouse = (m_dwInConsoleMode & ENABLE_MOUSE_INPUT  ? true : false);
}


//=============================================================================
bool CWinConsoleApp::Run(CWinConsoleAppObserver *pObserver, DWORD dwTimeout/* = INFINITE*/)
{
	if( ! m_bInitDone || nullptr == pObserver)
	{
		return false;
	}

	m_pObserver = pObserver;
	m_dwTimeout = dwTimeout;

	m_pObserver->OnStarted();

	// Enable the window and mouse input events.
	InputEnd();

	bool bWhile = true;
	while(bWhile)
	{
		DWORD dwRes = ::WaitForSingleObject(m_hStdin, m_dwTimeout);

		if(WAIT_TIMEOUT == dwRes)
		{
			bool bRes = m_pObserver->OnTimeout();
			if( ! bRes)
			{
				break;
			}
			continue;
		} // if(WAIT_TIMEOUT == dwRes)

		if(WAIT_OBJECT_0 == dwRes)
		{
			INPUT_RECORD irInBuf[128];
			DWORD dwNumRead = 0;;

			BOOL bRes = ::ReadConsoleInput(m_hStdin, irInBuf, 128, &dwNumRead);
			//::FlushConsoleInputBuffer(hStdin);
			if( ! bRes)
			{
				continue;
			}

			bool bExit = false;
			for(int k = 0; k < (int)dwNumRead; k++)
			{
				bool bRes = true;
				switch(irInBuf[k].EventType)
				{
					case KEY_EVENT: // keyboard input
					{
						bRes = OnKeyInput(irInBuf[k].Event.KeyEvent);
					} break;

					case MOUSE_EVENT: // mouse input
					{
						bRes = OnMouseInput(irInBuf[k].Event.MouseEvent);
					} break;
 
					default:
						break;
				}
				if( ! bRes)
				{
					bExit = true;
					break;
				}
			}
			if(bExit)
			{
				break;
			}
			continue;
		} // if(WAIT_OBJECT_0 == dwRes)
	} // while(bWhile)

	m_pObserver->OnFinished();

	return true;
}


//=============================================================================
void CWinConsoleApp::InputBegin()
{
	m_bInsideInputBeginEnd = true;

	// Restore input mode.
	::SetConsoleMode(m_hStdin, m_dwInConsoleModeOld);

	// Restore scroll up.
	::SetConsoleMode(m_hStdout, m_dwOutConsoleModeOld);

	// Restore cursor visible.
	CONSOLE_CURSOR_INFO ci = {};
	::GetConsoleCursorInfo(m_hStdout, &ci);
	ci.bVisible = m_bCursorVisibleOld;
	::SetConsoleCursorInfo(m_hStdout, &ci);
}
//=============================================================================
void CWinConsoleApp::InputEnd()
{
	// Enable the [window], [mouse] input events.
	::SetConsoleMode(m_hStdin, m_dwInConsoleMode);

	// Do not scroll up.
	DWORD dwMode = m_dwOutConsoleModeOld;
	dwMode &= ~ENABLE_WRAP_AT_EOL_OUTPUT;
	::SetConsoleMode(m_hStdout, dwMode);

	// Hide cursor.
	CONSOLE_CURSOR_INFO ci = {};
	::GetConsoleCursorInfo(m_hStdout, &ci);
	ci.bVisible = FALSE;
	::SetConsoleCursorInfo(m_hStdout, &ci);

	m_bInsideInputBeginEnd = false;
}


//=============================================================================
bool CWinConsoleApp::OnKeyInput(const KEY_EVENT_RECORD& keyEvent)
{
	SControlKeyState cks(keyEvent.dwControlKeyState);

	bool bRetCode = true;
	if(keyEvent.bKeyDown)
	{
		bRetCode = m_pObserver->OnKeyDown(keyEvent.wVirtualKeyCode, keyEvent.wRepeatCount, cks);
	}
	else // ( ! keyEvent.bKeyDown)
	{
		bRetCode = m_pObserver->OnKeyUp(keyEvent.wVirtualKeyCode, keyEvent.wRepeatCount, cks);
	}
	return bRetCode;
}


//=============================================================================
bool CWinConsoleApp::OnMouseInput(const MOUSE_EVENT_RECORD& mouseEvent)
{
	bool bRetCode = true;

	int iCol = mouseEvent.dwMousePosition.X;
	int iRow = mouseEvent.dwMousePosition.Y;

	SControlKeyState cks(mouseEvent.dwControlKeyState);

static int iLastBtnDown = 0; // 0 - left; 1 - middle; 2 - right.
	switch(mouseEvent.dwEventFlags)
	{
		case 0:
		{
static bool bLeftOld   = false;
static bool bMiddleOld = false;
static bool bRightOld  = false;

			bool bLeft   = (FROM_LEFT_1ST_BUTTON_PRESSED & mouseEvent.dwButtonState ? true : false);
			bool bMiddle = (FROM_LEFT_2ND_BUTTON_PRESSED & mouseEvent.dwButtonState ? true : false);
			bool bRight  = (RIGHTMOST_BUTTON_PRESSED     & mouseEvent.dwButtonState ? true : false);

			if(bLeftOld != bLeft)
			{
				if(bLeft)
				{
					bRetCode = m_pObserver->OnMouseLBtnDown(iCol, iRow, cks);
					iLastBtnDown = 0; // 0 - left; 1 - middle; 2 - right.
				}
				else // ( ! bLeft)
				{
					bRetCode = m_pObserver->OnMouseLBtnUp(iCol, iRow, cks);
				}
				bLeftOld = bLeft;
			}
			if(bMiddleOld != bMiddle)
			{
				if(bMiddle)
				{
					bRetCode = m_pObserver->OnMouseMBtnDown(iCol, iRow, cks);
					iLastBtnDown = 1; // 0 - left; 1 - middle; 2 - right.
				}
				else // ( ! bMiddle)
				{
					bRetCode = m_pObserver->OnMouseMBtnUp(iCol, iRow, cks);
				}
				bMiddleOld = bMiddle;
			}
			if(bRightOld != bRight)
			{
				if(bRight)
				{
					bRetCode = m_pObserver->OnMouseRBtnDown(iCol, iRow, cks);
					iLastBtnDown = 2; // 0 - left; 1 - middle; 2 - right.
				}
				else // ( ! bRight)
				{
					bRetCode = m_pObserver->OnMouseRBtnUp(iCol, iRow, cks);
				}
				bRightOld = bRight;
			}
		} break;

		case DOUBLE_CLICK:
		{
			switch(iLastBtnDown) // 0 - left; 1 - middle; 2 - right.
			{
				case 0: bRetCode = m_pObserver->OnMouseLBtnDblClick(iCol, iRow, cks); break;
				case 1: bRetCode = m_pObserver->OnMouseMBtnDblClick(iCol, iRow, cks); break;
				case 2: bRetCode = m_pObserver->OnMouseRBtnDblClick(iCol, iRow, cks); break;
				default: break;
			}
		} break;

		case MOUSE_MOVED:
			bRetCode = m_pObserver->OnMouseMove(iCol, iRow, cks);
		break;

		case MOUSE_WHEELED:
		{
			bool bUp = (signed short)HIWORD(mouseEvent.dwButtonState) > 0 ? true : false;
			bRetCode = m_pObserver->OnMouseWheel(bUp, iCol, iRow, cks);
		}
		break;

		case MOUSE_HWHEELED:
		{
			bool bRight = (signed short)HIWORD(mouseEvent.dwButtonState) > 0 ? true : false;
			bRetCode = m_pObserver->OnMouseHWheel(bRight, iCol, iRow, cks);
		} break;

		default:
			break;
	}

	return bRetCode;
}
