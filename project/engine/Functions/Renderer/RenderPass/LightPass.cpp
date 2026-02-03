#include "LightPass.h"

namespace NoEngine {
namespace Render {
using namespace Component;

void LightPass::Execute(GraphicsContext& gfx, ECS::Registry& registry) {
	Collect(registry);
	UploadToGpu(gfx);
}

void LightPass::Collect(ECS::Registry& registry) {
	auto view = registry.View<DirectionalLightComponent>();

	if (view.Empty())return;

	directionalLights_.clear();
	for (auto entity : view) {

		directionalLights_.push_back(*registry.GetComponent<DirectionalLightComponent>(entity));
	}
	
}

void LightPass::UploadToGpu(GraphicsContext& gfx) {
	if (directionalLights_.size() != directionalLightsSize_) {
		directionalLightsSize_ = directionalLights_.size();
		directionalLightUpload_.Create(L"DirectionalLight Upload", sizeof(DirectionalLightComponent) * directionalLightsSize_);
	}
	memcpy(directionalLightUpload_.Map(), directionalLights_.data(), sizeof(DirectionalLightComponent) * directionalLightsSize_);

	GetRenderContext()->SetDirectionalLight(gfx, directionalLightUpload_, static_cast<uint32_t>(directionalLightsSize_));
}

}
}
