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
#include "PostEffect/GrayscalePass.h"

#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
using namespace Render;

void RenderPassScheduler::AddPass(std::unique_ptr<RenderPass> pass) {
	RenderPassNode node;
	node.pass = std::move(pass);
	nodes_.push_back(std::move(node));
}

void RenderPassScheduler::Compile() {
	for (auto& node : nodes_) {
		RenderGraphBuilder builder;
		node.pass->Setup(builder);
		node.inputs = builder.inputs_;
		node.outputs = builder.outputs_;
	}
}

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
	passes_.push_back(std::make_unique<GrayscalePass>());

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

void CommonSetupRenderPass(RenderPassScheduler& renderPassScheduler, RenderResourceManager& renderResources) {
	Math::Vector2 windowSize = GraphicsCore::GetWindowSize();
	renderResources.Create("MainColor", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	renderResources.Create("ShadowMap", windowSize.x, windowSize.y, DXGI_FORMAT_R8_UNORM);
	renderResources.Create("WorldPosition", windowSize.x, windowSize.y, DXGI_FORMAT_R32G32B32A32_FLOAT);
	renderResources.Create("Normal", windowSize.x, windowSize.y, DXGI_FORMAT_R10G10B10A2_UNORM);
	(void)renderPassScheduler;
}

}
