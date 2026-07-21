#include "PrimitivePass.h"
#include "../../Primitive.h"
#include "engine/Math/Types/Calculations/Matrix4x4Calculations.h"
#include "engine/Functions/ECS/Component/Common/CameraComponent.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
namespace Render {
PrimitivePass::PrimitivePass() {
	
}

void PrimitivePass::Execute(GraphicsContext& gfx, const  RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(resourceRegistry);
	static_cast<void>(registry);
	DebugPrimitive::Initialize(*GetRenderContext());
	Draw(gfx);
	Draw2D(gfx);
}

void PrimitivePass::Draw(GraphicsContext& gfx) {
	using namespace Component;
	CameraComponent* camera{};
	camera = GetTargetCamera();
	if (!camera) return;
	DebugPrimitive::Render(gfx, camera->forGPU.viewProjection);
}

void PrimitivePass::Draw2D(GraphicsContext& gfx) {
	using namespace Component;
	auto size = GraphicsCore::sWindowManager.GetMainWindow()->GetWindowSize();
	Math::Matrix4x4 viewProj = MathCalculations::MakeOrthographicMatrix(0.f, 0.f, static_cast<float>(size.clientWidth), static_cast<float>(size.clientHeight), 0.1f, 100.f);
	auto* camera = GetTargetCamera2D();
	if (camera) {
		viewProj = camera->viewProjection;
	}
	DebugPrimitive::Render2D(gfx, viewProj);
}

}
}