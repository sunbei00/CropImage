#include <Windows.h>
#include "MainWindows.h"
#include "DATA.h"
#include "Grid.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
	DATA::HInstance = hInstance;
	DATA::GridClassName = (TCHAR*)L"Grid";
	registGridWindowClass();

	MainWindows::Init(hInstance, (TCHAR*)L"MainClass", (TCHAR*)L"Crop Image");

	MainWindows::GetInstance()->MsgFetch();
	return 0;
}
