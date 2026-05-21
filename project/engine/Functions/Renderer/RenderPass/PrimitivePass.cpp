#include "PrimitivePass.h"
#include "../Primitive.h"
#include "engine/Math/Types/Calculations/Matrix4x4Calculations.h"
#include "engine/Functions/ECS/Component/CameraComponent.h"
#include "engine/Runtime/GraphicsCore.h"

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
	auto size = GraphicsCore::sWindowManager.GetMainWindow()->GetWindowSize();
	Math::Matrix4x4 viewProj = MathCalculations::MakeOrthographicMatrix(0.f, 0.f, static_cast<float>(size.clientWidth), static_cast<float>(size.clientHeight), 0.1f, 100.f);
	for (auto entity : camera2DView) {
		viewProj = registry.GetComponent<Camera2DComponent>(entity)->viewProjection;
	}
	DebugPrimitive::Render2D(gfx, viewProj);
}

}
}