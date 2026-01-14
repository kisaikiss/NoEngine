#pragma once
#include "RenderPass.h"
#include "engine/Runtime/Command/GraphicsContext.h"
namespace NoEngine {
namespace Render {
class RenderPassScheduler {
public:
	void Initialize();
	void Render(GraphicsContext& gfx, ECS::Registry& registry);
	void SetCamera(CameraBase* camera);
private:
	std::vector<std::unique_ptr<RenderPass>> passes_;
};
}
}

