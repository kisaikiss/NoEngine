#pragma once
namespace NoEngine {

struct WindowCore {
    std::wstring title;          // ウィンドウタイトル
    WNDCLASSEX wcex{};              // ウィンドウクラス
    HWND hwnd{};                // ウィンドウハンドル
    HINSTANCE hInstance{};      // インスタンスハンドル
    DWORD windowStyle{};        // ウィンドウスタイル
};

}