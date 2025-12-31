#pragma once
#include "Event/IWindowEvent.h"
#include "WindowCore.h"
#include "WindowSize.h"

#include "../Runtime/Graphics/GraphicsSwapChain.h"
#include "engine/Runtime/GpuResource/PixelBuffer/ColorBuffer.h"
#include "engine/Runtime/GpuResource/PixelBuffer/DepthBuffer.h"

namespace NoEngine {

class Window {
public:

	~Window();

	/// <summary>
	/// サイズ変更モード
	/// </summary>
	enum class SizeChangeMode {
		kNone,           // サイズ変更不可
		kNormal,         // 自由変更
		kFixedAspect,    // アスペクト比固定
	};

	/// <summary>
	/// ウィンドウモード
	/// </summary>
	enum class WindowMode {
		kWindow,         // ウィンドウ
		kFullScreen,     // フルスクリーン
	};

	/// <summary>
	/// ウィンドウを生成します。
	/// </summary>
	/// <param name="windowProc">ウィンドウプロシージャ</param>
	/// <param name="title">ウィンドウタイトル</param>
	/// <param name="width">ウィンドウの幅</param>
	/// <param name="height">ウィンドウの高さ</param>
	/// <param name="iconPath">アイコンパス</param>
	void Create(WNDPROC windowProc, std::wstring title, uint32_t width, uint32_t height, const std::wstring& iconPath = L"");

	/// <summary>
	/// ウィンドウに表示されている画面をクリアします。
	/// </summary>
	void Clear(GraphicsContext& context);

	/// <summary>
	/// ウィンドウ
	/// </summary>
	void EndFrame(GraphicsContext& context);

	/// <summary>
	/// ウィンドウハンドルを取得
	/// </summary>
	/// <returns>ウィンドウハンドル</returns>
	[[nodiscard]]
	HWND GetWindowHandle() const noexcept { return core_.hwnd; }

	/// <summary>
	/// ウィンドウを破壊する
	/// </summary>
	void Destroy();

	/// <summary>
	/// プロセスメッセージ
	/// </summary>
	/// <returns>true == 失敗、false == 成功</returns>
	bool ProcessMessage();

	/// <summary>
	/// ウィンドウイベントを追加
	/// </summary>
	/// <param name="event">追加したいウィンドウイベント</param>
	void RegisterWindowEvent(std::unique_ptr<IWindowEvent> event);

	/// <summary>
	/// ウィンドウプロシージャ内で行うイベント
	/// </summary>
	/// <param name="msg"></param>
	/// <param name="wparam"></param>
	/// <param name="lparam"></param>
	/// <returns></returns>
	std::optional<LRESULT> HandleEvent(UINT msg, WPARAM wparam, LPARAM lparam);

	void CalculateAspectRatio();

	/// <summary>
	/// 死んでるかどうか
	/// </summary>
	/// <returns></returns>
	bool IsDead() const { return isDead_; }

	std::wstring GetTitleName() const { return core_.title; }
	float GetAspectRatio() const noexcept { return size_.aspectRatio; }
	SizeChangeMode GetSizeChangeMode() const noexcept { return sizeChangeMode_; }
	WindowSize& GetWindowSize()noexcept { return size_; }
	void SetSizeChangeMode(SizeChangeMode sizeChangeMode);
	void SetWindowMode(WindowMode windowMode);
	void SetWindowSize(UINT width, UINT height);

private:
	std::unordered_map<UINT,std::unique_ptr<IWindowEvent>> eventMap_;

	static inline const uint32_t sSwapChainBufferCount = 2;

	std::unique_ptr<Graphics::GraphicsSwapChain> swapChain_;
	std::array<std::unique_ptr<ColorBuffer>,sSwapChainBufferCount> colorBuffers_;
	std::unique_ptr<DepthBuffer> depthBuffer_;
	
	SizeChangeMode sizeChangeMode_ = SizeChangeMode::kNormal;
	WindowMode windowMode_ = WindowMode::kWindow;
	WindowSize size_;

	WindowCore core_;
	bool isDead_;
	bool isResize_;

	// ビューポート
	D3D12_VIEWPORT viewport_;
	// シザー矩形
	D3D12_RECT scissorRect_;

	UINT backBufferIndex_;

	void AdjustWindowSize();
	void CreatePixelBuffer();
	void DestroyPixelBuffer();
	void ResizeSignal();
	void Resize();
};
}

