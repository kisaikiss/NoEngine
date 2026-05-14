#include "PrimitivePass.h"
#include "../Primitive.h"
#include "engine/Functions/ECS/Component/CameraComponent.h"

namespace NoEngine {
namespace Render {
PrimitivePass::PrimitivePass() {
	DebugPrimitive::Initialize();
}

void PrimitivePass::Execute(GraphicsContext& gfx, ECS::Registry& registry) {
	Draw(gfx, registry);
	Draw2D(gfx, registry);
}

void PrimitivePass::Draw(GraphicsContext& gfx, ECS::Registry& registry) {
	using namespace Component;
	auto cameraView = registry.View<CameraComponent, ActiveCameraTag>();
	CameraComponent* camera{};
	for (auto entity : cameraView) {
		camera = registry.GetComponent<CameraComponent>(entity);
	}
	if (!camera) return;
	DebugPrimitive::Render(gfx, camera->forGPU.viewProjection);
}

void PrimitivePass::Draw2D(GraphicsContext& gfx, ECS::Registry& registry) {
	using namespace Component;
	auto camera2DView = registry.View<Camera2DComponent, ActiveCamera2DTag>();
	Camera2DComponent* camera2D{};
	for (auto entity : camera2DView) {
		camera2D = registry.GetComponent<Camera2DComponent>(entity);
	}
	if (!camera2D) return;
	DebugPrimitive::Render2D(gfx, camera2D->viewProjection);
}

}
}