#pragma once

#include "Window.h"

namespace NoEngine {
/// <summary>
/// ウィンドウを管理するクラス
/// </summary>
class WindowManager {
public:
	Window* Create(std::wstring title, uint32_t width, uint32_t height, const std::wstring& iconPath = L"resources/engine/noicon.ico");

	void Shutdown();

	bool ProcessMessage();

	void SetMainWindowName(std::wstring title);

private:
	bool isDead_ = false;
};

}
