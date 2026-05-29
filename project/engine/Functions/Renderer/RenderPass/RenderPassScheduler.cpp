#include "RenderPassScheduler.h"
#include "Rasterization/SpritePass.h"
#include "Rasterization/MeshPass.h"
#include "Rasterization/PrimitivePass.h"
#include "PrePasses/LightPass.h"
#include "Rasterization/ParticlePass.h"
#include "PrePasses/TLASBuildPass.h"
#include "PrePasses/PreRenderPass.h"
#include "Raytracing/RaytracingShadowPass.h"
#include "Rasterization/SkyBoxPass.h"
#include "PostEffect/GrayscalePass.h"
#include "PostEffect/VignettingPass.h"
#include "PostEffect/BoxFilterPass.h"

#include "engine/Runtime/GraphicsCore.h"
#include "engine/Editor/EditUtils.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI


namespace NoEngine {
using namespace Render;

namespace {
#ifdef USE_IMGUI
ImTextureID sDebugTexture;
#endif // USE_IMGUI
}

void RenderPassScheduler::AddPass(std::unique_ptr<RenderPass> pass) {
	RenderPassNode node;
	node.pass = std::move(pass);
	nodes_.push_back(std::move(node));
}

void RenderPassScheduler::Compile() {
	for (auto& node : nodes_) {
		RenderGraphBuilder builder;
		node.pass->Setup(builder);
		node.pass->SetRenderContext(&renderContext_);
		node.inputs = builder.inputs_;
		node.outputs = builder.outputs_;
		if (builder.hasDepthOutput_) {
			node.hasDepthOutput = true;
			node.depthOutput = builder.depthOutput_;
		}
	}
}


void RenderPassScheduler::Render(GraphicsContext& gfx, ECS::Registry& registry) {
	if (registry.Empty() || nodes_.empty()) return;
	renderContext_.Update(registry);
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

		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};
		bool useDepth = false;

