#pragma once
namespace NoEngine {
namespace Graphics {
/// <summary>
/// スワップチェーンのラッパークラス
/// </summary>
class GraphicsSwapChain final {
public:
	GraphicsSwapChain(HWND hwnd, float windowWidth, float windowHeight);
	GraphicsSwapChain(HWND hwnd, uint32_t windowWidth, uint32_t windowHeight);
	~GraphicsSwapChain();

	IDXGISwapChain4* GetSwapChain() { return swapChain_.Get(); }

	void Resize(float windowWidth, float windowHeight);
	void Resize(UINT windowWidth, UINT windowHegiht);

	void Destroy();

private:
	void Initialize(HWND hwnd, UINT windowWidth, UINT windowHeight);

	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
};
}
}
