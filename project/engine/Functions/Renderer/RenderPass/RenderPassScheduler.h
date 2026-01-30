#pragma once
#include "RenderPass.h"
#include "engine/Runtime/Command/GraphicsContext.h"
#include "../RenderContext.h"
namespace NoEngine {
namespace Render {
class RenderPassScheduler {
public:
	void Initialize();
	void Render(GraphicsContext& gfx, ECS::Registry& registry);
	void SetRenderContext(RenderContext& renderContext);
	void AddRenderPass(std::unique_ptr<RenderPass>&& pass);

private:
	std::vector<std::unique_ptr<RenderPass>> passes_;
};
}
}

