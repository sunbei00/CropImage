#pragma once
#include <Windows.h>

#define nFileNameMaxLen (200)

class MainWindows
{
	static MainWindows* instance;

	HINSTANCE hInstance;
	HWND hWnd;
	HWND hScroll;
	HMENU ID_hScroll;
	int scrollPos;
	MSG msg;
	TCHAR* windowClassName;
	OPENFILENAME OFN;
	WCHAR szFileName[nFileNameMaxLen];
	

	static LRESULT CALLBACK sMainWindowProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	MainWindows(HINSTANCE hInstance, TCHAR* windowClassName);
	void RegistMainWondwsClass();
	void CreateWindows(TCHAR* windowName);
public:
	void MsgFetch();
	static MainWindows* GetInstance();
	static void Init(HINSTANCE hInstance, TCHAR* windowClassName, TCHAR* windowName);

};

