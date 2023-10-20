//#pragma once
//#include <vector>
//#include <Windows.h>
//#include <vector>
//#include <string>
//
//#define MIN_SIZE_X (1800)
//#define MIN_SIZE_Y (650)
//
//#define MAX_SIZE_X (1800)
//#define MAX_SIZE_Y (1000)
//
//#define FILE_BUTTON_ID(i) ((i)+0x1000)
//#define RESET_IMAGE_BUTTON_ID(i) ((i)+0x1010)
//#define RESET_CROP_BUTTON_ID(i) ((i)+0x1020)
//#define COLOR_BUTTON_ID(i) ((i)+0x1030)
//#define WRITE_BUTTON_ID(i) ((i)+0x1040)
//#define SCROLL_TH (0x1050)
//#define CHECK_BOX (0x1060)
//
//
//#define BOX_SAVE 0x1070
//#define BOX_DELETE 0x1071
//
//#ifndef GetRValue	
//#define GetRValue(rgb)	( (BYTE)(rgb) )
//#endif
//#ifndef GetGValue
//#define GetGValue(rgb)	( (BYTE)(((WORD)(rgb))>>8) )
//#endif
//#ifndef GetBValue
//#define GetBValue(rgb)	( (BYTE)((rgb)>>16) )
//#endif
//
//
//
//typedef struct {
//	boolean b;
//	BITMAP bitmap;
//} imageDATA;
//
//typedef struct {
//	boolean b;
//	boolean click;
//	RECT rect;
//} CropRect;
//
//
//class DATA {
//public:
//	static TCHAR* GridClassName;
//	static HINSTANCE HInstance;
//	static int thick;
//	static HWND thickProgressBar;
//	static HWND checkBox;
//	static HWND boxSave;
//	static HWND boxDelete;
//	static HWND saveBoxSize;
//	static std::string programPath;
//	static std::vector<HWND> gridList;
//	static std::vector<HWND> textList;
//	static std::vector<HWND> fileWriteList;
//	static std::vector<HWND> fileReadList;
//	static std::vector<HWND> resetImageList;
//	static std::vector<HWND> resetCropList;
//	static std::vector<HWND> colorButtonList;
//	static std::vector<imageDATA> ImageList;
//	static std::vector<CropRect> cropList;
//	static std::vector<std::pair<CropRect, COLORREF>> saveCropList;
//	static std::vector<COLORREF> cropColorList;
//	static boolean window_size;
//	static boolean check; // 체크 : 프리 , 언체크 : 정사각형
//	static void CreateGrid(TCHAR* windowName, HWND hwnd);
//	static void CreateText(TCHAR* windowName, HWND hwnd);
//	static void CreateFileRead(TCHAR* windowName, HWND hwnd, HMENU COMMAND_ID);
//	static void CreateResetImage(TCHAR* windowName, HWND hwnd, HMENU COMMAND_ID);
//	static void CreateColorButton(TCHAR* windowName, HWND hwnd, HMENU COMMAND_ID);
//	static void CreateResetCrop(TCHAR* windowName, HWND hwnd, HMENU COMMAND_ID);
//	static void CreateFileWrite(TCHAR* windowName, HWND hwnd, HMENU COMMAND_ID);
//	static void ImageChange(int index, WCHAR* wchar);
//	
//};
//
