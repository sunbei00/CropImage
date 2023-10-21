#pragma once
#include <string>
#include <Windows.h>
#include "imgui.h"



std::string delimiter(std::string str, std::string delimiter = "\\") {
    size_t pos = 0;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        str.erase(0, pos + delimiter.length());
    }
    return str;
}


bool IsFile(const WCHAR* path) {
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesEx(path, GetFileExInfoStandard, &data))
        return false;
    else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        return false;
    else
        return true;
}

size_t wDelimiterPos(const std::wstring& str, const std::wstring& delimiter = L"\\") {
    size_t pos = str.rfind(delimiter);  
    return (pos != std::wstring::npos) ? pos + delimiter.length() : 0;
}

ImU32 ImVec2ImU32(ImVec4 color) {
	return IM_COL32(color.x * 255, color.y * 255 , color.z * 255, color.w * 255);
}



#define MAX_FILES_SELECT 100