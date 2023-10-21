#pragma once
#include "GlocalVariable.h"

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
		
		isDown = true;
		currBox.x = (mouseX - area.x) / area.z;
		currBox.y = (mouseY - area.y) / area.w;
		currBox.z = currBox.x;
		currBox.w = currBox.y;
	}

	if (!io.MouseDown[0]) {
		if (isDown) {
			isDown = false;
			if (textureName == "-1")
				return;
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

			if (!isFree) 
				if (currBox.z - currBox.x > currBox.w - currBox.y)
					currBox.z = currBox.x + (currBox.w - currBox.y);
				else
					currBox.w = currBox.y + (currBox.z - currBox.x);
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
	if (io.KeysDown['Z']) {
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
		return;

	if (io.KeysDown['C'] && textureName != "-1") {
		onWindow[textureName] = true;
		textureName = "-1";
	}
	if (io.KeysDown['R']) 
		boxList.clear();

	if (io.KeysDown['S']) {
		if (boxList.size() != 0) {
			auto& top = boxList[boxList.size() - 1];
			if (top.first.x == currBox.x && top.first.y == currBox.y && top.first.z == currBox.z && top.first.w == currBox.w)
				return;
		}
		boxList.push_back({ currBox, color });
	}
}