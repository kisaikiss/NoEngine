#include "RenderPassScheduler.h"
#include "SpritePass.h"
#include "MeshPass.h"
#include "PrimitivePass.h"
#include "PrePasses/LightPass.h"
#include "ParticlePass.h"
#include "PrePasses/TLASBuildPass.h"
#include "PrePasses/PreRenderPass.h"
#include "Raytracing/RaytracingShadowPass.h"
#include "SkyBoxPass.h"
#include "DebugRenderPass.h"

namespace NoEngine {
namespace Render {
void RenderPassScheduler::Initialize() {
	passes_.push_back(std::make_unique<TLASBuildPass>());
	passes_.push_back(std::make_unique<LightPass>());
	passes_.push_back(std::make_unique<PreRenderPass>());
	passes_.push_back(std::make_unique<RaytracingShadowPass>());
	passes_.push_back(std::make_unique<MeshPass>());
	passes_.push_back(std::make_unique<PrimitivePass>());
	passes_.push_back(std::make_unique<ParticlePass>());
	passes_.push_back(std::make_unique<SpritePass>());
	passes_.push_back(std::make_unique<SkyBoxPass>());

	passes_.push_back(std::make_unique<DebugRenderPass>());

	for (auto& pass : passes_) pass->SetRenderContext(&renderContext_);
}

void RenderPassScheduler::Render(GraphicsContext& gfx, ECS::Registry& registry) {
	if (registry.Empty()) return;
	renderContext_.Update(registry);
	for (auto& pass : passes_) pass->Execute(gfx, registry);
}

void RenderPassScheduler::SetRenderContext(RenderContext& renderContext) {
	for (auto& pass : passes_) pass->SetRenderContext(&renderContext);
}
void RenderPassScheduler::AddRenderPass(std::unique_ptr<RenderPass>&& pass)
{
	passes_.pop_back();
	passes_.push_back(std::move(pass));
	passes_.push_back(std::make_unique<SpritePass>());
}
}
}
