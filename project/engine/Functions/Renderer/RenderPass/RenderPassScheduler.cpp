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
#include "PostEffect/BoxFilterPass.h"
#include "PostEffect/DissolvePass.h"
#include "PostEffect/RadialBlurPass.h"
#include "PostEffect/RandomNoisePass.h"
#include "PostEffect/VignettingPass.h"
#include "PostEffect/GaussianFilterPass.h"
#include "PostEffect/DepthOfFieldPass.h"
#include "PostEffect/DepthBasedOutlinePass.h"
#include "PostEffect/BloomThresholdPass.h"
#include "PostEffect/BloomBlurPass.h"
#include "PostEffect/BloomCompositePass.h"
#include "Rasterization/TextPass.h"
#include "Rasterization/SpriteIDPass.h"

#include "../Primitive.h"

#include "engine/Runtime/GraphicsCore.h"
#include "engine/Editor/Utilities/EditUtils.h"
#include "engine/Functions/ECS/System/Editor/DrawManipulatorSystem.h"
#include "engine/Functions/Scene/IScene.h"

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

void RenderPassScheduler::AddPass(std::unique_ptr<RenderPass> pass, const std::string& editorName, bool enabledByDefault, bool editorVisible) {
	RenderPassNode node;
	node.pass = std::move(pass);
	node.editorName = editorName.empty() ? "RenderPass" + std::to_string(nodes_.size()) : editorName;
	node.enabledByDefault = enabledByDefault;
	node.editorVisible = editorVisible;
	nodes_.push_back(std::move(node));
}

