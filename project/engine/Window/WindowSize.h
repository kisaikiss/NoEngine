#pragma once
namespace NoEngine {
struct WindowSize {
    uint32_t clientWidth = 0;    // クライアント領域の幅
    uint32_t clientHeight = 0;   // クライアント領域の高さ
    float aspectRatio = 0.0f;   // アスペクト比
    RECT windowRect{};          // ウィンドウ矩形
    RECT clientRect{};          // クライアント矩形
};
}