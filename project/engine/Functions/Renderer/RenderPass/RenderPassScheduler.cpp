#include "RenderPassScheduler.h"
#include "SpritePass.h"
#include "MeshPass.h"
#include "PrimitivePass.h"

namespace NoEngine {
namespace Render {
void RenderPassScheduler::Initialize() {
	passes_.push_back(std::make_unique<MeshPass>());
	passes_.push_back(std::make_unique<PrimitivePass>());
	passes_.push_back(std::make_unique<SpritePass>());
}

void RenderPassScheduler::Render(GraphicsContext& gfx, ECS::Registry& registry) {
	if (registry.Empty()) return;
	for (auto& pass : passes_) pass->Execute(gfx, registry);
}

void RenderPassScheduler::SetCamera(CameraBase* camera) {
	for (auto& pass : passes_) pass->SetCamera(camera);
}
void RenderPassScheduler::AddRenderPass(std::unique_ptr<RenderPass>&& pass)
{
	passes_.push_back(std::move(pass));
}
}
}
