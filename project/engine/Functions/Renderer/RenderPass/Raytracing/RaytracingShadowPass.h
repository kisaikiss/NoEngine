#pragma once
#include "../RenderPass.h"
namespace NoEngine {
namespace Render {
class RaytracingShadowPass : public RenderPass {
public:
	void Execute(GraphicsContext& gfx, ECS::Registry& registry) override;
private:
	Component::CameraComponent* camera_ = nullptr;

	void Collect(ECS::Registry& registry);
	void Dispatch(GraphicsContext& gfx);
};
}
}

