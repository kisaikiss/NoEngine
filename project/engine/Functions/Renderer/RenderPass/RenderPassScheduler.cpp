#include "RenderPassScheduler.h"
#include "Rasterization/SpritePass.h"
#include "Rasterization/MeshPass.h"
#include "Rasterization/TransparentMeshPass.h"
#include "Rasterization/PrimitivePass.h"
#include "PrePasses/LightPass.h"
#include "Rasterization/CPUParticlePass.h"
#include "Rasterization/ParticlePass.h"
#include "PrePasses/TLASBuildPass.h"
#include "PrePasses/BLASUpdatePass.h"
#include "PrePasses/PreRenderPass.h"
#include "Raytracing/RaytracingShadowPass.h"
#include "Rasterization/SkyBoxPass.h"
#include "PostEffect/GrayscalePass.h"
#include "PostEffect/VignettingPass.h"
#include "PostEffect/GaussianFilterPass.h"
#include "PostEffect/DepthOfFieldPass.h"
#include "PostEffect/DepthBasedOutlinePass.h"
#include "PostEffect/BloomThresholdPass.h"
#include "PostEffect/BloomBlurPass.h"
#include "PostEffect/BloomCompositePass.h"

#include "../Primitive.h"

#include "engine/Runtime/GraphicsCore.h"
#include "engine/Editor/EditUtils.h"
#include "engine/Functions/ECS/System/Editor/DrawManipulatorSystem.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/ImGuizmo.h"
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
		node.uavOutputs = builder.uavOutputs_;
		if (builder.hasDepthOutput_) {
			node.hasDepthOutput = true;
			node.depthOutput = builder.depthOutput_;
		}
	}

