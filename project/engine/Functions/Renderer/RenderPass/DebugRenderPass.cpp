#include "stdafx.h"
#include "DebugRenderPass.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Utilities/Conversion/ConvertString.h"

namespace NoEngine {
namespace Render {

using namespace Component;

void DebugRenderPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(resourceRegistry);
#ifdef USE_IMGUI
	gfx.ClearDepthAndStencil(GraphicsCore::GetDepth());
	gfx.SetRenderTarget(GraphicsCore::GetDebugRenderBuffer().GetRTV(), GraphicsCore::GetDepth().GetDSV());
	Collect(registry);
	Sort();
	Render(gfx);
	RenderOutline(gfx);
#else
	static_cast<void>(gfx);
	static_cast<void>(registry);
#endif // USE_IMGUI
}

void DebugRenderPass::Collect(ECS::Registry& registry) {
	auto view = registry.View<
		TransformComponent,
		MeshComponent,
		MaterialComponent
	>();
	items_.clear();

	auto cameraView = registry.View<TransformComponent, CameraComponent, DebugCameraComponent>();
	Math::Vector3 cameraPos{};
	for (auto entity : cameraView) {
		auto* cameraTransform = registry.GetComponent<TransformComponent>(entity);
		cameraPos = cameraTransform->GetWorldPosition();
		camera_ = registry.GetComponent<CameraComponent>(entity);
	}


	for (auto entity : view) {
		auto* mesh = registry.GetComponent<MeshComponent>(entity);
		if (!mesh->isVisible)continue;
		auto* material = registry.GetComponent<MaterialComponent>(entity);
		auto* transform = registry.GetComponent<TransformComponent>(entity);
		auto pso = material->psoId;
		auto rootSig = material->rootSigId;
		auto name = material->psoName;
		float distance = MathCalculations::LengthSquared(transform->translate - cameraPos);

		items_.push_back({ mesh,material,transform, pso, rootSig, ConvertString(name), distance });
	}
}

}

}