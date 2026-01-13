#include "RenderPassScheduler.h"
#include "SpritePass.h"
namespace NoEngine {
namespace Render {
void RenderPassScheduler::Initialize() {
	passes_.push_back(std::make_unique<SpritePass>());
}

void RenderPassScheduler::Render(GraphicsContext& gfx, ECS::Registry& registry) {
	for (auto& pass : passes_) pass->Execute(gfx, registry);
}
}
}
