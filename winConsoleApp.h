#ifndef WINCONSOLEAPP_H__AC06C7F1_FFBF_4251_86EA_FC3E9DEA784A__INCLUDED
#define WINCONSOLEAPP_H__AC06C7F1_FFBF_4251_86EA_FC3E9DEA784A__INCLUDED


#include <windows.h>


//=============================================================================
struct SControlKeyState
{
	SControlKeyState(DWORD dwControlKeyState);

	bool bCapsLock;
	bool bNumLock;
	bool bScrollLock;
	//
	bool bEnhancedKey;
	//
	bool bAltLeft;
	bool bAltRight;
	//
	bool bCtrlLeft;
	bool bCtrlRight;
	//
	bool bShift;
};


//=============================================================================
class CWinConsoleAppObserver
{
public:
	CWinConsoleAppObserver() {}
	virtual ~CWinConsoleAppObserver() {}

public:
	virtual void GetSize(int &iCols, int &iLines) const { iCols = 80, iLines = 25; }
	virtual void GetEnableInput(bool &bKey, bool &bMouse, DWORD &dwTimeout) const { bKey = false; bMouse = false; dwTimeout = INFINITE; };

	virtual void OnStarted()  {}
	virtual void OnFinished() {}

// Return true to continue.
	virtual bool OnTimeout() { return true; }

	virtual bool OnKeyDown(WORD /*wVirtualKeyCode*/, WORD /*wRepeatCount*/, const SControlKeyState& /*cks*/) { return true; }
	virtual bool OnKeyUp  (WORD /*wVirtualKeyCode*/, WORD /*wRepeatCount*/, const SControlKeyState& /*cks*/) { return true; }

	virtual bool OnMouseLBtnDown    (int /*iCol*/, int /*iRow*/, const SControlKeyState& /*cks*/) { return true; }
	virtual bool OnMouseLBtnUp      (int /*iCol*/, int /*iRow*/, const SControlKeyState& /*cks*/) { return true; }
	virtual bool OnMouseLBtnDblClick(int /*iCol*/, int /*iRow*/, const SControlKeyState& /*cks*/) { return true; }
	//
	virtual bool OnMouseMBtnDown    (int /*iCol*/, int /*iRow*/, const SControlKeyState& /*cks*/) { return true; }
	virtual bool OnMouseMBtnUp      (int /*iCol*/, int /*iRow*/, const SControlKeyState& /*cks*/) { return true; }
	virtual bool OnMouseMBtnDblClick(int /*iCol*/, int /*iRow*/, const SControlKeyState& /*cks*/) { return true; }
	//
	virtual bool OnMouseRBtnDown    (int /*iCol*/, int /*iRow*/, const SControlKeyState& /*cks*/) { return true; }
	virtual bool OnMouseRBtnUp      (int /*iCol*/, int /*iRow*/, const SControlKeyState& /*cks*/) { return true; }
	virtual bool OnMouseRBtnDblClick(int /*iCol*/, int /*iRow*/, const SControlKeyState& /*cks*/) { return true; }

	virtual bool OnMouseMove(int /*iCol*/, int /*iRow*/, const SControlKeyState& /*cks*/) { return true; }

	virtual bool OnMouseWheel (bool /*bUp*/,    int /*iCol*/, int /*iRow*/, const SControlKeyState& /*cks*/) { return true; }
	virtual bool OnMouseHWheel(bool /*bRight*/, int /*iCol*/, int /*iRow*/, const SControlKeyState& /*cks*/) { return true; }
};


//=============================================================================
class CWinConsoleApp
{
public:
	static CWinConsoleApp& GetInstance();

private:
	CWinConsoleApp();
public:
	~CWinConsoleApp();

public:
	bool Init();

	bool SetSize(int iCol, int iLines);

	void SetEnableInput(bool bKey, bool bMouse);
	void GetEnableInput(bool &bKey, bool &bMouse);

	// pObserver must not be NULL.
	// dwTimeout in milliseconds.
	bool Run(CWinConsoleAppObserver *pObserver, DWORD dwTimeout = INFINITE);

	// To use cin:
	// INPUT_BEGIN // wca.InputBegin();
	// cin >> a;
	// INPUT_END // wca.InputEnd();
	//
	void InputBegin();
	void InputEnd();

public:
	inline void SetTimeout(DWORD dwTimeout) { m_dwTimeout = dwTimeout; }
	inline DWORD GetTimeout() const { return m_dwTimeout; }

private:
	bool OnKeyInput(const KEY_EVENT_RECORD& keyEvent);
	bool OnMouseInput(const MOUSE_EVENT_RECORD& mouseEvent);

private:
	bool m_bInitDone;

	HANDLE m_hStdin;
	DWORD m_dwInConsoleModeOld;
	DWORD m_dwInConsoleMode;

	HANDLE m_hStdout;
	DWORD m_dwOutConsoleModeOld;
	BOOL m_bCursorVisibleOld;

	bool m_bInsideInputBeginEnd;

	CWinConsoleAppObserver *m_pObserver;
	DWORD m_dwTimeout;
};
#define theWinConsoleApp  ( CWinConsoleApp::GetInstance() )
#define INPUT_BEGIN  theWinConsoleApp.InputBegin();
#define INPUT_END    theWinConsoleApp.InputEnd();


#endif // ndef WINCONSOLEAPP_H__AC06C7F1_FFBF_4251_86EA_FC3E9DEA784A__INCLUDED
