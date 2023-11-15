#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <filesystem>
#include <windows.h>
#include <commdlg.h>
#include <GL/GL.h>
#include <tchar.h>
#include <chrono>
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "opencv.hpp"
#include "utils.h"
#include "update.h"
#include "cropImage.h"

bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data);
void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void openFiles();
void getArea();
void clearImage();
void drawBox(ImVec4 box, ImVec4 inColor);
void drawImage();
void optionGUI();
void imageSelectGUI();
void imageGUI();
void render();
void callbackSave();

int main(int, char**) {
	wc = { sizeof(wc), CS_OWNDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"Crop Image", NULL };
	::RegisterClassExW(&wc);
	hwnd = ::CreateWindowW(wc.lpszClassName, L"Crop Image", WS_OVERLAPPEDWINDOW, 100, 100, 1660, 1080, NULL, NULL, wc.hInstance, NULL);

	if (!CreateDeviceWGL(hwnd, &g_MainWindow))
	{
		CleanupDeviceWGL(hwnd, &g_MainWindow);
		::DestroyWindow(hwnd);
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return true;
	}
	wglMakeCurrent(g_MainWindow.hDC, g_hRC);

	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_InitForOpenGL(hwnd);
	ImGui_ImplOpenGL3_Init();

	while (!done) {
		MSG msg;
		while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		scale = 1.0f;
		getArea();
		updateKeyboard();
		imageSelectGUI();
		optionGUI();
		imageGUI();
		updateMouse();
		render();
	}

	clearImage();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceWGL(hwnd, &g_MainWindow);
	wglDeleteContext(g_hRC);
	::DestroyWindow(hwnd);
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);
	return 0;
}



bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data) {
	HDC hDc = ::GetDC(hWnd);
	PIXELFORMATDESCRIPTOR pfd = { 0 };
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;

	const int pf = ::ChoosePixelFormat(hDc, &pfd);
	if (pf == 0)
		return false;
	if (::SetPixelFormat(hDc, pf, &pfd) == FALSE)
		return false;
	::ReleaseDC(hWnd, hDc);

	data->hDC = ::GetDC(hWnd);
	if (!g_hRC)
		g_hRC = wglCreateContext(data->hDC);
	return true;
}

void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data) {
	wglMakeCurrent(NULL, NULL);
	::ReleaseDC(hWnd, data->hDC);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			g_Width = LOWORD(lParam);
			g_Height = HIWORD(lParam);
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}


void openFiles() {
	OPENFILENAME ofn;
	size_t size = sizeof(TCHAR) * MAX_PATH * MAX_FILES_SELECT;
	TCHAR* szFile = (TCHAR*)malloc(size);
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = size;
	ofn.lpstrFilter = L"Images\0*.BMP;*.JPG;*.JPEG;*.PNG;*.GIF\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_ALLOWMULTISELECT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

	if (GetOpenFileName(&ofn) == TRUE) {
		TCHAR* p = ofn.lpstrFile;
		std::wstring directory = p;
		p += directory.size() + 1;
		std::wstring slash = L"\\";
		if (IsFile(directory.c_str())) {
			size_t pos = directory.rfind(L'\\');
			directory.clear();
			for (size_t i = 0; i < pos; i++)
				directory.push_back(ofn.lpstrFile[i]);
			p = ofn.lpstrFile + directory.size() + 1;
		}
		while (*p) {

			std::wstring filename = p;
			p += filename.size() + 1;

			std::wstring filePath = directory + slash + filename;
			std::filesystem::path path = filePath.c_str();
			int size_needed = WideCharToMultiByte(CP_UTF8, 0, &filename.c_str()[0], (int)filename.size(), NULL, 0, NULL, NULL);
			std::string utf8_string(size_needed, 0);
			WideCharToMultiByte(CP_UTF8, 0, &filename.c_str()[0], (int)filename.size(), &utf8_string[0], size_needed, NULL, NULL);
			auto str = delimiter(utf8_string);

			if (imagesGL.find(str) != imagesGL.end()) {
				wprintf(L"Exist : %s\n", filePath.c_str());
				continue;
			}
			cv::Mat image = cv::imread(path.string(), cv::IMREAD_COLOR);
			cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
			cv::Mat img;
			cv::cvtColor(image, img, cv::COLOR_RGB2RGBA);
			
			
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.cols, img.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data);
			glFlush();
			glFinish();
			glBindTexture(GL_TEXTURE_2D, 0);

			imagesGL.insert({ str, texture });
			imageSize.insert({ str, {image.cols,image.rows}});
			onWindow.insert({ str, false });
			imagesCV.insert({ str, image });
			wprintf(L"Add : %s\n", filePath.c_str());
		}
	}
	free(szFile);
}

