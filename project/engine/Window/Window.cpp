#include "Window.h"

#include "engine/Functions/Debug/Logger/Log.h"
#include "engine/Utilities/Conversion/ConvertString.h"

#include "Event/Default/CloseEvent.h"
#include "Event/Default/EnterSizeMoveEvent.h"
#include "Event/Default/GetMinMaxInfoEvent.h"
#include "Event/Default/SizeEvent.h"
#include "Event/Default/SizingEvent.h"

#include "engine/Runtime/Command/GraphicsContext.h"
#include "engine/Runtime/GraphicsCore.h"
#include "externals/imgui/imgui.h"

#include "resource.h"

namespace NoEngine {
using namespace std;
Window::~Window() {
	Destroy();
}

void Window::Create(WNDPROC windowProc, std::wstring title, uint32_t width, uint32_t height, const std::wstring& iconPath) {
	LogInfo("Window_WindowCreateStart title : " + ConvertString(title));
	core_.title = title;
	isDead_ = false;
	isResize_ = false;
	core_.hwnd = nullptr;	//ウィンドウハンドル

	size_.clientWidth = width;
	size_.clientHeight = height;
	if (size_.clientHeight > 0) {
		size_.aspectRatio = static_cast<float>(size_.clientWidth) / static_cast<float>(size_.clientHeight);
	} else {
		size_.aspectRatio = 1.0f;
	}
	// クライアント領域のサイズからウィンドウサイズを計算
	size_.clientRect = { 0, 0, LONG(size_.clientWidth), LONG(size_.clientHeight) };
	size_.windowRect = size_.clientRect;

	core_.wcex.cbSize = sizeof(WNDCLASSEX);
	core_.wcex.style = CS_HREDRAW | CS_VREDRAW;
	core_.wcex.lpfnWndProc = windowProc;
	core_.wcex.cbClsExtra = 0;
	core_.wcex.cbWndExtra = 0;
	core_.wcex.hInstance = GetModuleHandle(nullptr);
	core_.wcex.hIcon = (HICON)LoadImage(core_.wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
	core_.wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	core_.wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	core_.wcex.lpszMenuName = nullptr;
	core_.wcex.lpszClassName = title.c_str();
	core_.windowStyle = 0;
	//core_.wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

	 // アイコンのカスタマイズが必要ならここで読み込む
	if (!iconPath.empty()) {
		// アイコンの読み込み
		HICON hIcon = static_cast<HICON>(LoadImage(
			core_.hInstance,
			iconPath.c_str(),
			IMAGE_ICON,
			256, 256,
			LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED));
		if (hIcon) {
			core_.wcex.hIcon = hIcon;
		}
	}

	//ウィンドウクラスを登録する
	if (!RegisterClassEx(&core_.wcex)) {
		assert(false);
	}
	LogDebug("RegisterWindowClass title : " + ConvertString(title));

	//クライアント領域を元に実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&size_.windowRect, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウの生成
	core_.hwnd = CreateWindow(
		core_.wcex.lpszClassName,		//利用するクラス名
		core_.wcex.lpszClassName,		//タイトルバーの文字
		WS_OVERLAPPEDWINDOW,	//良く見るウィンドウスタイル
		CW_USEDEFAULT,			//表示X座標(Windowsに任せる)
		CW_USEDEFAULT,			//表示Y座標(Windowsに任せる)
		size_.windowRect.right - size_.windowRect.left,	//ウィンドウ横幅
		size_.windowRect.bottom - size_.windowRect.top,	//ウィンドウ縦幅
		nullptr,				//親ウィンドウハンドル
		nullptr,				//メニューハンドル
		core_.wcex.hInstance,	//インスタンスハンドル
		nullptr					//オプション
	);

	if (core_.hwnd == nullptr) {
		assert(false);
	}

	RegisterWindowEvent(make_unique<CloseEvent>());
	RegisterWindowEvent(make_unique<EnterSizeMoveEvent>());
	RegisterWindowEvent(make_unique<GetMinMaxInfoEvent>());
	RegisterWindowEvent(make_unique<SizeEvent>());
	RegisterWindowEvent(make_unique<SizingEvent>());

	// ウィンドウを表示します。
	ShowWindow(core_.hwnd, SW_SHOW);

	LogInfo("Window_WindowCreated title : " + ConvertString(title));
}


void Window::Destroy() {
	if (core_.hwnd) {
		DestroyWindow(core_.hwnd);
	}
	GraphicsCore::sCommandListManager.IdleGPU();
	isDead_ = true;
}

bool Window::ProcessMessage() {
	MSG msg{};
	// メッセージをウィンドウプロシージャに送信
	if (PeekMessage(&msg, core_.hwnd, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);	//キー入力メッセージの処理
		DispatchMessage(&msg);	//ウィンドウプロシージャにメッセージを送る
	}
	return false;
}
void Window::RegisterWindowEvent(std::unique_ptr<IWindowEvent> event) {
	UINT msg = event->GetTargetMassage();
	LogInfo("RegisterWindowEvent WindowTitle : " + ConvertString(core_.title) + ", message : " + to_string(msg));
	if (eventMap_.contains(msg)) {
		eventMap_[msg].reset();
	}
	eventMap_[msg] = move(event);
	eventMap_[msg]->SetWindow(this);
}

std::optional<LRESULT> Window::HandleEvent(UINT msg, WPARAM wparam, LPARAM lparam) {

	auto it = eventMap_.find(msg);
	if (it != eventMap_.end()) {
		return it->second->OnEvent(msg, wparam, lparam);
	}

	// どのリスナーも処理しなかった場合は std::nullopt
	return nullopt;

}

void Window::CalculateAspectRatio() {
	if (size_.clientHeight > 0) {
		size_.aspectRatio = static_cast<float>(size_.clientWidth) / static_cast<float>(size_.clientHeight);
	} else {
		size_.aspectRatio = 1.0f;
	}
}

void Window::SetSizeChangeMode(SizeChangeMode sizeChangeMode) {
	sizeChangeMode_ = sizeChangeMode;

	// ウィンドウスタイルの更新（必要に応じて）
	if (core_.hwnd) {
		LONG style = GetWindowLong(core_.hwnd, GWL_STYLE);

		switch (sizeChangeMode) {
		case SizeChangeMode::kNone:
			LogInfo("Set SizeChangeMode : None");
			// サイズ変更不可
			style &= ~(WS_SIZEBOX | WS_MAXIMIZEBOX);
			break;
		case SizeChangeMode::kNormal:
			LogInfo("Set SizeChangeMode : Normal");
			// 自由変更
			style |= (WS_SIZEBOX | WS_MAXIMIZEBOX);
			break;
		case SizeChangeMode::kFixedAspect:
			LogInfo("Set SizeChangeMode : FixedAspect");
			// アスペクト比固定（サイズ変更は可能）
			style |= (WS_SIZEBOX | WS_MAXIMIZEBOX);
			break;
		}

		SetWindowLong(core_.hwnd, GWL_STYLE, style);
		SetWindowPos(core_.hwnd, nullptr, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}
}

void Window::SetWindowMode(WindowMode windowMode) {
	if (windowMode_ == windowMode || !core_.hwnd) {
		return;
	}

	windowMode_ = windowMode;

	switch (windowMode) {
	case WindowMode::kWindow:
		LogInfo("Change WindowMode : Window");
		// ウィンドウモード
		size_ = preWindowSize_;
		SetWindowLong(core_.hwnd, GWL_STYLE, core_.windowStyle);
		ShowWindow(core_.hwnd, SW_NORMAL);
		AdjustWindowSize();

		/*SetWindowPos(core_.hwnd, HWND_TOP,
			size_.clientRect.left,
			size_.clientRect.top,
			size_.clientRect.right - size_.clientRect.left,
			size_.clientRect.bottom - size_.clientRect.top,
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);*/
		break;

	case WindowMode::kFullScreen:
		LogInfo("Change WindowMode : FullScreen");

		preWindowSize_ = size_;
		// フルスクリーンモード
		SetWindowLong(core_.hwnd, GWL_STYLE, WS_POPUP);

		// モニターサイズを取得
		MONITORINFO monitorInfo = {};
		monitorInfo.cbSize = sizeof(MONITORINFO);
		GetMonitorInfo(MonitorFromWindow(core_.hwnd, MONITOR_DEFAULTTOPRIMARY), &monitorInfo);

		SetWindowPos(core_.hwnd, HWND_TOP,
			monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top,
			monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
			monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
			SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
		ShowWindow(core_.hwnd, SW_NORMAL);
		break;
	}
}

void Window::SetWindowSize(UINT width, UINT height) {
	size_.clientWidth = width;
	size_.clientHeight = height;
	size_.clientRect = { 0, 0, LONG(size_.clientWidth), LONG(size_.clientHeight) };
	size_.windowRect = size_.clientRect;
	CalculateAspectRatio();
	AdjustWindowSize();
}

void Window::AdjustWindowSize() {
	if (!core_.hwnd) {
		return;
	}
	LogInfo("AdjustWindowSize");
}

void Window::ResizeSignal() {
	isResize_ = true;
}

void Window::Resize() {
	if (!isResize_) return;
	isResize_ = false;
}
}