#ifdef USE_IMGUI
	idReadbackBuffer_.Create(L"ID_Readback_Buffer", 1, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	positionReadbackBuffer_.Create(L"WorldPosition_Readback_Buffer", 1, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
#endif // USE_IMGUI
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
			} else {
				DepthBuffer* depthBuffer = resourceRegistry_.GetDepthBufferPointer(inputName);
				if (depthBuffer) {
					gfx.TransitionResource(*depthBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
				}
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

		for (const auto& output : node.uavOutputs) {
			ColorBuffer* buffer = resourceRegistry_.GetColorBufferPointer(output);
			if (buffer) {
				gfx.TransitionResource(*buffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
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

	DebugPrimitive::Shutdown();
#ifdef USE_IMGUI
	gfx.TransitionResource(*resourceRegistry_.GetColorBufferPointer(screenDrawBufferName_), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	gfx.TransitionResource(*resourceRegistry_.GetColorBufferPointer("DebugColor"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	gfx.TransitionResource(*resourceRegistry_.GetColorBufferPointer("WorldPosition"), D3D12_RESOURCE_STATE_COPY_SOURCE);
	gfx.FlushResourceBarriers();
	DrawGameImGuiWindow();
	DrawSceneImGuiWindow(registry, gfx, *resourceRegistry_.GetColorBufferPointer("WorldPosition"), positionReadbackBuffer_);

	// クリックによるオブジェクトの選択
	static bool isUsing = false;
	static bool preIsUsing = false;
	
	// 左クリックのReleaseをオブジェクト選択に使用するため、前フレームの使用状況を参照
	preIsUsing = isUsing;
	isUsing = ImGuizmo::IsUsingAny();
	if (!preIsUsing && !ECS::DrawManipulatorSystem::TriggerManipulateButton()) {
		gfx.TransitionResource(*resourceRegistry_.GetColorBufferPointer("ObjectID"), D3D12_RESOURCE_STATE_COPY_SOURCE);
		gfx.TransitionResource(idReadbackBuffer_, D3D12_RESOURCE_STATE_COPY_DEST);
		gfx.FlushResourceBarriers();
		auto e = PickObject(gfx, *resourceRegistry_.GetColorBufferPointer("ObjectID"), idReadbackBuffer_);
		if (e != ECS::INVALID_ENTITY && registry.Has<Editor::EditTag>(e) && !registry.Has<Editor::NoClickSelectTag>(e)) {
			registry.AddComponent<Editor::EditSelectedTag>(e);
		}
	}

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

void RenderPassScheduler::Resize(uint32_t width, uint32_t height) {
	if (width == 0 || height == 0) return;
	// パスのトポロジー（何を読み書きするか）はサイズに依存しないため
	// Compile()のやり直しは不要。バッファの中身だけ作り直す。
	resourceRegistry_.ResizeAll(width, height);

#ifdef USE_IMGUI
	// ColorBufferの中身が新しいリソースに差し替わったので、
	// ImGuiが参照しているディスクリプタも最新のリソースに焼き直す
	RefreshGameImGuiTexture(*resourceRegistry_.GetColorBufferPointer("MainColor"));
	RefreshSceneImGuiTexture(*resourceRegistry_.GetColorBufferPointer("DebugColor"));
#endif
}

void CommonSetupRenderPass(RenderPassScheduler& renderPassScheduler) {
	Math::Vector2 windowSize = GraphicsCore::GetWindowSize();
	auto& resourceRegistry = renderPassScheduler.GetResourceRegistry();
	InitGameImGuiWindow(*resourceRegistry.CreateColorBuffer("MainColor", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB));
	

	resourceRegistry.CreateColorBuffer("ShadowMask", windowSize.x, windowSize.y, 64, DXGI_FORMAT_R8_UNORM);
	resourceRegistry.CreateColorBuffer("WorldPosition", windowSize.x, windowSize.y, DXGI_FORMAT_R32G32B32A32_FLOAT);
	resourceRegistry.CreateColorBuffer("Normal", windowSize.x, windowSize.y, DXGI_FORMAT_R10G10B10A2_UNORM);
	resourceRegistry.CreateColorBuffer("ObjectID", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM);

	// ブルーム
	resourceRegistry.CreateColorBuffer("BloomThreshold", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	resourceRegistry.CreateColorBuffer("BloomBlurA", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	resourceRegistry.CreateColorBuffer("BloomBlurB", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	resourceRegistry.CreateColorBuffer("BloomComposite", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

	resourceRegistry.CreateColorBuffer("PostEffect", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	resourceRegistry.CreateColorBuffer("DepthOfField", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	renderPassScheduler.SetScreenDrawBuffer("MainColor");
	
	resourceRegistry.CreateDepthBuffer("MainDepth", windowSize.x, windowSize.y);

	renderPassScheduler.AddPass(std::make_unique<BLASUpdatePass>());
	renderPassScheduler.AddPass(std::make_unique<TLASBuildPass>());
	renderPassScheduler.AddPass(std::make_unique<LightPass>());

	auto preRenderPass = std::make_unique<PreRenderPass>();
	preRenderPass->AddOutput("WorldPosition");
	preRenderPass->AddOutput("Normal");
	preRenderPass->AddOutput("ObjectID");
	preRenderPass->SetClearTarget(true);
	preRenderPass->SetDepthOutput("MainDepth", true);
	renderPassScheduler.AddPass(std::move(preRenderPass));

	auto raytracingShadowPass = std::make_unique<RaytracingShadowPass>();
	raytracingShadowPass->AddInput("WorldPosition", "WorldPosition");
	raytracingShadowPass->AddInput("Normal", "Normal");
	raytracingShadowPass->AddUAVOutput("ShadowMask");
	raytracingShadowPass->SetClearTarget(true);

	renderPassScheduler.AddPass(std::move(raytracingShadowPass));

	auto meshPass = std::make_unique<MeshPass>();
	meshPass->AddInput("ShadowMask", "ShadowMask");
	meshPass->AddOutput("MainColor");
	meshPass->SetDepthOutput("MainDepth", true);
	meshPass->SetClearTarget(true);
	renderPassScheduler.AddPass(std::move(meshPass));

	auto primitivePass = std::make_unique<PrimitivePass>();
	primitivePass->AddOutput("MainColor");
	primitivePass->SetDepthOutput("MainDepth");
	renderPassScheduler.AddPass(std::move(primitivePass));


	auto depthBasedOutlinePass = std::make_unique<DepthBasedOutlinePass>();
	depthBasedOutlinePass->AddInput("InputColor", "MainColor");
	depthBasedOutlinePass->AddInput("InputDepth", "MainDepth");
	depthBasedOutlinePass->AddOutput("PostEffect");
	renderPassScheduler.AddPass(std::move(depthBasedOutlinePass));

	auto depthOfField = std::make_unique<DepthOfFieldPass>();
	depthOfField->AddInput("InputColor", "PostEffect");
	depthOfField->AddInput("InputDepth", "MainDepth");
	depthOfField->AddOutput("MainColor");
	renderPassScheduler.AddPass(std::move(depthOfField));

	auto transparentMeshPass = std::make_unique<TransparentMeshPass>();
	transparentMeshPass->AddInput("ShadowMask", "ShadowMask");
	transparentMeshPass->AddOutput("MainColor");
	transparentMeshPass->SetDepthOutput("MainDepth");
	renderPassScheduler.AddPass(std::move(transparentMeshPass));

	auto cpuParticlePass = std::make_unique<CPUParticlePass>();
	cpuParticlePass->AddOutput("MainColor");
	cpuParticlePass->SetDepthOutput("MainDepth");
	renderPassScheduler.AddPass(std::move(cpuParticlePass));

	auto particlePass = std::make_unique<ParticlePass>();
	particlePass->AddOutput("MainColor");
	particlePass->SetDepthOutput("MainDepth");
	renderPassScheduler.AddPass(std::move(particlePass));

	auto bloomThreshold = std::make_unique<BloomThresholdPass>();
	bloomThreshold->AddInput("InputColor", "MainColor");
	bloomThreshold->AddOutput("BloomThreshold");
	renderPassScheduler.AddPass(std::move(bloomThreshold));

	{
		auto bloomBlurH = std::make_unique<BloomBlurPass>(true);
		bloomBlurH->AddInput("InputColor", "BloomThreshold");
		bloomBlurH->AddOutput("BloomBlurA");
		renderPassScheduler.AddPass(std::move(bloomBlurH));

		auto bloomBlurV = std::make_unique<BloomBlurPass>(false);
		bloomBlurV->AddInput("InputColor", "BloomBlurA");
		bloomBlurV->AddOutput("BloomBlurB");
		renderPassScheduler.AddPass(std::move(bloomBlurV));
	}

	auto bloomComposite = std::make_unique<BloomCompositePass>();
	bloomComposite->AddInput("SceneColor", "MainColor");
	bloomComposite->AddInput("BloomColor", "BloomBlurB");
	bloomComposite->AddOutput("BloomComposite");
	renderPassScheduler.AddPass(std::move(bloomComposite));

	auto vignetting = std::make_unique<VignettingPass>();
	vignetting->AddInput("InputColor", "BloomComposite");
	vignetting->AddOutput("MainColor");
	renderPassScheduler.AddPass(std::move(vignetting));


	auto spritePass = std::make_unique<SpritePass>();
	spritePass->AddOutput("MainColor");
	spritePass->SetTargetCameraType(RenderPass::TargetCameraType::kMain);
	renderPassScheduler.AddPass(std::move(spritePass));
}

void CommonSetupDebugRenderPass(RenderPassScheduler& renderPassScheduler) {
#ifdef USE_IMGUI

	Math::Vector2 windowSize = GraphicsCore::GetWindowSize();
	auto& resourceRegistry = renderPassScheduler.GetResourceRegistry();
	InitSceneImGuiWindow(*resourceRegistry.CreateColorBuffer("DebugColor", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB));

	auto preRenderPass = std::make_unique<PreRenderPass>();
	preRenderPass->AddOutput("WorldPosition");
	preRenderPass->AddOutput("Normal");
	preRenderPass->AddOutput("ObjectID");
	preRenderPass->SetClearTarget(true);
	preRenderPass->SetDepthOutput("MainDepth", true);
	preRenderPass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(preRenderPass));


	auto raytracingShadowPass = std::make_unique<RaytracingShadowPass>();
	raytracingShadowPass->AddInput("WorldPosition", "WorldPosition");
	raytracingShadowPass->AddInput("Normal", "Normal");
	raytracingShadowPass->AddUAVOutput("ShadowMask");
	raytracingShadowPass->SetClearTarget(true);
	raytracingShadowPass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(raytracingShadowPass));

	auto meshPass = std::make_unique<MeshPass>();
	meshPass->AddInput("ShadowMask", "ShadowMask");
	meshPass->AddOutput("DebugColor");
	meshPass->SetDepthOutput("MainDepth", true);
	meshPass->SetClearTarget(true);
	meshPass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(meshPass));

	auto transparentMeshPass = std::make_unique<TransparentMeshPass>();
	transparentMeshPass->AddInput("ShadowMask", "ShadowMask");
	transparentMeshPass->AddOutput("DebugColor");
	transparentMeshPass->SetDepthOutput("MainDepth");
	transparentMeshPass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(transparentMeshPass));

	auto primitivePass = std::make_unique<PrimitivePass>();
	primitivePass->AddOutput("DebugColor");
	primitivePass->SetDepthOutput("MainDepth");
	primitivePass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(primitivePass));

	auto cpuParticlePass = std::make_unique<CPUParticlePass>();
	cpuParticlePass->AddOutput("DebugColor");
	cpuParticlePass->SetDepthOutput("MainDepth");
	cpuParticlePass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(cpuParticlePass));

	auto particlePass = std::make_unique<ParticlePass>();
	particlePass->AddOutput("DebugColor");
	particlePass->SetDepthOutput("MainDepth");
	particlePass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(particlePass));

	auto spritePass = std::make_unique<SpritePass>();
	spritePass->AddOutput("DebugColor");
	spritePass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(spritePass));
#else
	static_cast<void>(renderPassScheduler);
#endif // USE_IMGUI

}

}
