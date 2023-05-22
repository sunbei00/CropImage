#include "MainWindows.h"
#include "DATA.h"
#include <stdexcept>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <time.h>
#include <atlconv.h>

MainWindows* MainWindows::instance = NULL;

void saveImage(cv::Mat* mat, std::string dir, std::string file) {
	std::string path = DATA::programPath + dir;

	if (!std::filesystem::exists(path))
		std::filesystem::create_directory(path);
	cv::imwrite(path + file, *mat);
}

void drawPixsel(cv::Vec3b& pixel, COLORREF color) {
	pixel[0] = GetBValue(color);
	pixel[1] = GetGValue(color);
	pixel[2] = GetRValue(color);
}

void drawRect(cv::Mat* mat, int& index, int& top, int& height, int& left, int& width, int& right, int& bottom) {
	for (int y = 0; y < height; y++) {
		cv::Vec3b& matPixelLeft = mat->at<cv::Vec3b>(top + y, left);
		cv::Vec3b& matPixelRight = mat->at<cv::Vec3b>(top + y, right);
		drawPixsel(matPixelLeft, DATA::cropColorList[index]);
		drawPixsel(matPixelRight, DATA::cropColorList[index]);
	}
	for (int x = 0; x < width; x++) {
		cv::Vec3b& matPixelTop = mat->at<cv::Vec3b>(top, left + x);
		cv::Vec3b& matPixelBottom = mat->at<cv::Vec3b>(bottom, left + x);
		drawPixsel(matPixelTop, DATA::cropColorList[index]);
		drawPixsel(matPixelBottom, DATA::cropColorList[index]);
	}
}

void returnDrawRect(int index, cv::Mat* mat, size_t thickness) {
	if (mat == NULL)
		return;
	if (index < 0 || index > 9)
		return;
	if (!DATA::cropList[index].b)
		return;
	CropRect& cropRect = DATA::cropList[index];

	int width = std::abs(cropRect.rect.right - cropRect.rect.left);
	int height = std::abs(cropRect.rect.top - cropRect.rect.bottom);
	int left = std::min(cropRect.rect.left, cropRect.rect.right);
	int right = std::max(cropRect.rect.left, cropRect.rect.right);
	int top = std::min(cropRect.rect.bottom, cropRect.rect.top);;
	int bottom = std::max(cropRect.rect.bottom, cropRect.rect.top);;

	for (size_t i = 0; i < thickness; i++) {
		drawRect(mat, index, top, height, left, width, right, bottom);

		left++;
		bottom--;
		top++;
		right--;
		width = right - left;
		height = bottom - top;
	}
}

cv::Mat* returnMat(int index) {
	if (index < 0 || index > 9)
		return NULL;
	if (!DATA::ImageList[index].b)
		return NULL;
	BITMAP& bitmap = DATA::ImageList[index].bitmap;
	cv::Mat* mat = new cv::Mat(bitmap.bmHeight, bitmap.bmWidth, CV_8UC3);
	mat->create(bitmap.bmHeight, bitmap.bmWidth, CV_8UC3);

	for (int y = 0; y < bitmap.bmHeight; y++) {
		for (int x = 0; x < bitmap.bmWidth; x++) {
			RGBTRIPLE* pixel = reinterpret_cast<RGBTRIPLE*>(bitmap.bmBits) + y * bitmap.bmWidth + x;
			cv::Vec3b& matPixel = mat->at<cv::Vec3b>(y, x);
			matPixel[0] = pixel->rgbtBlue;
			matPixel[1] = pixel->rgbtGreen;
			matPixel[2] = pixel->rgbtRed;
		}
	}
	return mat;
}

