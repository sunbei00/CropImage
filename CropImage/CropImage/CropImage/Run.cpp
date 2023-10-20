#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <filesystem>
#include <windows.h>
#include <commdlg.h>
#include <GL/GL.h>
#include <tchar.h>
#include <string>
#include <map>
#include "opencv.hpp"
#include "utils.h"

struct WGL_WindowData { HDC hDC; };

static HGLRC            g_hRC;
static WGL_WindowData   g_MainWindow;
static int              g_Width;
static int              g_Height;

bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data);
void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void openFiles(bool isMulti);
void clearImage();
void updateKeyboard(std::string);
void updateKeyboard();
void updateMouse();

std::map<std::string, unsigned int> imagesGL;
std::map<std::string, cv::Mat> imagesCV;
std::map<std::string, ImVec2> imageSize;
std::map<std::string, bool> onWindow;

WNDCLASSEXW wc;
HWND hwnd;
float boxSize = 1.0;
bool isFree = false;
std::string textureName = "-1";
ImVec4 color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);


ImVec4 area;
ImVec4 currBox;

int main(int, char**)
{

#pragma region IMGUI_SETTING
	wc = { sizeof(wc), CS_OWNDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"Crop Image", NULL };
	::RegisterClassExW(&wc);
	hwnd = ::CreateWindowW(wc.lpszClassName, L"Crop Image", WS_OVERLAPPEDWINDOW, 100, 100, 1660, 1080, NULL, NULL, wc.hInstance, NULL);

	if (!CreateDeviceWGL(hwnd, &g_MainWindow))
	{
		CleanupDeviceWGL(hwnd, &g_MainWindow);
		::DestroyWindow(hwnd);
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
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
#pragma endregion

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	glEnable(GL_TEXTURE_2D);

	bool done = false;
	while (!done)
	{

#pragma region MSG
		MSG msg;
		while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;
#pragma endregion 
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

#pragma region MIDDLE_AREA
		float scale = 1.0f;
		if (textureName != "-1") {
			int middle_w = g_Width / 2;
			int middle_h = g_Height / 2;
			auto size = imageSize[textureName];
			
			while (size.x < 800 || size.y < 800) {
				size.x *= 1.3f;
				size.y *= 1.3f;
				scale *= 1.3f;
			}
			area.x = middle_w = middle_w - size.x / 2;
			area.y = middle_h - size.y / 2;
			area.z = size.x;
			area.w = size.y;
		}
#pragma endregion
#pragma region KEYBOARD
		updateKeyboard();
#pragma endregion
#pragma region IMAGE_SELECT
		ImGui::Begin("Image Select");
		ImGui::SetWindowPos({ 0,0 });
		auto selectWindowSize = ImGui::GetWindowSize();
		if (ImGui::Button("Single Image"))
			openFiles(false);
		if (ImGui::Button("Multiple Image"))
			openFiles(true);
		ImGui::NewLine();
		for (const auto& file : imagesGL)
			if (ImGui::Selectable(file.first.c_str()))
				if (textureName != file.first)
					onWindow[file.first] = !onWindow[file.first];
		ImGui::End();
#pragma endregion
#pragma region IMAGE_GUI
		for (auto& it : onWindow) {
			if (it.second) {
				ImGui::Begin(it.first.c_str());
				ImVec2 windowSize = ImGui::GetWindowSize();
				float windowWidth = windowSize.x;
				float windowHeight = windowSize.y;
				ImGui::Image(reinterpret_cast<ImTextureID>(imagesGL[it.first]), ImVec2(windowWidth, windowHeight));
				updateKeyboard(it.first);
				ImGui::End();
			}
		}
#pragma endregion
#pragma region OPTION
		ImGui::Begin("Option");
		ImGui::SetWindowPos({ 0,selectWindowSize.y });
		ImGui::DragFloat("Box Size", &boxSize, 0.1f, 0.01, 20);
		ImGui::Text("Capture Mode : ctrl + x");
		ImGui::Text("Undo Capture : ctrl + z");
		ImGui::Text("Exit Mode : ctrl + c");
		ImGui::Text("Close Image : ctrl + w");
		ImGui::Text("Delete Image : ctrl + r");
		ImGui::Text("Save Box : ctrl + s");
		ImGui::Checkbox("Free", &isFree);
		ImGui::ColorEdit4("Color", (float*)&color); 
		if (ImGui::Button("Reset Box")) {

		}
		if (ImGui::Button("Save Images")) {

		}
		ImGui::End();
#pragma endregion
#pragma region MOUSE
		updateMouse();
#pragma endregion
#pragma region RENDERING
		ImGui::Render();
		glViewport(0, 0, g_Width, g_Height);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		if (textureName != "-1") {
			glBindTexture(GL_TEXTURE_2D, imagesGL[textureName]);
			glViewport(area.x, area.y, area.z, area.w);

			glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.f, -1.f);
			glTexCoord2f(1.0f, 0.0f); glVertex2f(1.f, -1.f);
			glTexCoord2f(1.0f, 1.0f); glVertex2f(1.f, 1.f);
			glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.f, 1.f);
			glEnd();


			ImVec4 box = currBox;
			box.x = (box.x - 0.5) * 2;
			box.y = (box.y - 0.5) * 2;
			box.z = (box.z - 0.5) * 2;
			box.w = (box.w - 0.5) * 2;
			box.w *= -1;
			box.y *= -1;
			glLineWidth(boxSize * scale);
			glBegin(GL_LINE_LOOP);
			glVertex2f(box.x, box.y);
			glVertex2f(box.z, box.y);
			glVertex2f(box.z, box.w);
			glVertex2f(box.x, box.w);
			glEnd();
			glColor3f(1.0f, 1.0f, 1.0f);

			glBindTexture(GL_TEXTURE_2D, 0);
		}

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		::SwapBuffers(g_MainWindow.hDC);
#pragma endregion
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

void updateMouse() {
	ImGuiIO& io = ImGui::GetIO();
	if (textureName == "-1")
		return;
	if (ImGui::IsWindowFocused() || ImGui::IsWindowHovered())
		return;
	float mouseX = io.MousePos.x;
	float mouseY = io.MousePos.y;

	static bool isDown = false;
	if (io.MouseDown[0] && !isDown) {

		if (mouseX <= area.x || mouseX >= area.x + area.z)
			return;
		if (mouseY <= area.y || mouseY >= area.y + area.w)
			return;
		printf("down\n");
		isDown = true;
		currBox.x = (mouseX - area.x) / area.z;
		currBox.y = (mouseY - area.y) / area.w;
		currBox.z = currBox.x;
		currBox.w = currBox.y;
	}
	if (io.MouseDown[0]) {

		if (mouseX <= area.x || mouseX >= area.x + area.z)
			return;
		if (mouseY <= area.y || mouseY >= area.y + area.w)
			return;
	}

	if (!io.MouseDown[0]) {
		if (isDown) {
			isDown = false;
			if (textureName == "-1")
				return;
			printf("up\n");
			currBox.z = (mouseX - area.x) / area.z;
			currBox.w = (mouseY - area.y) / area.w;
			currBox.z = std::clamp(currBox.z, 0.f, 1.f);
			currBox.w = std::clamp(currBox.w, 0.f, 1.f);

			float tmp = std::min(currBox.z, currBox.x);
			currBox.z = std::max(currBox.z, currBox.x);
			currBox.x = tmp;
			tmp = std::min(currBox.w, currBox.y);
			currBox.w = std::max(currBox.w, currBox.y);
			currBox.y = tmp;
		}
	}

}

void updateKeyboard(std::string name) {
	ImGuiIO& io = ImGui::GetIO();
	if (!ImGui::IsWindowFocused())
		return;
	if (!io.KeyCtrl)
		return;

	if (io.KeysDown['W'])
		onWindow[name] = false;
	if (io.KeysDown['R']) {
		onWindow.erase(name);
		glDeleteTextures(1, &imagesGL[name]);
		imagesGL.erase(name);
		imagesCV.erase(name);
		imageSize.erase(name);
	}
	if (io.KeysDown['X'] && textureName == "-1") {
		onWindow[name] = false;
		textureName = name;
	}

}

void updateKeyboard() {
	ImGuiIO& io = ImGui::GetIO();
	if (GetForegroundWindow() != hwnd && ImGui::IsWindowFocused())
		return;
	if (!io.KeyCtrl)
		return;;
	if (io.KeysDown['C'] && textureName != "-1") {
		onWindow[textureName] = true;
		textureName = "-1";
	}
	if (io.KeysDown['Z']) {

	}
}

bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
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

void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
	wglMakeCurrent(NULL, NULL);
	::ReleaseDC(hWnd, data->hDC);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
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
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}