void RenderPassScheduler::Compile() {
	for (auto& node : nodes_) {
		RefreshPassNode(node);
		if (!defaultPassSettings_.contains(node.editorName)) defaultPassSettings_[node.editorName] = node.pass->SaveEditorSettings();
	}

#ifdef USE_IMGUI
	idReadbackBuffer_.Create(L"ID_Readback_Buffer", 1, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	positionReadbackBuffer_.Create(L"WorldPosition_Readback_Buffer", 1, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
#endif // USE_IMGUI
}

void RenderPassScheduler::RefreshPassNode(RenderPassNode& node) {
	RenderGraphBuilder builder;
	node.pass->Setup(builder);
	node.pass->SetRenderContext(&renderContext_);
	node.inputs = builder.inputs_;
	node.outputs = builder.outputs_;
	node.uavOutputs = builder.uavOutputs_;
	node.hasDepthOutput = builder.hasDepthOutput_;
	node.depthOutput = builder.depthOutput_;
}


void RenderPassScheduler::Render(GraphicsContext& gfx, ECS::Registry& registry) {
	if (registry.Empty() || nodes_.empty()) return;
	LoadPassConfiguration(registry);
	renderContext_.Update(registry);
	std::vector<std::string> renderPassNames = activePassNames_;
	for (const auto& node : nodes_) {
		if (!node.editorVisible) renderPassNames.push_back(node.editorName);
	}
	for (const auto& passName : renderPassNames) {
		auto foundNode = std::find_if(nodes_.begin(), nodes_.end(), [&](const RenderPassNode& candidate) { return candidate.editorName == passName; });
		if (foundNode == nodes_.end()) continue;
		auto& node = *foundNode;
		bool validResources = true;
		for (const auto& input : node.pass->GetEditorInputs()) {
			const bool expectsDepth = input.first.find("Depth") != std::string::npos;
			const bool exists = expectsDepth ? resourceRegistry_.GetDepthBufferPointer(input.second) != nullptr : resourceRegistry_.GetColorBufferPointer(input.second) != nullptr;
			if (!exists) validResources = false;
			for (const auto& output : node.outputs) if (input.second == output.name) validResources = false;
		}
		for (const auto& output : node.outputs) if (!resourceRegistry_.GetColorBufferPointer(output.name)) validResources = false;
		for (const auto& output : node.uavOutputs) if (!resourceRegistry_.GetColorBufferPointer(output)) validResources = false;
		if (node.hasDepthOutput && !resourceRegistry_.GetDepthBufferPointer(node.depthOutput.name)) validResources = false;
		if (!validResources) {
			if (invalidPassWarnings_.insert(node.editorName).second) LogWarning("Render pass skipped because a referenced render resource was not found: " + node.editorName);
			continue;
		}
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
		// バリアはパスの利用状態が切り替わる位置で確定する。
		// 描画コマンドが発行されないパスが続いても、同一リソースへの
		// 複数回の遷移を一つの ResourceBarrier 呼び出しに混在させない。
		gfx.FlushResourceBarriers();
		node.pass->Execute(gfx, resourceRegistry_, registry);
	}

	DebugPrimitive::FrameEnd();
#ifdef USE_IMGUI
	gfx.TransitionResource(*resourceRegistry_.GetColorBufferPointer(screenDrawBufferName_), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	gfx.TransitionResource(*resourceRegistry_.GetColorBufferPointer("DebugColor"), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	gfx.TransitionResource(*resourceRegistry_.GetColorBufferPointer("WorldPosition"), D3D12_RESOURCE_STATE_COPY_SOURCE);
	gfx.FlushResourceBarriers();
	DrawGameImGuiWindow();
	DrawSceneImGuiWindow(registry, gfx, *resourceRegistry_.GetColorBufferPointer("WorldPosition"), positionReadbackBuffer_);
	DrawPassEditor(registry);

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

void RenderPassScheduler::LoadPassConfiguration(ECS::Registry& registry) {
	const std::string sceneName = Scene::GetCurrentSceneName(registry);
	if (sceneName.empty()) return;
	const std::string path = "resources/game/Scenes/" + sceneName + ".json";
	std::error_code timeError;
	auto writeTime = std::filesystem::last_write_time(path, timeError);
	if (passConfigLoaded_ && loadedSceneName_ == sceneName && !timeError && writeTime == loadedPassConfigWriteTime_) return;
	loadedSceneName_ = sceneName;
	passConfigLoaded_ = true;
	loadedPassConfigWriteTime_ = timeError ? std::filesystem::file_time_type{} : writeTime;
	activePassNames_.clear();
	lastSavedSettings_.clear();
	for (auto& node : nodes_) {
		auto defaults = defaultPassSettings_.find(node.editorName);
		if (defaults != defaultPassSettings_.end()) node.pass->LoadEditorSettings(defaults->second);
		RefreshPassNode(node);
	}

	nlohmann::json sceneData;
	std::ifstream in(path);
	if (in) {
		try { in >> sceneData; }
		catch (const std::exception&) { sceneData = nlohmann::json::object(); }
	}
	const bool hasSavedPasses = sceneData.contains("renderPasses") && sceneData["renderPasses"].is_array();
	if (hasSavedPasses) {
		for (const auto& entry : sceneData["renderPasses"]) {
			if (!entry.is_object() || !entry.contains("name") || !entry["name"].is_string()) continue;
			const std::string name = entry["name"].get<std::string>();
			auto node = std::find_if(nodes_.begin(), nodes_.end(), [&](const RenderPassNode& n) { return n.editorName == name; });
			if (node == nodes_.end() || !node->editorVisible) continue;
			if (std::find(activePassNames_.begin(), activePassNames_.end(), name) != activePassNames_.end()) continue;
			activePassNames_.push_back(name);
			if (entry.contains("settings")) node->pass->LoadEditorSettings(entry["settings"]);
			RefreshPassNode(*node);
			lastSavedSettings_[name] = node->pass->SaveEditorSettings();
		}
	} else {
		for (const auto& node : nodes_) {
			if (node.editorVisible && node.enabledByDefault) activePassNames_.push_back(node.editorName);
			lastSavedSettings_[node.editorName] = node.pass->SaveEditorSettings();
		}
	#ifdef USE_IMGUI
		SavePassConfiguration(registry);
	#endif
	}
}

void RenderPassScheduler::SavePassConfiguration(ECS::Registry& registry) {
	const std::string sceneName = Scene::GetCurrentSceneName(registry);
	if (sceneName.empty() || sceneName.find_first_of("/\\:*") != std::string::npos) return;
	std::error_code directoryError;
	std::filesystem::create_directories("resources/game/Scenes", directoryError);
	if (directoryError) return;
	const std::string path = "resources/game/Scenes/" + sceneName + ".json";
	nlohmann::json data = nlohmann::json::object();
	{
		std::ifstream in(path);
		if (in) {
			try { in >> data; }
			catch (const std::exception&) { return; }
		}
	}
	if (!data.contains("entities")) data["entities"] = nlohmann::json::object();
	data["renderPasses"] = nlohmann::json::array();
	for (const auto& name : activePassNames_) {
		auto node = std::find_if(nodes_.begin(), nodes_.end(), [&](const RenderPassNode& n) { return n.editorName == name; });
		if (node == nodes_.end() || !node->editorVisible) continue;
		data["renderPasses"].push_back({ {"name", name}, {"settings", node->pass->SaveEditorSettings()} });
	}
	std::ofstream out(path);
	if (out) {
		out << data.dump(4);
		out.flush();
		out.close();
		std::error_code ec;
		loadedPassConfigWriteTime_ = std::filesystem::last_write_time(path, ec);
	}
}

void RenderPassScheduler::DrawPassEditor(ECS::Registry& registry) {
#ifdef USE_IMGUI
	if (!ImGui::Begin("Render Passes")) { ImGui::End(); return; }
	bool changed = false;
	for (size_t i = 0; i < activePassNames_.size();) {
		const std::string name = activePassNames_[i];
		auto node = std::find_if(nodes_.begin(), nodes_.end(), [&](const RenderPassNode& n) { return n.editorName == name; });
		if (node == nodes_.end() || !node->editorVisible) { activePassNames_.erase(activePassNames_.begin() + i); changed = true; continue; }
		ImGui::PushID(static_cast<int>(i));
		const bool headerOpen = ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_AllowOverlap);
		if (ImGui::BeginDragDropSource()) {
			ImGui::SetDragDropPayload("RENDER_PASS_NAME", name.c_str(), name.size() + 1);
			ImGui::TextUnformatted(name.c_str());
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RENDER_PASS_NAME")) {
				const std::string dragged(static_cast<const char*>(payload->Data));
				auto source = std::find(activePassNames_.begin(), activePassNames_.end(), dragged);
				if (source != activePassNames_.end()) {
					const size_t sourceIndex = static_cast<size_t>(source - activePassNames_.begin());
					if (sourceIndex != i) {
						std::string moved = *source;
						activePassNames_.erase(source);
						const size_t targetIndex = sourceIndex < i ? i - 1 : i;
						activePassNames_.insert(activePassNames_.begin() + targetIndex, std::move(moved));
						changed = true;
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
		if (headerOpen) {
			const nlohmann::json beforeSettings = node->pass->SaveEditorSettings();
			node->pass->DrawEditor();
			if (node->pass->SaveEditorSettings() != beforeSettings) {
				RefreshPassNode(*node);
				changed = true;
			}
			if (ImGui::Button("Remove pass")) {
				activePassNames_.erase(activePassNames_.begin() + i);
				changed = true;
				ImGui::PopID();
				continue;
			}
		}
		ImGui::PopID();
		++i;
	}
	static int selectedPass = -1;
	if (ImGui::BeginCombo("Add pass", selectedPass >= 0 ? nodes_[selectedPass].editorName.c_str() : "Select a pass")) {
		for (size_t i = 0; i < nodes_.size(); ++i) {
			if (!nodes_[i].editorVisible) continue;
			const bool active = std::find(activePassNames_.begin(), activePassNames_.end(), nodes_[i].editorName) != activePassNames_.end();
			if (active) continue;
			if (ImGui::Selectable(nodes_[i].editorName.c_str(), selectedPass == static_cast<int>(i))) selectedPass = static_cast<int>(i);
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	if (ImGui::Button("Add") && selectedPass >= 0 && selectedPass < static_cast<int>(nodes_.size())) {
		const std::string& name = nodes_[selectedPass].editorName;
		if (std::find(activePassNames_.begin(), activePassNames_.end(), name) == activePassNames_.end()) {
			activePassNames_.push_back(name);
			changed = true;
		}
	}
	for (const auto& name : activePassNames_) {
		auto node = std::find_if(nodes_.begin(), nodes_.end(), [&](const RenderPassNode& n) { return n.editorName == name; });
		if (node == nodes_.end() || !node->editorVisible) continue;
		const nlohmann::json current = node->pass->SaveEditorSettings();
		if (!lastSavedSettings_.contains(name) || current != lastSavedSettings_[name]) {
			lastSavedSettings_[name] = current;
			changed = true;
		}
	}
	if (changed) SavePassConfiguration(registry);
	ImGui::End();
#else
	static_cast<void>(registry);
#endif
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

	renderPassScheduler.AddPass(std::make_unique<BLASUpdatePass>(), "BLAS update");
	renderPassScheduler.AddPass(std::make_unique<TLASBuildPass>(), "TLAS build");
	renderPassScheduler.AddPass(std::make_unique<LightPass>(), "Lighting");

	auto preRenderPass = std::make_unique<PreRenderPass>();
	preRenderPass->AddOutput("WorldPosition");
	preRenderPass->AddOutput("Normal");
	preRenderPass->AddOutput("ObjectID");
	preRenderPass->SetClearTarget(true);
	preRenderPass->SetDepthOutput("MainDepth", true);
	renderPassScheduler.AddPass(std::move(preRenderPass), "Pre-render");

	auto raytracingShadowPass = std::make_unique<RaytracingShadowPass>();
	raytracingShadowPass->AddInput("WorldPosition", "WorldPosition");
	raytracingShadowPass->AddInput("Normal", "Normal");
	raytracingShadowPass->AddUAVOutput("ShadowMask");
	raytracingShadowPass->SetClearTarget(true);

	renderPassScheduler.AddPass(std::move(raytracingShadowPass), "Raytracing shadow");

	auto meshPass = std::make_unique<MeshPass>();
	meshPass->AddInput("ShadowMask", "ShadowMask");
	meshPass->AddOutput("MainColor");
	meshPass->SetDepthOutput("MainDepth", true);
	meshPass->SetClearTarget(true);
	renderPassScheduler.AddPass(std::move(meshPass), "Opaque mesh");


	auto depthBasedOutlinePass = std::make_unique<DepthBasedOutlinePass>();
	depthBasedOutlinePass->AddInput("InputColor", "MainColor");
	depthBasedOutlinePass->AddInput("InputDepth", "MainDepth");
	depthBasedOutlinePass->AddOutput("PostEffect");
	renderPassScheduler.AddPass(std::move(depthBasedOutlinePass), "Depth outline");

	auto depthOfField = std::make_unique<DepthOfFieldPass>();
	depthOfField->AddInput("InputColor", "PostEffect");
	depthOfField->AddInput("InputDepth", "MainDepth");
	depthOfField->AddOutput("MainColor");
	depthOfField->SetFocusRange(60.0f);
	renderPassScheduler.AddPass(std::move(depthOfField), "Depth of field");

	auto transparentMeshPass = std::make_unique<TransparentMeshPass>();
	transparentMeshPass->AddInput("ShadowMask", "ShadowMask");
	transparentMeshPass->AddOutput("MainColor");
	transparentMeshPass->SetDepthOutput("MainDepth");
	renderPassScheduler.AddPass(std::move(transparentMeshPass), "Transparent mesh");

	auto cpuParticlePass = std::make_unique<CPUParticlePass>();
	cpuParticlePass->AddOutput("MainColor");
	cpuParticlePass->SetDepthOutput("MainDepth");
	renderPassScheduler.AddPass(std::move(cpuParticlePass), "CPU particles");

	auto particlePass = std::make_unique<ParticlePass>();
	particlePass->AddOutput("MainColor");
	particlePass->SetDepthOutput("MainDepth");
	renderPassScheduler.AddPass(std::move(particlePass), "GPU particles");

	auto bloomThreshold = std::make_unique<BloomThresholdPass>();
	bloomThreshold->AddInput("InputColor", "MainColor");
	bloomThreshold->AddOutput("BloomThreshold");
	renderPassScheduler.AddPass(std::move(bloomThreshold), "Bloom threshold");

	{
		auto bloomBlurH = std::make_unique<BloomBlurPass>(true);
		bloomBlurH->AddInput("InputColor", "BloomThreshold");
		bloomBlurH->AddOutput("BloomBlurA");
		renderPassScheduler.AddPass(std::move(bloomBlurH), "Bloom blur horizontal");

		auto bloomBlurV = std::make_unique<BloomBlurPass>(false);
		bloomBlurV->AddInput("InputColor", "BloomBlurA");
		bloomBlurV->AddOutput("BloomBlurB");
		renderPassScheduler.AddPass(std::move(bloomBlurV), "Bloom blur vertical");
	}

	auto bloomComposite = std::make_unique<BloomCompositePass>();
	bloomComposite->AddInput("SceneColor", "MainColor");
	bloomComposite->AddInput("BloomColor", "BloomBlurB");
	bloomComposite->AddOutput("BloomComposite");
	renderPassScheduler.AddPass(std::move(bloomComposite), "Bloom composite");

	auto vignetting = std::make_unique<VignettingPass>();
	vignetting->AddInput("InputColor", "BloomComposite");
	vignetting->AddOutput("MainColor");
	renderPassScheduler.AddPass(std::move(vignetting), "Vignetting");


	auto spritePass = std::make_unique<SpritePass>();
	spritePass->AddOutput("MainColor");
	spritePass->SetTargetCameraType(RenderPass::TargetCameraType::kMain);
	renderPassScheduler.AddPass(std::move(spritePass), "Main sprites");

	auto textPass = std::make_unique<TextPass>();
	textPass->AddOutput("MainColor");
	textPass->SetTargetCameraType(RenderPass::TargetCameraType::kMain);
	renderPassScheduler.AddPass(std::move(textPass), "Main text");

	// エディタから追加できる、コードに実装済みのポストエフェクト。
	// 既定のシーン構成には含めず、シーンごとに必要なものだけ追加する。
	auto grayscale = std::make_unique<GrayscalePass>();
	grayscale->AddInput("InputColor", "BloomComposite");
	grayscale->AddOutput("MainColor");
	renderPassScheduler.AddPass(std::move(grayscale), "Grayscale", false);

	auto gaussian = std::make_unique<GaussianFilterPass>();
	gaussian->AddInput("InputColor", "BloomComposite");
	gaussian->AddOutput("MainColor");
	renderPassScheduler.AddPass(std::move(gaussian), "Gaussian filter", false);

	auto boxFilter = std::make_unique<BoxFilterPass>();
	boxFilter->AddInput("InputColor", "BloomComposite");
	boxFilter->AddOutput("MainColor");
	renderPassScheduler.AddPass(std::move(boxFilter), "Box filter", false);

	auto radialBlur = std::make_unique<RadialBlurPass>();
	radialBlur->AddInput("InputColor", "BloomComposite");
	radialBlur->AddOutput("MainColor");
	renderPassScheduler.AddPass(std::move(radialBlur), "Radial blur", false);

	auto randomNoise = std::make_unique<RandomNoisePass>();
	randomNoise->AddInput("InputColor", "BloomComposite");
	randomNoise->AddOutput("MainColor");
	renderPassScheduler.AddPass(std::move(randomNoise), "Random noise", false);

	auto dissolve = std::make_unique<DissolvePass>();
	dissolve->AddInput("InputColor", "BloomComposite");
	dissolve->AddOutput("MainColor");
	renderPassScheduler.AddPass(std::move(dissolve), "Dissolve", false);

	auto skyBox = std::make_unique<SkyBoxPass>();
	skyBox->AddOutput("MainColor");
	renderPassScheduler.AddPass(std::move(skyBox), "Skybox", false);
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
	renderPassScheduler.AddPass(std::move(preRenderPass), "Debug pre-render", true, false);

	auto spriteIdPass = std::make_unique<SpriteIDPass>();
	spriteIdPass->AddOutput("ObjectID");
	spriteIdPass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(spriteIdPass), "Debug sprite IDs", true, false);

	auto raytracingShadowPass = std::make_unique<RaytracingShadowPass>();
	raytracingShadowPass->AddInput("WorldPosition", "WorldPosition");
	raytracingShadowPass->AddInput("Normal", "Normal");
	raytracingShadowPass->AddUAVOutput("ShadowMask");
	raytracingShadowPass->SetClearTarget(true);
	raytracingShadowPass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(raytracingShadowPass), "Debug raytracing shadow", true, false);

	auto meshPass = std::make_unique<MeshPass>();
	meshPass->AddInput("ShadowMask", "ShadowMask");
	meshPass->AddOutput("DebugColor");
	meshPass->SetDepthOutput("MainDepth", true);
	meshPass->SetClearTarget(true);
	meshPass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(meshPass), "Debug opaque mesh", true, false);

	auto transparentMeshPass = std::make_unique<TransparentMeshPass>();
	transparentMeshPass->AddInput("ShadowMask", "ShadowMask");
	transparentMeshPass->AddOutput("DebugColor");
	transparentMeshPass->SetDepthOutput("MainDepth");
	transparentMeshPass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(transparentMeshPass), "Debug transparent mesh", true, false);

	auto primitivePass = std::make_unique<PrimitivePass>();
	primitivePass->AddOutput("DebugColor");
	primitivePass->SetDepthOutput("MainDepth");
	primitivePass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(primitivePass), "Debug primitives", true, false);

	auto cpuParticlePass = std::make_unique<CPUParticlePass>();
	cpuParticlePass->AddOutput("DebugColor");
	cpuParticlePass->SetDepthOutput("MainDepth");
	cpuParticlePass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(cpuParticlePass), "Debug CPU particles", true, false);

	auto particlePass = std::make_unique<ParticlePass>();
	particlePass->AddOutput("DebugColor");
	particlePass->SetDepthOutput("MainDepth");
	particlePass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(particlePass), "Debug GPU particles", true, false);

	auto spritePass = std::make_unique<SpritePass>();
	spritePass->AddOutput("DebugColor");
	spritePass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(spritePass), "Debug sprites", true, false);

	auto textPass = std::make_unique<TextPass>();
	textPass->AddOutput("DebugColor");
	textPass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(textPass), "Debug text", true, false);
#else
	static_cast<void>(renderPassScheduler);
#endif // USE_IMGUI

}

}
