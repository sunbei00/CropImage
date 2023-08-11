#include "DATA.h"
#include <opencv2/opencv.hpp>
#include <filesystem>

TCHAR* DATA::GridClassName = NULL;
HINSTANCE DATA::HInstance = NULL;
std::vector<HWND> DATA::textList;
std::vector<HWND> DATA::gridList;
std::vector<HWND> DATA::fileWriteList;
std::vector<HWND> DATA::fileReadList;
std::vector<HWND> DATA::resetImageList;
std::vector<HWND> DATA::resetCropList;
std::vector<HWND> DATA::colorButtonList;
std::vector<imageDATA> DATA::ImageList;
std::vector<COLORREF> DATA::cropColorList;
std::vector<CropRect> DATA::cropList;
std::vector<std::pair<CropRect,COLORREF>> DATA::saveCropList;
boolean DATA::window_size = true;
HWND DATA::checkBox;
int DATA::thick = 3;
std::string DATA::programPath;
HWND DATA::thickProgressBar;
boolean DATA::check = false;

HWND DATA::boxSave;
HWND DATA::boxDelete;
HWND DATA::saveBoxSize;


void DATA::CreateGrid(TCHAR* windowName, HWND hwnd)
{
	gridList.push_back(CreateWindow(DATA::GridClassName,
		windowName,
		WS_CHILD | WS_VISIBLE,
		100, 100, 768, 768,
		hwnd, NULL, DATA::HInstance, NULL));
}

void DATA::CreateResetImage(TCHAR* windowName, HWND hwnd, HMENU COMMAND_ID)
{
	resetImageList.push_back(CreateWindow(L"button",
		windowName,
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		hwnd, COMMAND_ID, DATA::HInstance, NULL));
}
void DATA::CreateColorButton(TCHAR* windowName, HWND hwnd, HMENU COMMAND_ID)
{
	colorButtonList.push_back(CreateWindow(L"button",
		windowName,
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		hwnd, COMMAND_ID, DATA::HInstance, NULL));
}
void DATA::CreateResetCrop(TCHAR* windowName, HWND hwnd, HMENU COMMAND_ID)
{
	resetCropList.push_back(CreateWindow(L"button",
		windowName,
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		hwnd, COMMAND_ID, DATA::HInstance, NULL));
}

void DATA::CreateText(TCHAR* windowName, HWND hwnd)
{
	textList.push_back(CreateWindow(L"static",
		windowName,
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		hwnd, NULL, DATA::HInstance, NULL));
}

void DATA::CreateFileRead(TCHAR* windowName, HWND hwnd, HMENU COMMAND_ID)
{
	fileReadList.push_back(CreateWindow(L"button",
		windowName,
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		hwnd, COMMAND_ID, DATA::HInstance, NULL));
}

void DATA::CreateFileWrite(TCHAR* windowName, HWND hwnd, HMENU COMMAND_ID)
{
	fileWriteList.push_back(CreateWindow(L"button",
		windowName,
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		hwnd, COMMAND_ID, DATA::HInstance, NULL));
}

void DATA::ImageChange(int index, WCHAR* wchar)
{
	std::filesystem::path path = wchar;
	cv::Mat mat = cv::imread(path.string(), cv::IMREAD_COLOR);

	uchar* data = new uchar[mat.cols * mat.rows * 3];
	memcpy(data, mat.data, sizeof(uchar) * mat.cols * mat.rows * 3);

	if (ImageList[index].bitmap.bmBits != NULL)
		delete[] ImageList[index].bitmap.bmBits;
	memset(&(ImageList[index].bitmap), 0 ,sizeof(BITMAP));
	ImageList[index].bitmap.bmType = 0;
	ImageList[index].bitmap.bmWidth = mat.cols;
	ImageList[index].bitmap.bmHeight = mat.rows;
	ImageList[index].bitmap.bmWidthBytes = mat.cols * 3; 
	ImageList[index].bitmap.bmPlanes = 1; 
	ImageList[index].bitmap.bmBitsPixel = 24;
	ImageList[index].bitmap.bmBits = data;

	ImageList[index].b = true;
}

