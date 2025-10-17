#include "WindowManager.h"

#include <cassert>
#include <cstdint>

namespace NoEngine {
	LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

	void WindowManager::Initialize(std::wstring titleName) {
		HWND hWnd = nullptr;	//ウィンドウハンドル
		WNDCLASSEX wcex{};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WindowProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = GetModuleHandle(nullptr);
        //wcex.hIcon = LoadIcon(hInst, IDI_APPLICATION);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = L"NoWindowClass";
        //wcex.hIconSm = LoadIcon(hInst, IDI_APPLICATION);

		//ウィンドウクラスを登録する
		if (!RegisterClassEx(&wcex)) {
			assert(false);
		}

		//ウィンドウサイズを表す構造体にクライアント領域を入れる
		uint32_t windowWidth = 1280;
		uint32_t windowHeight = 720;

		RECT wrc = { 0,0,windowWidth,windowHeight };

		//クライアント領域を元に実際のサイズにwrcを変更してもらう
		AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

		//ウィンドウの生成
		hWnd = CreateWindow(
			wcex.lpszClassName,		//利用するクラス名
			titleName.c_str(),		//タイトルバーの文字
			WS_OVERLAPPEDWINDOW,	//良く見るウィンドウスタイル
			CW_USEDEFAULT,			//表示X座標(Windowsに任せる)
			CW_USEDEFAULT,			//表示Y座標(Windowsに任せる)
			wrc.right - wrc.left,	//ウィンドウ横幅
			wrc.bottom - wrc.top,	//ウィンドウ縦幅
			nullptr,				//親ウィンドウハンドル
			nullptr,				//メニューハンドル
			wcex.hInstance,	//インスタンスハンドル
			nullptr					//オプション
		);

		if (hWnd == nullptr) {
			assert(false);
		}
		//ウィンドウを表示する
		ShowWindow(hWnd, SW_SHOW);
	}

	bool WindowManager::ProcessMessage() {
		MSG msg{};	//メッセージ

		//Windowにメッセージが来てたら最優先で処理させる
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);	//キー入力メッセージの処理
			DispatchMessage(&msg);	//ウィンドウプロシージャにメッセージを送る
		}

		if (msg.message == WM_QUIT) {
			return true;
		}

		return false;
	}

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wpram, LPARAM lparam) {
		//ImGuiにメッセージを渡す。ImGuiが処理中なら後続の処理を打ち切る
		/*if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wpram, lparam)) {
			return true;
		}*/
		//メッセージに応じてゲーム固有の処理を行う
		switch (msg) {
			//ウィンドウが破棄された
		case WM_DESTROY:
			//OSに対して、アプリの終了を伝える
			PostQuitMessage(0);
			return 0;
		}
		//標準のメッセージ処理を行う
		return DefWindowProc(hwnd, msg, wpram, lparam);
    }
}