#define MAX_FILES_SELECT 100
void openFiles(bool isMulti) {
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
	if (isMulti) {
		ofn.Flags = OFN_ALLOWMULTISELECT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

		if (GetOpenFileName(&ofn) == TRUE) {
			TCHAR* p = ofn.lpstrFile;
			std::wstring directory = p;
			p += directory.size() + 1;
			std::wstring slash = L"\\";
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
					wprintf(L"Exist : %s%s\n", directory.c_str(), filename.c_str());
					continue;
				}
				cv::Mat image = cv::imread(path.string(), cv::IMREAD_COLOR);
				cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
				unsigned int texture;
				glGenTextures(1, &texture);
				glBindTexture(GL_TEXTURE_2D, texture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);
				glBindTexture(GL_TEXTURE_2D, 0);

				imagesGL.insert({ str, texture });
				imageSize.insert({ str, ImVec2{(float)image.cols,(float)image.rows} });
				onWindow.insert({ str, false });
				imagesCV.insert({ str, image });
				wprintf(L"Add : %s%s\n", directory.c_str(), filename.c_str());
			}
		}
	}
	else {
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if (GetOpenFileName(&ofn) == TRUE) {
			std::filesystem::path path = ofn.lpstrFile;
			int size_needed = WideCharToMultiByte(CP_UTF8, 0, &ofn.lpstrFile[0], (int)ofn.lStructSize, NULL, 0, NULL, NULL);
			std::string utf8_string(size_needed, 0);
			WideCharToMultiByte(CP_UTF8, 0, &ofn.lpstrFile[0], (int)ofn.lStructSize, &utf8_string[0], size_needed, NULL, NULL);
			auto str = delimiter(utf8_string);

			if (imagesGL.find(str) != imagesGL.end()) {
				printf("Exist : %s\n", utf8_string.c_str());
				goto finish;
			}
			cv::Mat image = cv::imread(path.string(), cv::IMREAD_COLOR);
			cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
			unsigned int texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, image.data);
			glBindTexture(GL_TEXTURE_2D, 0);

			imagesGL.insert({ str, texture });
			imageSize.insert({ str, ImVec2{(float)image.cols,(float)image.rows} });
			onWindow.insert({ str, false });
			imagesCV.insert({ str, image });
			printf("Add : %s\n", utf8_string.c_str());
		}
	}

finish:;
	free(szFile);
}



void clearImage() {
	for (auto& image : imagesGL)
		glDeleteTextures(1, &image.second);
	imagesGL.clear();
}