cv::Mat* returnCropMat(int index, int cropIndex) {
	if (index < 0 || index > 9)
		return NULL;
	if (!DATA::ImageList[index].b)
		return NULL;
	if (!DATA::cropList[cropIndex].b)
		return NULL;

	CropRect& cropRect = DATA::cropList[cropIndex];
	BITMAP& bitmap = DATA::ImageList[index].bitmap;

	size_t width = std::abs(cropRect.rect.right - cropRect.rect.left);
	size_t height = std::abs(cropRect.rect.top - cropRect.rect.bottom);
	size_t left = std::min(cropRect.rect.left, cropRect.rect.right);
	size_t top = std::min(cropRect.rect.bottom, cropRect.rect.top);;


	cv::Mat* mat = new cv::Mat(height, width, CV_8UC3);
	mat->create(height, width, CV_8UC3);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			RGBTRIPLE* pixel = reinterpret_cast<RGBTRIPLE*>(bitmap.bmBits) + (y + top) * bitmap.bmWidth + (x + left);
			cv::Vec3b& matPixel = mat->at<cv::Vec3b>(y, x);
			matPixel[0] = pixel->rgbtBlue;
			matPixel[1] = pixel->rgbtGreen;
			matPixel[2] = pixel->rgbtRed;
		}
	}
	return mat;
}

