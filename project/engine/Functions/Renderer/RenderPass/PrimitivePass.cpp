#include "PrimitivePass.h"
#include "../Primitive.h"
#include "engine/Functions/ECS/Component/CameraComponent.h"

namespace NoEngine {
namespace Render {
PrimitivePass::PrimitivePass() {
	
}

void PrimitivePass::Execute(GraphicsContext& gfx, const  RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(resourceRegistry);
	DebugPrimitive::Initialize(*GetRenderContext());
	Draw(gfx);
	Draw2D(gfx, registry);
}

void PrimitivePass::Draw(GraphicsContext& gfx) {
	using namespace Component;
	CameraComponent* camera{};
	camera = GetRenderContext()->GetCamera();
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