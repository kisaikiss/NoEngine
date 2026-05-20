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
#include "PostEffect/VignettingPass.h"
#include "PostEffect/BoxFilterPass.h"

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
	passes_.push_back(std::make_unique<VignettingPass>());

	passes_.push_back(std::make_unique<DebugRenderPass>());

	for (auto& pass : passes_) pass->SetRenderContext(&renderContext_);
}

void RenderPassScheduler::Render(GraphicsContext& gfx, ECS::Registry& registry) {
	if (registry.Empty() || nodes_.empty()) return;

	for (auto& node : nodes_) {
		// 入力リソース（SRVとして読むもの）を PIXEL_SHADER_RESOURCE ステートへ
		for (const auto& inputName : node.inputs) {
			ColorBuffer* buffer = resourceRegistry_.GetColorBufferPointer(inputName);
			if (buffer) {
				gfx.TransitionResource(*buffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			}
		}

		// 出力リソース（RTVとして書くもの）を RENDER_TARGET ステートへ
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetViews;
		for (const auto& output : node.outputs) {
			ColorBuffer* buffer = resourceRegistry_.GetColorBufferPointer(output.name);
			if (buffer) {
				gfx.TransitionResource(*buffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
				renderTargetViews.push_back(buffer->GetRTV()); // RTVのディスクリプタを収集
				if (output.autoClear) {
					gfx.ClearColor(*buffer);
				}
			}
		}

		if (!renderTargetViews.empty()) gfx.SetRenderTargets(static_cast<UINT>(renderTargetViews.size()), renderTargetViews.data());
		node.pass->Execute(gfx, resourceRegistry_, registry);
	}

}

void RenderPassScheduler::RenderAll(GraphicsContext& gfx, ECS::Registry& registry) {
	if (registry.Empty()) return;
	renderContext_.Update(registry);
	for (auto& pass : passes_) pass->Execute(gfx, resourceRegistry_, registry);
}

void RenderPassScheduler::SetRenderContext(RenderContext& renderContext) {
	for (auto& pass : passes_) pass->SetRenderContext(&renderContext);
}

void RenderPassScheduler::AddRenderPass(std::unique_ptr<RenderPass>&& pass) {
	passes_.pop_back();
	passes_.push_back(std::move(pass));
	passes_.push_back(std::make_unique<SpritePass>());
}

void CommonSetupRenderPass(RenderPassScheduler& renderPassScheduler) {
	Math::Vector2 windowSize = GraphicsCore::GetWindowSize();
	auto resourceRegistry = renderPassScheduler.GetResourceRegistry();
	resourceRegistry.Create("MainColor", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	resourceRegistry.Create("ShadowMap", windowSize.x, windowSize.y, DXGI_FORMAT_R8_UNORM);
	resourceRegistry.Create("WorldPosition", windowSize.x, windowSize.y, DXGI_FORMAT_R32G32B32A32_FLOAT);
	resourceRegistry.Create("Normal", windowSize.x, windowSize.y, DXGI_FORMAT_R10G10B10A2_UNORM);

	resourceRegistry.Create("PostEffect1", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	resourceRegistry.Create("PostEffect2", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);


	renderPassScheduler.AddPass(std::make_unique<TLASBuildPass>());
	renderPassScheduler.AddPass(std::make_unique<LightPass>());

	auto preRenderPass = std::make_unique<PreRenderPass>();
	preRenderPass->AddOutput("WorldPosition");
	preRenderPass->AddOutput("Normal");
	renderPassScheduler.AddPass(std::move(preRenderPass));

	auto raytracingShadowPass = std::make_unique<RaytracingShadowPass>();
	raytracingShadowPass->AddInput("WorldPosition");
	raytracingShadowPass->AddInput("Normal");
	raytracingShadowPass->AddOutput("ShadowMap");
	raytracingShadowPass->SetClearTarget(true);
	renderPassScheduler.AddPass(std::move(raytracingShadowPass));

	auto meshPass = std::make_unique<MeshPass>();
	meshPass->AddInput("ShadowMap");
	meshPass->AddOutput("MainColor");
	meshPass->SetClearTarget(true);
	renderPassScheduler.AddPass(std::move(meshPass));

	auto primitivePass = std::make_unique<PrimitivePass>();
	primitivePass->AddOutput("MainColor");
	renderPassScheduler.AddPass(std::move(primitivePass));

	auto particlePass = std::make_unique<ParticlePass>();
	particlePass->AddOutput("MainColor");
	renderPassScheduler.AddPass(std::move(particlePass));

	auto skyBoxPass = std::make_unique<SkyBoxPass>();
	skyBoxPass->AddOutput("MainColor");
	renderPassScheduler.AddPass(std::move(skyBoxPass));

}

void CommonSetupDebugRenderPass(RenderPassScheduler& renderPassScheduler) {
	Math::Vector2 windowSize = GraphicsCore::GetWindowSize();
	auto resourceRegistry = renderPassScheduler.GetResourceRegistry();
	resourceRegistry.Create("DebugColor", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

	auto preRenderPass = std::make_unique<PreRenderPass>();
	preRenderPass->AddOutput("WorldPosition");
	preRenderPass->AddOutput("Normal");
	renderPassScheduler.AddPass(std::move(preRenderPass));

	auto raytracingShadowPass = std::make_unique<RaytracingShadowPass>();
	raytracingShadowPass->AddInput("WorldPosition");
	raytracingShadowPass->AddInput("Normal");
	raytracingShadowPass->AddOutput("ShadowMap");
	raytracingShadowPass->SetClearTarget(true);
	renderPassScheduler.AddPass(std::move(raytracingShadowPass));

	auto meshPass = std::make_unique<MeshPass>();
	meshPass->AddInput("ShadowMap");
	meshPass->AddOutput("DebugColor");
	meshPass->SetClearTarget(true);
	renderPassScheduler.AddPass(std::move(meshPass));

	auto primitivePass = std::make_unique<PrimitivePass>();
	primitivePass->AddOutput("DebugColor");
	renderPassScheduler.AddPass(std::move(primitivePass));

	auto particlePass = std::make_unique<ParticlePass>();
	particlePass->AddOutput("DebugColor");
	renderPassScheduler.AddPass(std::move(particlePass));

	auto skyBoxPass = std::make_unique<SkyBoxPass>();
	skyBoxPass->AddOutput("DebugColor");
	renderPassScheduler.AddPass(std::move(skyBoxPass));

}

}