LRESULT CALLBACK MainWindows::MainWindowProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static TCHAR tmpBuffer[200];
	static CHOOSECOLOR COL;
	static COLORREF crTemp[16];
	static cv::Mat* mat;
	static std::filesystem::path path;
	static std::stringstream ssDouble;

	switch (iMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		hScroll = ::CreateWindow(L"scrollbar", NULL, WS_CHILD | WS_VISIBLE | SBS_HORZ, 10, 70, 200, 20, hwnd, ID_hScroll, hInstance, NULL);
		DATA::thickProgressBar = ::CreateWindow(L"scrollbar", NULL, WS_CHILD | WS_VISIBLE | SBS_VERT, 0, 0, 0, 0, hwnd, (HMENU)SCROLL_TH, hInstance, NULL);
		::SetScrollRange(hScroll, SB_CTL, 0, 81, TRUE);
		::SetScrollRange(DATA::thickProgressBar, SB_CTL, 1, 10, TRUE);
		for (int i = 0; i < 10; i++) {
			swprintf_s(tmpBuffer, 200, L"%d", i);
			DATA::CreateGrid((TCHAR*)tmpBuffer, hwnd);
		}
		DATA::CreateText((TCHAR*)L"Original", hwnd);
		for (int i = 1; i < 10; i++) {
			swprintf_s(tmpBuffer, 200, L"%d", i);
			DATA::CreateText((TCHAR*)tmpBuffer, hwnd);
		}
		for (int i = 0; i < 10; i++)
			DATA::CreateResetImage((TCHAR*)L"Reset Image", hwnd, (HMENU)RESET_IMAGE_BUTTON_ID(i));
		for (int i = 0; i < 10; i++)
			DATA::CreateResetCrop((TCHAR*)L"Reset Box", hwnd, (HMENU)RESET_CROP_BUTTON_ID(i));
		for (int i = 0; i < 10; i++)
			DATA::CreateColorButton((TCHAR*)L"Color", hwnd, (HMENU)COLOR_BUTTON_ID(i));
		for (int i = 0; i < 10; i++)
			DATA::CreateFileRead((TCHAR*)L"File", hwnd, (HMENU)FILE_BUTTON_ID(i));
		for (int i = 0; i < 10; i++)
			DATA::CreateFileWrite((TCHAR*)L"Save", hwnd, (HMENU)WRITE_BUTTON_ID(i));
		for (int i = 0; i < 10; i++)
			DATA::ImageList.push_back({ false,BITMAP() });
		for (int i = 0; i < 10; i++)
			DATA::cropList.push_back({ false,false,RECT() });
		for (int i = 0; i < 10; i++)
			DATA::cropColorList.push_back(RGB(255, 0, 0));
		DATA::checkBox = CreateWindow(L"button",
			L"free",
			WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			0, 0, 0, 0,
			hwnd, (HMENU)CHECK_BOX, DATA::HInstance, NULL);

		memset(&OFN, 0, sizeof(OPENFILENAME));
		OFN.lStructSize = sizeof(OPENFILENAME);
		OFN.hwndOwner = hwnd;
		OFN.lpstrFilter = L"All Files(*.*)\0*.*\0";
		OFN.nMaxFile = nFileNameMaxLen;
		OFN.lpstrFile = szFileName;

		memset(&COL, 0, sizeof(CHOOSECOLOR));
		COL.lStructSize = sizeof(CHOOSECOLOR);
		COL.hwndOwner = hWnd;
		COL.lpCustColors = crTemp;
		COL.Flags = 0;

		DATA::programPath = std::filesystem::current_path().string() + "\\";
		::SetScrollPos(DATA::thickProgressBar, SB_CTL, DATA::thick, TRUE);
		break;
	case WM_SIZE:
		::SetWindowPos(DATA::thickProgressBar, NULL, (0 - ((float)scrollPos / 10)) * (DATA::window_size ? 430 : 800), 30, 30, 150, 0);
		::SetWindowPos(DATA::checkBox, NULL, (-1 * ((float)scrollPos / 10)) * (DATA::window_size ? 430 : 800), 200, 50,50, 0);
		::SetWindowPos(hScroll, NULL, 0, HIWORD(lParam) - 30, LOWORD(lParam), 30, 0);
		for (int i = 0; i < DATA::gridList.size(); i++)
			::SetWindowPos(DATA::gridList[i], NULL, 50 + ((float)i - ((float)scrollPos / 10)) * (DATA::window_size ? 430 : 800), 50, DATA::window_size ? 384 : 768, DATA::window_size ? 384 : 768, 0);
		for (int i = 0; i < DATA::textList.size(); i++)
			::SetWindowPos(DATA::textList[i], NULL, 50 + ((float)i - ((float)scrollPos / 10)) * (DATA::window_size ? 430 : 800), 50 + (DATA::window_size ? 384 : 768), 384, 20, 0);
		for (int i = 0; i < DATA::fileWriteList.size(); i++)
			::SetWindowPos(DATA::fileWriteList[i], NULL, 50 + ((float)i - ((float)scrollPos / 10)) * (DATA::window_size ? 430 : 800), 25, 80, 20, 0);
		for (int i = 0; i < DATA::fileReadList.size(); i++)
			::SetWindowPos(DATA::fileReadList[i], NULL, 50 + ((float)i - ((float)scrollPos / 10)) * (DATA::window_size ? 430 : 800) + 80, 25, 50, 20, 0);
		for (int i = 0; i < DATA::resetImageList.size(); i++)
			::SetWindowPos(DATA::resetImageList[i], NULL, 50 + ((float)i - ((float)scrollPos / 10)) * (DATA::window_size ? 430 : 800) + 130, 25, 100, 20, 0);
		for (int i = 0; i < DATA::resetCropList.size(); i++)
			::SetWindowPos(DATA::resetCropList[i], NULL, 50 + ((float)i - ((float)scrollPos / 10)) * (DATA::window_size ? 430 : 800) + 230, 25, 100, 20, 0);
		for (int i = 0; i < DATA::colorButtonList.size(); i++)
			::SetWindowPos(DATA::colorButtonList[i], NULL, 50 + ((float)i - ((float)scrollPos / 10)) * (DATA::window_size ? 430 : 800) + 330, 25, 50, 20, 0);
		::InvalidateRect(hwnd, NULL, TRUE);
		break;
	case WM_VSCROLL:
		switch (LOWORD(wParam))
		{
		case SB_LINEDOWN:
			DATA::thick = std::min(10, DATA::thick + 1);
			break;
		case SB_PAGEDOWN:
			DATA::thick = std::min(10, DATA::thick + 1);
			break;
		case SB_LINEUP:
			DATA::thick = std::max(1, DATA::thick - 1);
			break;
		case SB_PAGEUP:
			DATA::thick = std::max(1, DATA::thick - 1);
			break;
		case SB_THUMBTRACK:
			DATA::thick = HIWORD(wParam);
			break;
		}
		::SetScrollPos(DATA::thickProgressBar, SB_CTL, DATA::thick, TRUE);
		::InvalidateRect(hwnd, NULL, TRUE);
		break;
	case WM_HSCROLL:
		switch (LOWORD(wParam))
		{
		case SB_LINELEFT:
			scrollPos = std::max(0, scrollPos - 1);
			break;
		case SB_PAGELEFT:
			scrollPos = std::max(0, scrollPos - 5);
			break;
		case SB_LINERIGHT:
			scrollPos = std::min(81, scrollPos + 1);
			break;
		case SB_PAGERIGHT:
			scrollPos = std::min(81, scrollPos + 5);
			break;
		case SB_THUMBTRACK:
			scrollPos = HIWORD(wParam);
			break;
		}
		::SetWindowPos(DATA::thickProgressBar, NULL, (-1 * ((float)scrollPos / 10))* (DATA::window_size ? 430 : 800), 30, 30, 150, 0);
		::SetWindowPos(DATA::checkBox, NULL, (-1 * ((float)scrollPos / 10))* (DATA::window_size ? 430 : 800), 200, 50, 50, 0);
		for (int i = 0; i < DATA::gridList.size(); i++)
			::SetWindowPos(DATA::gridList[i], NULL, 50 + ((float)i - ((float)scrollPos / 10)) * (DATA::window_size ? 430 : 800), 50, DATA::window_size ? 384 : 768, DATA::window_size ? 384 : 768, 0);
		for (int i = 0; i < DATA::fileWriteList.size(); i++)
			::SetWindowPos(DATA::fileWriteList[i], NULL, 50 + ((float)i - ((float)scrollPos / 10)) * (DATA::window_size ? 430 : 800), 25, 80, 20, 0);
		for (int i = 0; i < DATA::textList.size(); i++)
			::SetWindowPos(DATA::textList[i], NULL, 50 + ((float)i - ((float)scrollPos / 10)) * (DATA::window_size ? 430 : 800), 50 + (DATA::window_size ? 384 : 768), 384, 20, 0);
		for (int i = 0; i < DATA::fileReadList.size(); i++)
			::SetWindowPos(DATA::fileReadList[i], NULL, 50 + ((float)i - ((float)scrollPos / 10)) * (DATA::window_size ? 430 : 800) + 80, 25, 50, 20, 0);
		for (int i = 0; i < DATA::resetImageList.size(); i++)
			::SetWindowPos(DATA::resetImageList[i], NULL, 50 + ((float)i - ((float)scrollPos / 10)) * (DATA::window_size ? 430 : 800) + 130, 25, 100, 20, 0);
		for (int i = 0; i < DATA::resetCropList.size(); i++)
			::SetWindowPos(DATA::resetCropList[i], NULL, 50 + ((float)i - ((float)scrollPos / 10)) * (DATA::window_size ? 430 : 800) + 230, 25, 100, 20, 0);
		for (int i = 0; i < DATA::colorButtonList.size(); i++)
			::SetWindowPos(DATA::colorButtonList[i], NULL, 50 + ((float)i - ((float)scrollPos / 10)) * (DATA::window_size ? 430 : 800) + 330, 25, 50, 20, 0);
		::SetScrollPos(hScroll, SB_CTL, scrollPos, TRUE);
		
		::InvalidateRect(hwnd, NULL, TRUE);
		break;
	case WM_GETMINMAXINFO:
		if (DATA::window_size) {
			((LPMINMAXINFO)lParam)->ptMinTrackSize.x = MIN_SIZE_X;
			((LPMINMAXINFO)lParam)->ptMinTrackSize.y = MIN_SIZE_Y;
			((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = MIN_SIZE_X;
			((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = MIN_SIZE_Y;
			((LPMINMAXINFO)lParam)->ptMaxSize.x = MIN_SIZE_X;
			((LPMINMAXINFO)lParam)->ptMaxSize.y = MIN_SIZE_Y;
		}
		else {
			((LPMINMAXINFO)lParam)->ptMinTrackSize.x = MAX_SIZE_X;
			((LPMINMAXINFO)lParam)->ptMinTrackSize.y = MAX_SIZE_Y;
			((LPMINMAXINFO)lParam)->ptMaxTrackSize.x = MAX_SIZE_X;
			((LPMINMAXINFO)lParam)->ptMaxTrackSize.y = MAX_SIZE_Y;
			((LPMINMAXINFO)lParam)->ptMaxSize.x = MAX_SIZE_X;
			((LPMINMAXINFO)lParam)->ptMaxSize.y = MAX_SIZE_Y;
		}
		break;
	case WM_SYSCOMMAND:
		if (wParam == SC_MAXIMIZE)
			DATA::window_size = !DATA::window_size;
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) { 
		case CHECK_BOX:
			switch (HIWORD(wParam)) {
			case BN_CLICKED: // push 
				if (!SendDlgItemMessage(hWnd, CHECK_BOX, BM_GETCHECK, 0, 0)) {  // 체크상태이면
					DATA::check = false;
					SendMessage(hWnd, BM_SETCHECK, BST_UNCHECKED, 0);
				}
				else {
					DATA::check = true;
					SendMessage(hWnd, BM_SETCHECK, BST_CHECKED, 0);
				}
				break;
			}
			break;
		}
		switch (wParam) {
		case FILE_BUTTON_ID(0):
		case FILE_BUTTON_ID(1):
		case FILE_BUTTON_ID(2):
		case FILE_BUTTON_ID(3):
		case FILE_BUTTON_ID(4):
		case FILE_BUTTON_ID(5):
		case FILE_BUTTON_ID(6):
		case FILE_BUTTON_ID(7):
		case FILE_BUTTON_ID(8):
		case FILE_BUTTON_ID(9):
			if (0 != GetOpenFileName(&OFN)) {
				DATA::ImageChange(wParam - FILE_BUTTON_ID(0), szFileName);
				wchar_t* pos = szFileName;
				for (int j = 0; j < 100; j++)
					if (szFileName[j] == L'\\')
						pos = &szFileName[j+1];
				SetWindowText(DATA::textList[wParam - FILE_BUTTON_ID(0)], pos);
			}
			break;
		case RESET_IMAGE_BUTTON_ID(0):
		case RESET_IMAGE_BUTTON_ID(1):
		case RESET_IMAGE_BUTTON_ID(2):
		case RESET_IMAGE_BUTTON_ID(3):
		case RESET_IMAGE_BUTTON_ID(4):
		case RESET_IMAGE_BUTTON_ID(5):
		case RESET_IMAGE_BUTTON_ID(6):
		case RESET_IMAGE_BUTTON_ID(7):
		case RESET_IMAGE_BUTTON_ID(8):
		case RESET_IMAGE_BUTTON_ID(9):

			SetWindowText(DATA::textList[wParam - RESET_IMAGE_BUTTON_ID(0)], L"Reset");
			DATA::ImageList[wParam - RESET_IMAGE_BUTTON_ID(0)].b = false;
			DATA::cropList[wParam - RESET_IMAGE_BUTTON_ID(0)].b = false;
			break;
		case RESET_CROP_BUTTON_ID(0):
		case RESET_CROP_BUTTON_ID(1):
		case RESET_CROP_BUTTON_ID(2):
		case RESET_CROP_BUTTON_ID(3):
		case RESET_CROP_BUTTON_ID(4):
		case RESET_CROP_BUTTON_ID(5):
		case RESET_CROP_BUTTON_ID(6):
		case RESET_CROP_BUTTON_ID(7):
		case RESET_CROP_BUTTON_ID(8):
		case RESET_CROP_BUTTON_ID(9):
			DATA::cropList[wParam - RESET_CROP_BUTTON_ID(0)].b = false;
			break;
		case COLOR_BUTTON_ID(0):
		case COLOR_BUTTON_ID(1):
		case COLOR_BUTTON_ID(2):
		case COLOR_BUTTON_ID(3):
		case COLOR_BUTTON_ID(4):
		case COLOR_BUTTON_ID(5):
		case COLOR_BUTTON_ID(6):
		case COLOR_BUTTON_ID(7):
		case COLOR_BUTTON_ID(8):
		case COLOR_BUTTON_ID(9):
			if (ChooseColor(&COL) != 0)
				DATA::cropColorList[wParam - COLOR_BUTTON_ID(0)] = COL.rgbResult;
			break;
		case WRITE_BUTTON_ID(0):
			ssDouble.str("");
			ssDouble << time(NULL);
			for (int i = 0; i < 10; i++) {
				mat = returnMat(i);
				if (mat == NULL)
					continue;
				returnDrawRect(0, mat, DATA::thick);
				GetWindowText(DATA::textList[i], tmpBuffer, 200);
				path = tmpBuffer;
				saveImage(mat, ssDouble.str(),"\\"+ path.string());
				delete mat;
				mat = NULL;
			}
			for (int i = 0; i < 10; i++) {
				mat = returnCropMat(i, 0);
				if (mat == NULL)
					continue;
				GetWindowText(DATA::textList[i], tmpBuffer, 200);
				path = tmpBuffer;
				saveImage(mat, ssDouble.str(), "\\crop_" + path.string());
				delete mat;
				mat = NULL;
			}

			break;
		case WRITE_BUTTON_ID(1):
		case WRITE_BUTTON_ID(2):
		case WRITE_BUTTON_ID(3):
		case WRITE_BUTTON_ID(4):
		case WRITE_BUTTON_ID(5):
		case WRITE_BUTTON_ID(6):
		case WRITE_BUTTON_ID(7):
		case WRITE_BUTTON_ID(8):
		case WRITE_BUTTON_ID(9):
			ssDouble.str("");
			ssDouble << time(NULL);
			mat = returnMat(wParam - WRITE_BUTTON_ID(0));
			if (mat == NULL)
				break;
			returnDrawRect(wParam - WRITE_BUTTON_ID(0), mat, DATA::thick);
			GetWindowText(DATA::textList[wParam - WRITE_BUTTON_ID(0)], tmpBuffer, 200);
			path = tmpBuffer;
			saveImage(mat, ssDouble.str(), "\\" + path.string());
			delete mat;
			mat = NULL;

			mat = returnMat(0);
			if (mat == NULL)
				break;
			returnDrawRect(wParam - WRITE_BUTTON_ID(0), mat, DATA::thick);
			GetWindowText(DATA::textList[0], tmpBuffer, 200);
			path = tmpBuffer;
			saveImage(mat, ssDouble.str(), "\\" + path.string());
			delete mat;
			mat = NULL;

			mat = returnCropMat(0, wParam - WRITE_BUTTON_ID(0));
			if (mat == NULL)
				break;
			GetWindowText(DATA::textList[0], tmpBuffer, 200);
			path = tmpBuffer;
			saveImage(mat, ssDouble.str(), "\\crop_" + path.string());
			delete mat;
			mat = NULL;

			mat = returnCropMat(wParam - WRITE_BUTTON_ID(0), wParam - WRITE_BUTTON_ID(0));
			if (mat == NULL)
				break;
			GetWindowText(DATA::textList[wParam - WRITE_BUTTON_ID(0)], tmpBuffer, 200);
			path = tmpBuffer;
			saveImage(mat, ssDouble.str(), "\\crop_" + path.string());
			delete mat;
			mat = NULL;
			break;
		}
		::InvalidateRect(hwnd, NULL, TRUE);
		break;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}


MainWindows::MainWindows(HINSTANCE hInstance, TCHAR* windowClassName) {
	this->hInstance = hInstance;
	this->windowClassName = windowClassName;
}

void MainWindows::RegistMainWondwsClass()
{
	WNDCLASS    wndclass;

	wndclass.style = 0;
	wndclass.lpfnWndProc = sMainWindowProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wndclass.lpszMenuName = this->windowClassName;
	wndclass.lpszClassName = this->windowClassName;
	RegisterClass(&wndclass);

}

void MainWindows::CreateWindows(TCHAR* windowName)
{
	hWnd = ::CreateWindow(this->windowClassName,
		windowName,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU,
		0, 0, 1280, 760,
		NULL, NULL, hInstance, NULL);
}

void MainWindows::MsgFetch()
{
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void MainWindows::Init(HINSTANCE hInstance, TCHAR* windowClassName, TCHAR* windowName)
{
	if (MainWindows::instance == NULL) {
		MainWindows::instance = new MainWindows(hInstance, windowClassName);
		MainWindows::instance->RegistMainWondwsClass();
		MainWindows::instance->CreateWindows(windowName);
	}
}

MainWindows* MainWindows::GetInstance()
{
	if (instance == NULL)
		throw std::runtime_error("Does not init");
	return instance;
}


LRESULT MainWindows::sMainWindowProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	return instance->MainWindowProc(hwnd, iMsg, wParam, lParam);
}

