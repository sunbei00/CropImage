#pragma once
#include <Windows.h>
#include "DATA.h"

LRESULT CALLBACK wndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static WCHAR tmp[20];
	static int i = 0;
	static int x, y;
	static BITMAPINFOHEADER bi;
	static PAINTSTRUCT ps;
	static HDC hdc;
	static HPEN hpen;
	static HPEN hpenOld;
	static HBRUSH hbr;
	static HBRUSH hbrOld;
	static BITMAP bitmap;
	static HDC hdcMem;
	static HBITMAP hBitmap;
	static HBITMAP hBitmapOld;


	switch (iMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		break;
	case WM_SIZE:
		break;
	case WM_LBUTTONDOWN:
		GetWindowText(hwnd, tmp, sizeof(WCHAR) * 20);
		i = _wtoi(tmp);
		if (DATA::ImageList[i].b) {
			if (DATA::window_size) {
				DATA::cropList[i].rect.left = LOWORD(lParam);
				DATA::cropList[i].rect.top = HIWORD(lParam);
			}
			else {
				DATA::cropList[i].rect.left = LOWORD(lParam) / 2;
				DATA::cropList[i].rect.top = HIWORD(lParam) / 2;
			}
			DATA::cropList[i].b = true;
			DATA::cropList[i].click = true;
		}
		break;
	case WM_MOUSEMOVE:
		GetWindowText(hwnd, tmp, sizeof(WCHAR) * 20);
		i = _wtoi(tmp);
		if (DATA::ImageList[i].b && DATA::cropList[i].click) {

			if (DATA::window_size) {
				DATA::cropList[i].rect.right = LOWORD(lParam);
				DATA::cropList[i].rect.bottom = HIWORD(lParam);
			}
			else {
				DATA::cropList[i].rect.right = LOWORD(lParam) / 2;
				DATA::cropList[i].rect.bottom = HIWORD(lParam) / 2;
			}

			if (!DATA::check) {
				int width = abs(DATA::cropList[i].rect.left - DATA::cropList[i].rect.right);
				int height = abs(DATA::cropList[i].rect.top - DATA::cropList[i].rect.bottom);
				
				bool rightUp = DATA::cropList[i].rect.right > DATA::cropList[i].rect.left;
				bool bottomUp = DATA::cropList[i].rect.bottom > DATA::cropList[i].rect.top;
				int distance = width > height ? width : height;

				if (rightUp)
					DATA::cropList[i].rect.right = min(DATA::cropList[i].rect.left + distance, 384);
				else
					DATA::cropList[i].rect.right = max(DATA::cropList[i].rect.left - distance, 0);

				if (bottomUp)
					DATA::cropList[i].rect.bottom = min(DATA::cropList[i].rect.top + distance, 384);
				else
					DATA::cropList[i].rect.bottom = max(DATA::cropList[i].rect.top - distance, 0);

			}
			InvalidateRect(hwnd, NULL, TRUE);
		}
		break;
	case WM_LBUTTONUP:
		GetWindowText(hwnd, tmp, sizeof(WCHAR) * 20);
		i = _wtoi(tmp);
		DATA::cropList[i].click = false;
		if (i == 0)
			for (int j = 0; j < 10; j++)
				InvalidateRect(DATA::gridList[j], NULL, TRUE);
		else {
			InvalidateRect(DATA::gridList[0], NULL, TRUE);
			InvalidateRect(DATA::gridList[i], NULL, TRUE);
		}
		break;

	case WM_PAINT:
		GetWindowText(hwnd, tmp, sizeof(WCHAR) * 20);
		i = _wtoi(tmp);

		hdc = BeginPaint(hwnd, &ps);
		if (DATA::ImageList[i].b) {
			bitmap = DATA::ImageList[i].bitmap;

			bi.biSize = sizeof(BITMAPINFOHEADER);
			bi.biWidth = bitmap.bmWidth;
			bi.biHeight = -bitmap.bmHeight;
			bi.biPlanes = 1;
			bi.biBitCount = 24;
			bi.biCompression = BI_RGB;
			bi.biSizeImage = 0;
			bi.biXPelsPerMeter = 0;
			bi.biYPelsPerMeter = 0;
			bi.biClrUsed = 0;
			bi.biClrImportant = 0;

			hdcMem = CreateCompatibleDC(hdc);
			hBitmap = CreateCompatibleBitmap(hdc, bitmap.bmWidth, bitmap.bmHeight);
			hBitmapOld = (HBITMAP)SelectObject(hdcMem, hBitmap);

			SetDIBits(hdcMem, hBitmap, 0, bitmap.bmHeight, bitmap.bmBits, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
			if (DATA::cropList[i].b) {

				hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
				hpen = CreatePen(PS_SOLID, DATA::thick, DATA::cropColorList[i]);
				hbrOld = (HBRUSH)::SelectObject(hdcMem, hbr);
				hpenOld = (HPEN)::SelectObject(hdcMem, (HGDIOBJ)hpen);

				Rectangle(hdcMem, DATA::cropList[i].rect.left, DATA::cropList[i].rect.top, DATA::cropList[i].rect.right, DATA::cropList[i].rect.bottom);

				hbr = (HBRUSH)::SelectObject(hdcMem, hbrOld);
				hpen = (HPEN)::SelectObject(hdcMem, hpenOld);
				DeleteObject(hbr);
				DeleteObject(hpen);
			}
			if (i == 0) {
				for (int j = 1; j < 10; j++) {
					if (!DATA::cropList[j].b)
						continue;
					hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
					hpen = CreatePen(PS_SOLID, DATA::thick, DATA::cropColorList[j]);
					hbrOld = (HBRUSH)::SelectObject(hdcMem, hbr);
					hpenOld = (HPEN)::SelectObject(hdcMem, (HGDIOBJ)hpen);

					Rectangle(hdcMem, DATA::cropList[j].rect.left, DATA::cropList[j].rect.top, DATA::cropList[j].rect.right, DATA::cropList[j].rect.bottom);

					hbr = (HBRUSH)::SelectObject(hdcMem, hbrOld);
					hpen = (HPEN)::SelectObject(hdcMem, hpenOld);
					DeleteObject(hbr);
					DeleteObject(hpen);
				}
			}
			if (i != 0 && DATA::cropList[0].b) {
				hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
				hpen = CreatePen(PS_SOLID, DATA::thick, DATA::cropColorList[0]);
				hbrOld = (HBRUSH)::SelectObject(hdcMem, hbr);
				hpenOld = (HPEN)::SelectObject(hdcMem, (HGDIOBJ)hpen);

				Rectangle(hdcMem, DATA::cropList[0].rect.left, DATA::cropList[0].rect.top, DATA::cropList[0].rect.right, DATA::cropList[0].rect.bottom);

				hbr = (HBRUSH)::SelectObject(hdcMem, hbrOld);
				hpen = (HPEN)::SelectObject(hdcMem, hpenOld);
				DeleteObject(hbr);
				DeleteObject(hpen);
			}
			for (auto& it : DATA::saveCropList) {
				if (i == 0) {
					for (int j = 0; j < 10; j++) {
						if (!DATA::cropList[j].b)
							continue;
						hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
						hpen = CreatePen(PS_SOLID, DATA::thick, it.second);
						hbrOld = (HBRUSH)::SelectObject(hdcMem, hbr);
						hpenOld = (HPEN)::SelectObject(hdcMem, (HGDIOBJ)hpen);

						Rectangle(hdcMem, it.first.rect.left, it.first.rect.top, it.first.rect.right, it.first.rect.bottom);

						hbr = (HBRUSH)::SelectObject(hdcMem, hbrOld);
						hpen = (HPEN)::SelectObject(hdcMem, hpenOld);
						DeleteObject(hbr);
						DeleteObject(hpen);
					}
				}
				if (i != 0 && DATA::cropList[0].b) {
					hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
					hpen = CreatePen(PS_SOLID, DATA::thick, it.second);
					hbrOld = (HBRUSH)::SelectObject(hdcMem, hbr);
					hpenOld = (HPEN)::SelectObject(hdcMem, (HGDIOBJ)hpen);

					Rectangle(hdcMem, it.first.rect.left, it.first.rect.top, it.first.rect.right, it.first.rect.bottom);

					hbr = (HBRUSH)::SelectObject(hdcMem, hbrOld);
					hpen = (HPEN)::SelectObject(hdcMem, hpenOld);
					DeleteObject(hbr);
					DeleteObject(hpen);
				}
			}

			StretchBlt(hdc, 0, 0, bitmap.bmWidth * (DATA::window_size ? 1 : 2), bitmap.bmHeight * (DATA::window_size ? 1 : 2), hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY);

			SelectObject(hdcMem, hBitmapOld);
			DeleteObject(hBitmap);
			DeleteDC(hdcMem);
		}
		EndPaint(hwnd, &ps);
		break;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void registGridWindowClass() {

	WNDCLASS    wndclass;
	RECT        rect;

	wndclass.style = 0;
	wndclass.lpfnWndProc = wndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = DATA::HInstance;
	wndclass.hIcon = NULL;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = DATA::GridClassName;
	wndclass.lpszClassName = DATA::GridClassName;
	RegisterClass(&wndclass);
}


