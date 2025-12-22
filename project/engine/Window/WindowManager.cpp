#include "WindowManager.h"
#include "engine/Debug/Logger/Log.h"
#include "engine/Utilities/Conversion/ConvertString.h"

namespace NoEngine {
using namespace std;
namespace {
unordered_map<HWND, unique_ptr<Window>> sWindowMap;
unordered_map<std::wstring, HWND> sHWNDMap;
wstring sMainWindowName;
}

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

Window* WindowManager::Create(std::wstring title, uint32_t width, uint32_t height, const std::wstring& iconPath) {
	Log::DebugPrint("WindowManager_WindowCreateStart title : " + ConvertString(title));
	unique_ptr<Window> window = make_unique<Window>();
	window->Create(WindowProc, title, width, height, iconPath);
	HWND hwnd = window->GetWindowHandle();
	sWindowMap[hwnd] = move(window);
	sHWNDMap[title] = hwnd;
	Log::DebugPrint("WindowManager_WindowCreated title : " + ConvertString(title));
	return sWindowMap[hwnd].get();
}

void WindowManager::Shutdown() {
	Log::DebugPrint("WindowManager Shutdown", VerbosityLevel::kInfo);
	sWindowMap.clear();
}

bool WindowManager::ProcessMessage() {
	for (auto it = sWindowMap.begin(); it != sWindowMap.end();) {
		Window* window = it->second.get();

		if (window->IsDead()) {
			Log::DebugPrint("Window is dead");
			if (window->GetTitleName() == sMainWindowName) {
				Log::DebugPrint("MainWindow is dead", VerbosityLevel::kInfo);
				isDead_ = true;
			}
			// ToDo : sHWNDMapもsWindowMapの要素削除と同時に削除すべきです。
			it = sWindowMap.erase(it);
			Log::DebugPrint("Window is erase");
		} else {
			++it;
		}
	}

	if (!sWindowMap.empty()) {
		for (auto& pair : sWindowMap) {

			bool isFailure = pair.second->ProcessMessage();

			if (isFailure) {
				Log::DebugPrint("ProcessMessage is failure", VerbosityLevel::kCritical);
				return true;
			}
		}
	}

	if (isDead_ == true) {
		return true;
	}

	return false;
}

void WindowManager::SetMainWindowName(std::wstring title) {
	sMainWindowName = title;
	Log::DebugPrint("SetMainWindowName : " + ConvertString(title), VerbosityLevel::kInfo);
}

Window* WindowManager::GetWindow(const std::wstring& windowTitle) {
	return sWindowMap[sHWNDMap[windowTitle]].get();
}

void WindowManager::Clear(GraphicsContext& context) {
	for (auto& window : sWindowMap) {
		if (!window.second->IsDead()) {
			window.second->Clear(context);
		}
	}
}

void WindowManager::EndFrame(GraphicsContext& context){
	for (auto& window : sWindowMap) {
		if (!window.second->IsDead()) {
			window.second->EndFrame(context);
		}
	}
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	// ImGuiにメッセージを渡します。ImGuiが処理中なら後続の処理を打ち切ります。
	/*if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wpram, lparam)) {
		return true;
	}*/
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

