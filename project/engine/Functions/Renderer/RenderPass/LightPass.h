#pragma once
#include "RenderPass.h"
#include "engine/Runtime/GpuResource/UploadBuffer.h"
#include "engine/Functions/ECS/Component/LightComponent.h"

namespace NoEngine {
namespace Render {
class LightPass : public RenderPass {
public:
	void Execute(GraphicsContext& gfx, ECS::Registry& registry) override;

	const UploadBuffer& GetDirectionalLightUpload() { return directionalLightUpload_; }
private:
	void Collect(ECS::Registry& registry);
	void UploadToGpu(GraphicsContext& gfx);

	UploadBuffer directionalLightUpload_;
	std::vector<Component::DirectionalLightComponent> directionalLights_;
	size_t directionalLightsSize_;
};
}
}

