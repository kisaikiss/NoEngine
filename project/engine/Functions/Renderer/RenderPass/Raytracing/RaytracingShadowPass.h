#pragma once
#include "../RenderPass.h"
namespace NoEngine {
namespace Render {
class RaytracingShadowPass : public RenderPass {
public:
	void Execute(GraphicsContext& gfx, ECS::Registry& registry) override;
private:
	Component::CameraComponent* camera_ = nullptr;
	Math::Vector3 lightDir_;

	void Collect(ECS::Registry& registry);
	void Dispatch(GraphicsContext& gfx);
};
}
}

