#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <filesystem>
#include <windef.h>
#include <wingdi.h>
#include "opencv.hpp"
#include "imgui.h"


#ifndef GetRValue	
#define GetRValue(rgb)	( (BYTE)(rgb) )
#endif
#ifndef GetGValue
#define GetGValue(rgb)	( (BYTE)(((WORD)(rgb))>>8) )
#endif
#ifndef GetBValue
#define GetBValue(rgb)	( (BYTE)((rgb)>>16) )
#endif

void saveImage(cv::Mat mat, std::wstring dir, cv::String file) {
	char charDir[MAX_PATH];
	size_t size = MAX_PATH;
	wcstombs_s(&size, charDir, dir.c_str(), MAX_PATH);
	cv::String path = cv::String(charDir);

	cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
	cv::imwrite(path + file, mat);
}

void initSaveFolder(std::wstring dir) {
	char charDir[MAX_PATH];
	size_t size = MAX_PATH;
	wcstombs_s(&size, charDir, dir.c_str(), MAX_PATH);
	cv::String path = cv::String(charDir);

	if (!std::filesystem::exists(path))
		std::filesystem::create_directory(path);
}

void drawPixsel(cv::Vec3b& pixel, COLORREF color) {
	pixel[2] = GetBValue(color);
	pixel[1] = GetGValue(color);
	pixel[0] = GetRValue(color);
}

void drawRect(cv::Mat* mat, COLORREF color, int width, int height, int top, int bottom, int left, int right) {
	for (int y = 0; y <= height; y++) {
		cv::Vec3b& matPixelLeft = mat->at<cv::Vec3b>(top + y, left);
		cv::Vec3b& matPixelRight = mat->at<cv::Vec3b>(top + y, right);
		drawPixsel(matPixelLeft, color);
		drawPixsel(matPixelRight, color);
	}
	for (int x = 0; x <= width; x++) {
		cv::Vec3b& matPixelTop = mat->at<cv::Vec3b>(top, left + x);
		cv::Vec3b& matPixelBottom = mat->at<cv::Vec3b>(bottom, left + x);
		drawPixsel(matPixelTop, color);
		drawPixsel(matPixelBottom, color);
	}
}

void drawRect(cv::Mat* mat, ImVec4 box, ImVec4 colorVec, std::pair<int, int> imageSize, int thickness) {
	COLORREF color = RGB(colorVec.x * 255, colorVec.y * 255, colorVec.z * 255);
	int width = std::ceil(imageSize.first * (box.z - box.x));
	int height = std::ceil(imageSize.second * (box.w - box.y));
	int top = std::ceil(imageSize.second * box.y);
	int bottom = top + height;
	int left = std::ceil(imageSize.first * box.x);
	int right = left + width;

	for (int i = 0;i < thickness;i++)
		drawRect(mat, color, width - i * 2, height - i * 2, top + i, bottom - i, left + i, right - i);
}


cv::Mat cropMat(cv::Mat* mat, ImVec4 box, std::pair<int, int> imageSize) {
	int width = std::ceil(imageSize.first * (box.z - box.x));
	int height = std::ceil(imageSize.second * (box.w - box.y));
	int top = std::ceil(imageSize.second * box.y);
	int left = std::ceil(imageSize.first * box.x);
	cv::Rect roi(left, top, width, height);
	return (*mat)(roi).clone();
}