void clearImage() {
	for (auto& image : imagesGL)
		glDeleteTextures(1, &image.second);
	imagesGL.clear();
}

void getArea() {
	std::pair<int, int> size;
	int middle_w = g_Width / 2;
	int middle_h = g_Height / 2;
	if (textureName != "-1") {
		size = imageSize[textureName];

		while (size.first < 800 || size.second < 800) {
			size.first *= 1.1f;
			size.second *= 1.1f;
			scale *= 1.1f;
		}

		while (size.first > 1200 || size.second > 1200) {
			size.first *= 0.9f;
			size.second *= 0.9f;
			scale *= 0.9f;
		}
		area.x = middle_w = middle_w - size.first / 2;
		area.y = middle_h - size.second / 2;
		area.z = size.first;
		area.w = size.second;
	}
	else {
		size = { 800,800 };
		area.x = middle_w = middle_w - size.first / 2;
		area.y = middle_h - size.second / 2;
		area.z = size.first;
		area.w = size.second;
	}
}

void drawBox(ImVec4 box, ImVec4 inColor) {
	box.x = (box.x - 0.5) * 2;
	box.y = (box.y - 0.5) * 2;
	box.z = (box.z - 0.5) * 2;
	box.w = (box.w - 0.5) * 2;
	box.w *= -1;
	box.y *= -1;
	glLineWidth(boxSize * scale);
	glColor4f(inColor.x, inColor.y, inColor.z, inColor.w);
	glBegin(GL_LINE_LOOP);
	glVertex2f(box.x, box.y);
	glVertex2f(box.z, box.y);
	glVertex2f(box.z, box.w);
	glVertex2f(box.x, box.w);
	glEnd();
	glColor4f(1, 1, 1, 1);
}


void optionGUI() {
	ImGui::Begin("Option");
	ImGui::SetWindowPos({ 0, selectWindowSize.y });
	char str[100];
	sprintf_s(str, 100, "Box List Size : %zu", boxList.size());
	ImGui::Text(str);

	ImGui::Separator();
	ImGui::Text("Exit Mode : ctrl + c");
	ImGui::Text("Capture Mode : ctrl + x");
	ImGui::Text("Capture Box : ctrl + s");
	ImGui::Text("Reset Capture : ctrl + r");
	ImGui::Separator();
	ImGui::Text("Close Image : ctrl + w");
	ImGui::Text("Delete Image : ctrl + z");
	ImGui::Separator();
	ImGui::Checkbox("Free", &isFree);
	ImGui::DragInt("Box Size", &boxSize, 1, 1, 20);
	ImGui::ColorEdit4("Color", (float*)&color);
	ImGui::Separator();
	if (ImGui::Button("Save Images")) 
		callbackSave();
	ImGui::End();
}

void imageSelectGUI() {
	ImGui::Begin("Image Select");
	selectWindowSize = ImGui::GetWindowSize();
	ImGui::SetWindowPos({ 0,0 });
	if (ImGui::Button("Image Select"))
		openFiles();
	ImGui::Separator();
	for (const auto& file : imagesGL)
		if (ImGui::Selectable(file.first.c_str()))
			if (textureName != file.first)
				onWindow[file.first] = !onWindow[file.first];
	ImGui::End();
}

