#pragma once
namespace NoEngine {
namespace Graphics {

/// <summary>
/// DXGI関連を所有するクラス
/// </summary>
class GraphicsInfrastructures {
public:
	GraphicsInfrastructures();
	~GraphicsInfrastructures() = default;

	IDXGIFactory7* GetDXGIFactory() const { return dxgiFactory_.Get(); }
	IDXGIAdapter4* GetDXGIAdapter() const { return dxgiAdapter_.Get(); }

private:
	// DXGIFactory
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;
	// DXGIAdapter(使用するGPU)
	Microsoft::WRL::ComPtr<IDXGIAdapter4> dxgiAdapter_;
};

}
}