#include "RenderPassScheduler.h"
#include "SpritePass.h"
#include "MeshPass.h"
namespace NoEngine {
namespace Render {
void RenderPassScheduler::Initialize() {
	passes_.push_back(std::make_unique<MeshPass>());
}

void RenderPassScheduler::Render(GraphicsContext& gfx, ECS::Registry& registry) {
	if (registry.Empty()) return;
	for (auto& pass : passes_) pass->Execute(gfx, registry);
}

void RenderPassScheduler::SetCamera(CameraBase* camera) {
	for (auto& pass : passes_) pass->SetCamera(camera);
}
}
}
