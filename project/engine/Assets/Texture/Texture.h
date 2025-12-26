#pragma once
#include "engine/Runtime/GpuResource/GpuResource.h"

namespace NoEngine {
class Texture : public GpuResource {
	friend class CommandContext;
public:
	Texture() { cpuDescriptorHandle_.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }
	Texture(D3D12_CPU_DESCRIPTOR_HANDLE Handle) : cpuDescriptorHandle_(Handle) {}

	void Create2D(size_t RowPitchBytes, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitData);
	// void CreateCube(size_t RowPitchBytes, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitialData);

	virtual void Destroy() override {
		GpuResource::Destroy();
		cpuDescriptorHandle_.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV() const { return cpuDescriptorHandle_; }

	uint32_t GetWidth() const { return width_; }
	uint32_t GetHeight() const { return height_; }
	uint32_t GetDepth() const { return depth_; }
protected:
	uint32_t width_;
	uint32_t height_;
	uint32_t depth_;

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle_;

};
}
