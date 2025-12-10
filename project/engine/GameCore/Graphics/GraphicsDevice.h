#pragma once
namespace NoEngine {
namespace Graphics {
/// <summary>
/// ID3D12Deviceを所有するクラス
/// </summary>
class GraphicsDevice {
public:
	GraphicsDevice(IDXGIAdapter4* adapter);
	~GraphicsDevice();

	ID3D12Device5* GetDevice() const { return device_.Get(); }
private:
	Microsoft::WRL::ComPtr<ID3D12Device5> device_;
};
}
}