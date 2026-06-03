#include "WindowManager.h"
#include "engine/Functions/Debug/Logger/Log.h"
#include "engine/Utilities/Conversion/ConvertString.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // USE_IMGUI

namespace NoEngine {
using namespace std;
namespace {
unordered_map<HWND, unique_ptr<Window>> sWindowMap;
unordered_map<std::wstring, HWND> sHWNDMap;
wstring sMainWindowName;
}

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

Window* WindowManager::Create(std::wstring title, uint32_t width, uint32_t height, const std::wstring& iconPath) {
	LogDebug("WindowManager_WindowCreateStart title : " + ConvertString(title));
	unique_ptr<Window> window = make_unique<Window>();
	window->Create(WindowProc, title, width, height, iconPath);
	HWND hwnd = window->GetWindowHandle();
	sWindowMap[hwnd] = move(window);
	sHWNDMap[title] = hwnd;
	LogDebug("WindowManager_WindowCreated title : " + ConvertString(title));
	return sWindowMap[hwnd].get();
}

void WindowManager::Shutdown() {
	LogInfo("WindowManager Shutdown");
	sWindowMap.clear();
}

bool WindowManager::ProcessMessage() {
	

	if (!sWindowMap.empty()) {
		for (auto& pair : sWindowMap) {

			bool isFailure = pair.second->ProcessMessage();

			if (isFailure) {
				LogCritical("ProcessMessage is failure");
				return true;
			}
		}
	}

	for (auto it = sWindowMap.begin(); it != sWindowMap.end();) {
		Window* window = it->second.get();

		if (window->IsDead()) {
			LogDebug("Window is dead");
			if (window->GetTitleName() == sMainWindowName) {
				LogInfo("MainWindow is dead");
				isDead_ = true;
			}
			// ToDo : sHWNDMapもsWindowMapの要素削除と同時に削除すべきです。
			it = sWindowMap.erase(it);
			LogDebug("Window is erase");
		} else {
			++it;
		}
	}

	if (isDead_ == true) {
		return true;
	}

	return false;
}

void WindowManager::SetMainWindowName(std::wstring title) {
	sMainWindowName = title;
	LogInfo("SetMainWindowName : " + ConvertString(title));
}

Window* WindowManager::GetWindow(const std::wstring& windowTitle) {
	return sWindowMap[sHWNDMap[windowTitle]].get();
}

Window* WindowManager::GetMainWindow() {
	return sWindowMap[sHWNDMap[sMainWindowName]].get();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
#ifdef USE_IMGUI
	// ImGuiにメッセージを渡します。ImGuiが処理中なら後続の処理を打ち切ります。
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) return true;
#endif // USE_IMGUI
	// ウィンドウのインスタンスを取得します。
	auto it = sWindowMap.find(hwnd);
	if (it == sWindowMap.end()) {
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	Window* window = it->second.get();
	assert(window);
	// ウィンドウにイベントの処理をさせます。
	auto result = window->HandleEvent(msg, wparam, lparam);
	if (result.has_value()) {
		return result.value();
	}


	// 標準のメッセージ処理を行います。
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
}

