#pragma once
#include "engine/Utilities/NonCopyable.h"
#include "engine/Runtime/GpuResource/GpuBuffer.h"
#include "engine/Runtime/GpuResource/UploadBuffer.h"
#include "engine/Runtime/Command/GraphicsContext.h"

namespace NoEngine {
class RenderContext : NonCopyable {
public:
	struct LightNums {
		uint32_t directionalLightNum = 0;
		uint32_t pointLightNum = 0;
		uint32_t spotLightNum = 0;
	};

	void SetDirectionalLight(GraphicsContext& gfx, UploadBuffer& directionalLightUpload, uint32_t directionalLightNum);
	const LightNums* GetLightNums() { return &lightNums_; }

	D3D12_CPU_DESCRIPTOR_HANDLE GetDirectionalLightSRV() { return directionalLightBuffer_.GetSRV(); }
private:
	StructuredBuffer directionalLightBuffer_;
	LightNums lightNums_;
};
}