void imageGUI() {
	for (auto& it : onWindow) {
		if (it.second) {
			ImGui::SetNextWindowPos(ImVec2(area.x + area.w + area.x / 4, 100), ImGuiCond_Once);
			ImGui::SetNextWindowSize(ImVec2(area.x / 2, area.x / 2), ImGuiCond_Once);
			ImGui::Begin(it.first.c_str());
			ImVec2 windowSize = ImGui::GetContentRegionAvail();
			float boxScale = std::max(imageSize[it.first].first / windowSize.x, imageSize[it.first].second / windowSize.y);
			ImVec2 paddingSize = { ImGui::GetStyle().WindowPadding.x, ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2 + ImGui::GetStyle().WindowPadding.y };
			ImGui::Image(reinterpret_cast<ImTextureID>(imagesGL[it.first]), ImVec2(windowSize.x, windowSize.y));
			if (textureName != "-1") {
				ImVec2 min = { ImGui::GetWindowPos().x + currBox.x * windowSize.x + paddingSize.x, ImGui::GetWindowPos().y + currBox.y * windowSize.y + paddingSize.y };
				ImVec2 max = { ImGui::GetWindowPos().x + currBox.z * windowSize.x + paddingSize.x, ImGui::GetWindowPos().y + currBox.w * windowSize.y + paddingSize.y };
				ImGui::GetWindowDrawList()->AddRect(min, max, ImVec2ImU32(color), 0, 0, boxSize / boxScale);
			}
			for (auto& it : boxList) {
				ImVec4 box = it.first;
				ImVec2 min = { ImGui::GetWindowPos().x + box.x * windowSize.x + paddingSize.x, ImGui::GetWindowPos().y + box.y * windowSize.y + paddingSize.y };
				ImVec2 max = { ImGui::GetWindowPos().x + box.z * windowSize.x + paddingSize.x, ImGui::GetWindowPos().y + box.w * windowSize.y + paddingSize.y };
				ImGui::GetWindowDrawList()->AddRect(min, max, ImVec2ImU32(it.second), 0, 0, boxSize / boxScale);
			}
			updateKeyboard(it.first);
			ImGui::End();
		}
	}
}

void drawImage() {
	glPushMatrix();
	glRotatef(180, 1, 0, 0);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.f, -1.f);
	glTexCoord2f(1.0f, 0.0f); glVertex2f(1.f, -1.f);
	glTexCoord2f(1.0f, 1.0f); glVertex2f(1.f, 1.f);
	glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.f, 1.f);
	glEnd();
	glPopMatrix();
}

void render() {
	ImGui::Render();
	glEnable(GL_TEXTURE_2D);
	glViewport(0, 0, g_Width, g_Height);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0f, 1.0f, 1.0f);
	if (textureName != "-1") {
		glViewport(area.x, area.y, area.z, area.w);
		glBindTexture(GL_TEXTURE_2D, imagesGL[textureName]);
		drawImage();
		glBindTexture(GL_TEXTURE_2D, 0);
		drawBox(currBox, color);
		for (auto& it : boxList)
			drawBox(it.first, it.second);
	}
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	::SwapBuffers(g_MainWindow.hDC);
}

void callbackSave() {
	std::wstring buffer;
	buffer.resize(MAX_PATH);
	GetModuleFileName(NULL, (LPWSTR)buffer.c_str(), MAX_PATH);
	buffer.resize(buffer.rfind(L'\\'));
	buffer += L"\\Result";
	initSaveFolder(buffer);
	std::wstringstream timeString;
	timeString.str(L"");
	timeString << "\\" << time(NULL) << "\\";
	buffer += timeString.str();
	initSaveFolder(buffer);

	
	for (auto& it : imagesCV) {
		int cropI = 0;
		auto& name = it.first;
		auto mat = it.second.clone();
		for (auto& save : boxList) {
			cropI++;
			drawRect(&mat, save.first, save.second, imageSize[name], boxSize);
			std::string cropName = std::to_string(cropI) + "_" + name;
			saveImage(cropMat(&it.second, save.first, imageSize[name]), buffer, cropName);
		}
		saveImage(mat, buffer, name);
	}
}