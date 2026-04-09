#include "LightPass.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"

namespace NoEngine {
namespace Render {
using namespace Component;

void LightPass::Execute(GraphicsContext& gfx, ECS::Registry& registry) {
	Collect(registry);
	UploadToGpu(gfx);
}

void LightPass::Collect(ECS::Registry& registry) {
	// 方向ライトの収集
	auto directionalView = registry.View<DirectionalLightComponent>();
	directionalLights_.clear();
	for (auto entity : directionalView) {
		auto* directionalLight = registry.GetComponent<DirectionalLightComponent>(entity);
		directionalLight->direction = directionalLight->direction.Normalize();
		directionalLights_.push_back(*directionalLight);
	}

	// ポイントライトの収集
	auto pointLightView = registry.View<PointLightComponent, TransformComponent>();
	for (auto entity : pointLightView) {
		// コンポーネントの取得
		auto* pointLightComponent = registry.GetComponent<PointLightComponent>(entity);
		auto* transformComponent = registry.GetComponent<TransformComponent>(entity);

		// 取得したコンポーネントからGPUへ送るための構造体の中身を構築する
		PointLightForGPU pointLight;
		pointLight.color = pointLightComponent->color;
		pointLight.intensity = pointLightComponent->intensity;
		pointLight.decay = pointLightComponent->decay;
		pointLight.radius = pointLightComponent->radius;
		pointLight.position = transformComponent->GetWorldPosition();
		pointLights_.push_back(pointLight);
	}
	
}

void LightPass::UploadToGpu(GraphicsContext& gfx) {
	RenderContext* renderContext = GetRenderContext();
	// 方向ライトをGPUへ送る
	if (directionalLights_.size() != directionalLightsSize_) {
		directionalLightsSize_ = directionalLights_.size();
		directionalLightUpload_.Create(L"DirectionalLight Upload", sizeof(DirectionalLightComponent) * directionalLightsSize_);
	}
	memcpy(directionalLightUpload_.Map(), directionalLights_.data(), sizeof(DirectionalLightComponent) * directionalLightsSize_);

	renderContext->SetDirectionalLight(gfx, directionalLightUpload_, static_cast<uint32_t>(directionalLightsSize_));

	// ポイントライトをGPUへ送る
	if (pointLights_.size() != 0) {
		if (pointLights_.size() != pointLightsSize_) {
			pointLightsSize_ = pointLights_.size();
			pointLightUpload_.Create(L"PointLight Upload", sizeof(PointLightForGPU) * pointLightsSize_);
		}
		memcpy(pointLightUpload_.Map(), pointLights_.data(), sizeof(PointLightForGPU) * pointLightsSize_);

		renderContext->SetPointLight(gfx, pointLightUpload_, static_cast<uint32_t>(pointLightsSize_));
	}
	
}

}
}