		if (node.hasDepthOutput) { // このパスがDepthBufferを使う場合
			DepthBuffer* depthBuffer = resourceRegistry_.GetDepthBufferPointer(node.depthOutput.name);
			if (depthBuffer) {
				// DirectX12のバリア：書き込み状態(DEPTH_WRITE)へ自動遷移
				gfx.TransitionResource(*depthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
				dsvHandle = depthBuffer->GetDSV();
				useDepth = true;

				// 自動クリアの実行
				if (node.depthOutput.autoClear) {
					// MiniEngineのClearDepth関数を呼び出す
					gfx.ClearDepthAndStencil(*depthBuffer);
				}
			}
		}

		if (!renderTargetViews.empty() || useDepth) {
			if(useDepth){
				gfx.SetRenderTargets(static_cast<UINT>(renderTargetViews.size()), renderTargetViews.data(), dsvHandle);
			} else {
				gfx.SetRenderTargets(static_cast<UINT>(renderTargetViews.size()), renderTargetViews.data());
			}
			
		} 
		node.pass->Execute(gfx, resourceRegistry_, registry);
	}

#ifdef USE_IMGUI
	gfx.TransitionResource(*resourceRegistry_.GetColorBufferPointer(screenDrawBufferName_), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	gfx.TransitionResource(*resourceRegistry_.GetColorBufferPointer("DebugColor"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	DrawGameImGuiWindow();
	DrawSceneImGuiWindow();
#endif // USE_IMGUI


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
	auto& resourceRegistry = renderPassScheduler.GetResourceRegistry();
	InitGameImGuiWindow(*resourceRegistry.CreateColorBuffer("MainColor", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB));
	

	resourceRegistry.CreateColorBuffer("ShadowMap", windowSize.x, windowSize.y, DXGI_FORMAT_R8_UNORM);
	resourceRegistry.CreateColorBuffer("WorldPosition", windowSize.x, windowSize.y, DXGI_FORMAT_R32G32B32A32_FLOAT);
	resourceRegistry.CreateColorBuffer("Normal", windowSize.x, windowSize.y, DXGI_FORMAT_R10G10B10A2_UNORM);

	resourceRegistry.CreateColorBuffer("PostEffect", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	renderPassScheduler.SetScreenDrawBuffer("MainColor");
	
	resourceRegistry.CreateDepthBuffer("MainDepth", windowSize.x, windowSize.y);


	renderPassScheduler.AddPass(std::make_unique<TLASBuildPass>());
	renderPassScheduler.AddPass(std::make_unique<LightPass>());

	auto preRenderPass = std::make_unique<PreRenderPass>();
	preRenderPass->AddOutput("WorldPosition");
	preRenderPass->AddOutput("Normal");
	preRenderPass->SetClearTarget(true);
	preRenderPass->SetDepthOutput("MainDepth", true);
	renderPassScheduler.AddPass(std::move(preRenderPass));

	auto raytracingShadowPass = std::make_unique<RaytracingShadowPass>();
	raytracingShadowPass->AddInput("WorldPosition", "WorldPosition");
	raytracingShadowPass->AddInput("Normal", "Normal");
	raytracingShadowPass->AddOutput("ShadowMap");
	raytracingShadowPass->SetClearTarget(true);

	renderPassScheduler.AddPass(std::move(raytracingShadowPass));

	auto meshPass = std::make_unique<MeshPass>();
	meshPass->AddInput("ShadowMap", "ShadowMap");
	meshPass->AddOutput("MainColor");
	meshPass->SetDepthOutput("MainDepth");
	meshPass->SetClearTarget(true);
	renderPassScheduler.AddPass(std::move(meshPass));

	auto spritePass = std::make_unique<SpritePass>();
	spritePass->AddOutput("MainColor");
	renderPassScheduler.AddPass(std::move(spritePass));

	auto primitivePass = std::make_unique<PrimitivePass>();
	primitivePass->AddOutput("MainColor");
	primitivePass->SetDepthOutput("MainDepth");
	renderPassScheduler.AddPass(std::move(primitivePass));

	auto skyBoxPass = std::make_unique<SkyBoxPass>();
	skyBoxPass->AddOutput("MainColor");
	skyBoxPass->SetDepthOutput("MainDepth");
	renderPassScheduler.AddPass(std::move(skyBoxPass));

	auto particlePass = std::make_unique<ParticlePass>();
	particlePass->AddOutput("MainColor");
	particlePass->SetDepthOutput("MainDepth");
	renderPassScheduler.AddPass(std::move(particlePass));

}

void CommonSetupDebugRenderPass(RenderPassScheduler& renderPassScheduler) {
#ifdef USE_IMGUI

	Math::Vector2 windowSize = GraphicsCore::GetWindowSize();
	auto& resourceRegistry = renderPassScheduler.GetResourceRegistry();
	InitSceneImGuiWindow(*resourceRegistry.CreateColorBuffer("DebugColor", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB));

	auto preRenderPass = std::make_unique<PreRenderPass>();
	preRenderPass->AddOutput("WorldPosition");
	preRenderPass->AddOutput("Normal");
	preRenderPass->SetClearTarget(true);
	preRenderPass->SetDepthOutput("MainDepth", true);
	preRenderPass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(preRenderPass));

	auto raytracingShadowPass = std::make_unique<RaytracingShadowPass>();
	raytracingShadowPass->AddInput("WorldPosition", "WorldPosition");
	raytracingShadowPass->AddInput("Normal", "Normal");
	raytracingShadowPass->AddOutput("ShadowMap");
	raytracingShadowPass->SetClearTarget(true);
	raytracingShadowPass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(raytracingShadowPass));

	auto meshPass = std::make_unique<MeshPass>();
	meshPass->AddInput("ShadowMap", "ShadowMap");
	meshPass->AddOutput("DebugColor");
	meshPass->SetDepthOutput("MainDepth");
	meshPass->SetClearTarget(true);
	meshPass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(meshPass));

	auto skyBoxPass = std::make_unique<SkyBoxPass>();
	skyBoxPass->AddOutput("DebugColor");
	skyBoxPass->SetDepthOutput("MainDepth");
	skyBoxPass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(skyBoxPass));

	auto particlePass = std::make_unique<ParticlePass>();
	particlePass->AddOutput("DebugColor");
	particlePass->SetDepthOutput("MainDepth");
	particlePass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(particlePass));
#else
	static_cast<void>(renderPassScheduler);
#endif // USE_IMGUI

}

}
