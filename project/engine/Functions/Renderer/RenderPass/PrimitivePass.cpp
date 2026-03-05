#include "PrimitivePass.h"
#include "../Primitive.h"
#include "engine/Functions/ECS/Component/CameraComponent.h"

namespace NoEngine {
namespace Render {
PrimitivePass::PrimitivePass() {
	Primitive::Initialize();
}

void PrimitivePass::Execute(GraphicsContext& gfx, ECS::Registry& registry) {
	using namespace Component;
	auto cameraView = registry.View<CameraComponent, ActiveCameraTag>();
	CameraComponent* camera{};
	for (auto entity : cameraView) {
		camera = registry.GetComponent<CameraComponent>(entity);
	}
	Primitive::Render(gfx, camera->forGPU.viewProjection);
}
}
}