#include "engine/Editor/Utilities/MultiEdit.h"
#include "RenderPassScheduler.h"
#include "engine/Editor/Panels/FrameProfiler.h"
#include "RenderPassConfigurationFile.h"
#include "engine/Editor/Panels/EditorWindowManager.h"
#include "engine/Editor/Commands/EditorCommandOperator.h"
#include "RenderPassEditorCommand.h"
#include "Rasterization/SpritePass.h"
#include "Rasterization/MeshPass.h"
#include "Rasterization/TransparentMeshPass.h"
#include "Rasterization/PrimitivePass.h"
#include "Rasterization/GridPass.h"
#include "PrePasses/LightPass.h"
#include "Rasterization/CPUParticlePass.h"
#include "Rasterization/ParticlePass.h"
#include "PrePasses/TLASBuildPass.h"
#include "PrePasses/BLASUpdatePass.h"
#include "PrePasses/PreRenderPass.h"
#include "Raytracing/RaytracingShadowPass.h"
#include "Raytracing/VolumetricFogPass.h"
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

void RenderPassScheduler::AddPass(std::unique_ptr<RenderPass> pass, const std::string& editorName, bool enabledByDefault, bool editorVisible, std::vector<std::string> prerequisites) {
	RenderPassNode node;
	node.pass = std::move(pass);
	node.editorName = editorName.empty() ? "RenderPass" + std::to_string(nodes_.size()) : editorName;
	node.enabledByDefault = enabledByDefault;
	node.editorVisible = editorVisible;
    node.prerequisites = std::move(prerequisites);
	nodes_.push_back(std::move(node));
}

void RenderPassScheduler::Compile() {
	for (auto& node : nodes_) {
		RefreshPassNode(node);
		if (!defaultPassSettings_.contains(node.editorName)) defaultPassSettings_[node.editorName] = node.pass->SaveEditorSettings();
	}


    editorSchemas_.clear();
    editorResources_.clear();
    for (const auto& desc : resourceRegistry_.colorBufferDescs_) {
        const auto* buffer = resourceRegistry_.GetColorBufferPointer(desc.name);
        editorResources_.push_back({desc.name, EditorModel::ResourceType::Color, static_cast<unsigned>(desc.format), desc.depthOrArraySize,
            buffer && buffer->GetUAV().ptr != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN && buffer->GetUAV().ptr != 0});
    }
    for (const auto& desc : resourceRegistry_.depthBufferDescs_)
        editorResources_.push_back({desc.name, EditorModel::ResourceType::Depth, static_cast<unsigned>(desc.format), 1, false});
    std::sort(editorResources_.begin(), editorResources_.end(), [](const auto& a, const auto& b) { return a.name < b.name; });
    for (auto& node : nodes_) {
        const auto& defaults = defaultPassSettings_.at(node.editorName);
        editorSchemas_.push_back({node.editorName, defaults, node.pass->GetEditorInputTypes(), node.prerequisites, node.editorVisible});
        std::unordered_map<std::string, std::vector<std::string>> options;
        auto addOptions = [&](const std::string& label, const std::string& expected, EditorModel::ResourceType type, bool uav = false) {
            for (const auto& resource : editorResources_)
                if (EditorModel::Compatible(editorResources_, expected, resource.name, type, uav)) options[label].push_back(resource.name);
        };
        for (const auto& [role, type] : node.pass->GetEditorInputTypes())
            addOptions(role + (type == EditorModel::ResourceType::Depth ? " [深度]" : " [カラー]"), defaults["inputs"][role], type);
        for (size_t i = 0; i < defaults["outputs"].size(); ++i)
            addOptions("出力 " + std::to_string(i) + " [カラー]", defaults["outputs"][i], EditorModel::ResourceType::Color);
        for (size_t i = 0; i < defaults["uavOutputs"].size(); ++i)
            addOptions("UAV出力 " + std::to_string(i) + " [UAV]", defaults["uavOutputs"][i], EditorModel::ResourceType::Color, true);
        if (!defaults["depthOutput"].get<std::string>().empty()) addOptions("深度出力 [深度]", defaults["depthOutput"], EditorModel::ResourceType::Depth);
        node.pass->SetEditorResourceOptions(std::move(options));
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
	renderContext_.Update(registry);
	LoadPassConfiguration(registry);
	std::vector<std::string> renderPassNames = activePassNames_;
	for (const auto& node : nodes_) {
		if (!node.editorVisible) renderPassNames.push_back(node.editorName);
	}
#ifdef USE_IMGUI
	auto* profiler = Editor::FrameProfiler::Active();
	if (profiler) profiler->PrepareRenderPasses(renderPassNames.size());
#endif
	for (const auto& passName : renderPassNames) {
		auto foundNode = std::find_if(nodes_.begin(), nodes_.end(), [&](const RenderPassNode& candidate) { return candidate.editorName == passName; });
		if (foundNode == nodes_.end()) continue;
		auto& node = *foundNode;
		bool validResources = true;
		for (const auto& input : node.pass->GetEditorInputs()) {
			const bool expectsDepth = node.pass->GetEditorInputTypes().at(input.first) == EditorModel::ResourceType::Depth;
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
#ifdef USE_IMGUI
		if (profiler) profiler->BeginRenderPass(gfx, node.editorName);
#endif
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
#ifdef USE_IMGUI
		if (profiler) profiler->EndRenderPass(gfx);
#endif
	}

	DebugPrimitive::FrameEnd();
#ifdef USE_IMGUI
	if (profiler) profiler->ResolveRenderPasses(gfx);
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
		bool toggleSelection = false;
		bool addSelection = false;
		auto e = PickObject(gfx, *resourceRegistry_.GetColorBufferPointer("ObjectID"), idReadbackBuffer_, &toggleSelection, &addSelection);
		if (e != ECS::INVALID_ENTITY && registry.Has<Editor::EditTag>(e) && !registry.Has<Editor::NoClickSelectTag>(e)) {
			if (toggleSelection || addSelection) {
				auto selection = Editor::GetEditorSelection(registry);
				const auto found = std::find(selection.begin(), selection.end(), e);
				if (toggleSelection && found != selection.end()) selection.erase(found);
				else if (found == selection.end()) selection.push_back(e);
				Editor::SetEditorSelection(registry, selection, e);
			} else {
				registry.AddComponent<Editor::EditSelectedTag>(e);
			}
		}
	}

#endif // USE_IMGUI

}

nlohmann::json RenderPassScheduler::DefaultConfiguration() const {
    auto result = nlohmann::json::array();
    for (const auto& node : nodes_) if (node.editorVisible && node.enabledByDefault)
        result.push_back({{"name", node.editorName}, {"settings", defaultPassSettings_.at(node.editorName)}});
    return result;
}

std::vector<EditorModel::Diagnostic> RenderPassScheduler::ValidateConfiguration(const nlohmann::json& configuration) {
    auto errors = EditorModel::Validate(configuration, editorSchemas_, editorResources_, screenDrawBufferName_);
    if (!errors.empty()) return errors;
    for (const auto& entry : configuration) {
        const auto& settings = entry["settings"];
        if (settings["targetCameraType"].get<int>() == static_cast<int>(RenderPass::TargetCameraType::kCustom)) {
            const std::string camera = settings["targetCameraName"];
            // パスが2D/3Dどちらを参照しても安全なカスタムカメラだけを適用する。
            if (!renderContext_.GetCamera(camera) || !renderContext_.GetCamera2D(camera))
                errors.push_back({entry["name"], "指定した名前の2D・3Dカメラが両方必要です：" + camera});
        }
    }
    return errors;
}

bool RenderPassScheduler::ApplyConfiguration(const nlohmann::json& configuration) {
    if (!ValidateConfiguration(configuration).empty()) return false;
    std::vector<nlohmann::json> previous;
    for (const auto& node : nodes_) previous.push_back(node.pass->SaveEditorSettings());
    std::vector<std::string> names;
    try {
        for (const auto& entry : configuration) {
            const std::string name = entry["name"];
            auto node = std::find_if(nodes_.begin(), nodes_.end(), [&](const auto& n) { return n.editorName == name; });
            node->pass->LoadEditorSettings(defaultPassSettings_.at(name));
            node->pass->LoadEditorSettings(entry["settings"]);
            RefreshPassNode(*node);
            names.push_back(name);
        }
    } catch (const std::exception& exception) {
        for (size_t i = 0; i < nodes_.size(); ++i) {
            nodes_[i].pass->LoadEditorSettings(previous[i]);
            RefreshPassNode(nodes_[i]);
        }
        if (editorDocument_) editorDocument_->status = std::string("適用に失敗しました：") + exception.what();
        return false;
    }
    activePassNames_ = std::move(names);
    invalidPassWarnings_.clear();
    return true;
}

void RenderPassScheduler::RecordEditorChange(const nlohmann::json& before) {
#ifdef USE_IMGUI
    const auto document = editorDocument_;
    if (!document || document->draft == before) return;
    const auto after = document->draft;
    // Documentを履歴と共有し、Scheduler破棄後や別シーンの編集時にも参照を安全に保つ。
    Editor::EditorCommandOperator::AddCommand(std::make_unique<EditorModel::EditCommand>(document, before, after));
#else
    static_cast<void>(before);
#endif
}

void RenderPassScheduler::FlushEditorChange() {
    if (editorDocument_ && !editorDocument_->pendingBefore.is_null()) {
        RecordEditorChange(editorDocument_->pendingBefore);
        editorDocument_->pendingBefore = nullptr;
    }
}

void RenderPassScheduler::LoadPassConfiguration(ECS::Registry& registry) {
    const std::string sceneName = Scene::GetCurrentSceneName(registry);
    if (sceneName.empty() || sceneName.find_first_of("/\\:*") != std::string::npos) return;
    const std::string path = "resources/game/Scenes/" + sceneName + ".json";
    std::error_code ec;
    const auto writeTime = std::filesystem::last_write_time(path, ec);
    if (passConfigLoaded_ && loadedSceneName_ == sceneName) {
        if (!ec && writeTime != loadedPassConfigWriteTime_) {
            // 外部更新で未保存編集や適用済み構成を上書きしない。
            loadedPassConfigWriteTime_ = writeTime;
            try {
                std::ifstream in(path);
                const auto data = nlohmann::json::parse(in);
                if (data.value("renderPasses", nlohmann::json()) != editorDocument_->diskPasses)
                    editorDocument_->status = "ファイル内の描画パスが外部で変更されました。保存する前に「ファイルから再読込」で確認してください。";
            } catch (const std::exception&) { editorDocument_->status = "シーンファイルを読み込めません。編集中の設定と適用済みの設定は保持しています。"; }
        }
        return;
    }
    FlushEditorChange();
    loadedSceneName_ = sceneName;
    loadedPassConfigWriteTime_ = ec ? std::filesystem::file_time_type{} : writeTime;
    passConfigLoaded_ = true;
    auto& document = editorDocuments_[sceneName];
    if (!document) {
        document = std::make_shared<EditorModel::Document>();
        document->draft = DefaultConfiguration();
        try {
            std::ifstream in(path);
            if (in) {
                const auto data = nlohmann::json::parse(in);
                if (!data.is_object()) throw std::runtime_error("シーンデータはJSONオブジェクトである必要があります。");
                document->diskPasses = data.value("renderPasses", nlohmann::json());
                if (!document->diskPasses.is_null()) document->draft = EditorModel::Normalize(document->diskPasses, editorSchemas_);
            } else if (std::filesystem::exists(path)) throw std::runtime_error("シーンファイルを開けません。");
        } catch (const std::exception& exception) { document->status = std::string("読込に失敗しました：") + exception.what(); }
        document->saved = document->draft;
    }
    editorDocument_ = document;
    const auto& desired = document->applied.empty() ? document->draft : document->applied;
    if (ApplyConfiguration(desired)) document->applied = desired;
    else {
        const auto defaults = DefaultConfiguration();
        if (ApplyConfiguration(defaults)) document->applied = defaults;
        else activePassNames_.clear();
        document->status = "保存された構成に問題があります。描画には安全な既定の構成を使用します。設定を修正するか、ファイルから再読込してください。";
    }
}

bool RenderPassScheduler::SavePassConfiguration(ECS::Registry& registry) {
    if (!editorDocument_ || !ValidateConfiguration(editorDocument_->draft).empty()) return false;
    const std::string sceneName = Scene::GetCurrentSceneName(registry);
    if (sceneName != loadedSceneName_ || sceneName.empty() || sceneName.find_first_of("/\\:*") != std::string::npos) return false;
    const std::filesystem::path path = std::filesystem::path("resources/game/Scenes") / (sceneName + ".json");
    try {
        EditorModel::SaveConfigurationFile(path, editorDocument_->diskPasses, editorDocument_->draft);
        editorDocument_->saved = editorDocument_->draft;
        editorDocument_->diskPasses = editorDocument_->draft;
        std::error_code ec;
        loadedPassConfigWriteTime_ = std::filesystem::last_write_time(path, ec);
        editorDocument_->status = "保存しました。描画への反映には「適用」を押してください。";
        return true;
    } catch (const std::exception& exception) {
        editorDocument_->status = std::string("保存に失敗しました：") + exception.what();
        return false;
    }
}

void RenderPassScheduler::DrawPassEditor(ECS::Registry& registry) {
#ifdef USE_IMGUI
    for (auto& node : nodes_) if (node.editorName == "Debug grid") node.pass->DrawEditor();
    auto& window = Editor::EditorWindowManager::GetInstance().Get(Editor::EditorWindowId::RenderPasses);
    if (!window.open || !editorDocument_) { FlushEditorChange(); return; }
    if (!ImGui::Begin(window.label, &window.open)) { FlushEditorChange(); ImGui::End(); return; }
    auto& document = *editorDocument_;
    // スライダー操作をフレーム単位でなく、一回のドラッグとして履歴にまとめる。
    if (!ImGui::IsAnyItemActive()) FlushEditorChange();
    ImGui::Text("シーン：%s", loadedSceneName_.c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("%s | %s", !document.diskPasses.is_null() && document.draft == document.saved ? "保存済み" : "未保存",
        document.draft == document.applied ? "適用済み" : "未適用");
    ImGui::BeginDisabled(!Editor::EditorCommandOperator::CanUndo());
    if (ImGui::Button("元に戻す")) { FlushEditorChange(); Editor::EditorCommandOperator::Undo(); }
    ImGui::EndDisabled(); ImGui::SameLine();
    ImGui::BeginDisabled(!Editor::EditorCommandOperator::CanRedo());
    if (ImGui::Button("やり直す")) { FlushEditorChange(); Editor::EditorCommandOperator::Redo(); }
    ImGui::EndDisabled(); ImGui::SameLine();
    auto errors = ValidateConfiguration(document.draft);
    ImGui::BeginDisabled(!errors.empty());
    if (ImGui::Button("適用")) {
        FlushEditorChange();
        if (ApplyConfiguration(document.draft)) { document.applied = document.draft; document.status = "描画に適用しました。この構成をファイルに残すには「保存」を押してください。"; }
    }
    ImGui::SameLine();
    if (ImGui::Button("保存")) { FlushEditorChange(); SavePassConfiguration(registry); }
    ImGui::EndDisabled(); ImGui::SameLine();
    if (ImGui::Button("保存状態に戻す")) {
        FlushEditorChange(); const auto before = document.draft;
        document.draft = document.saved; RecordEditorChange(before);
        document.status = "編集中の設定を保存状態に戻しました。描画も戻すには「適用」を押してください。";
    }
    ImGui::SameLine();
    if (ImGui::Button("ファイルから再読込")) { FlushEditorChange(); ImGui::OpenPopup("描画パスを再読込しますか？"); }
    bool reload = false;
    if (ImGui::BeginPopupModal("描画パスを再読込しますか？", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("編集中の設定と描画の構成を、ファイルの内容で置き換えますか？");
        ImGui::TextUnformatted("未保存の変更は破棄されます。");
        if (ImGui::Button("再読込")) { reload = true; ImGui::CloseCurrentPopup(); }
        ImGui::SameLine(); if (ImGui::Button("キャンセル")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("元に戻す／やり直すは共通の編集履歴を使用します（Ctrl+Z／Ctrl+Shift+Z）。適用と保存は別の操作です。");
    ImGui::PopTextWrapPos();
    if (!document.status.empty()) ImGui::TextWrapped("%s", document.status.c_str());
    ImGui::Separator();
    const float height = std::max(160.0f, ImGui::GetContentRegionAvail().y - 125.0f);
    if (ImGui::BeginTable("Pass layout", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn("実行順", ImGuiTableColumnFlags_WidthFixed, 250.0f);
        ImGui::TableSetupColumn("詳細", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow(); ImGui::TableNextColumn();
        ImGui::BeginChild("Pass list", ImVec2(0, height));
        // 追加・削除・移動は描画ループの後で反映し、参照とImGui IDを安定させる。
        int moveFrom = -1, moveTo = -1, removeIndex = -1;
        if (document.draft.is_array()) for (size_t i = 0; i < document.draft.size(); ++i) {
            const auto& entry = document.draft[i];
            if (!entry.is_object() || !entry.contains("name") || !entry["name"].is_string()) {
                ImGui::PushID(static_cast<int>(i));
                ImGui::TextUnformatted("不正なパス項目");
                if (ImGui::SmallButton("不正な項目を削除")) removeIndex = static_cast<int>(i);
                ImGui::PopID(); continue;
            }
            const std::string name = entry["name"];
            ImGui::PushID(name.c_str());
            const bool invalid = std::any_of(errors.begin(), errors.end(), [&](const auto& e) { return e.pass == name; });
            const std::string label = std::to_string(i + 1) + ". " + (invalid ? "[!] " : "") + name;
            if (ImGui::Selectable(label.c_str(), document.selected == name)) document.selected = name;
            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("RENDER_PASS_NAME", name.c_str(), name.size() + 1);
                ImGui::TextUnformatted(name.c_str()); ImGui::EndDragDropSource();
            }
            if (ImGui::BeginDragDropTarget()) {
                if (const auto* payload = ImGui::AcceptDragDropPayload("RENDER_PASS_NAME")) {
                    const std::string source(static_cast<const char*>(payload->Data));
                    for (size_t j = 0; j < document.draft.size(); ++j) if (EditorModel::HasName(document.draft[j], source)) {
                        moveFrom = static_cast<int>(j); moveTo = static_cast<int>(i); break;
                    }
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::PopID();
        }
        if (ImGui::BeginCombo("パスを追加", "パスを選択")) {
            for (const auto& node : nodes_) {
                if (!node.editorVisible || !document.draft.is_array()) continue;
                const bool active = std::any_of(document.draft.begin(), document.draft.end(), [&](const auto& entry) { return EditorModel::HasName(entry, node.editorName); });
                if (!active && ImGui::Selectable(node.editorName.c_str())) {
                    FlushEditorChange(); const auto before = document.draft;
                    document.draft.push_back({{"name", node.editorName}, {"settings", defaultPassSettings_.at(node.editorName)}});
                    document.selected = node.editorName; RecordEditorChange(before);
                }
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("既定の構成に戻す")) {
            FlushEditorChange(); const auto before = document.draft;
            document.draft = DefaultConfiguration(); RecordEditorChange(before);
        }
        ImGui::EndChild(); ImGui::TableNextColumn();
        ImGui::BeginChild("Pass details", ImVec2(0, height));
        auto selected = document.draft.end();
        if (document.draft.is_array()) selected = std::find_if(document.draft.begin(), document.draft.end(), [&](const auto& entry) { return EditorModel::HasName(entry, document.selected); });
        if (selected != document.draft.end()) {
            const int index = static_cast<int>(selected - document.draft.begin());
            ImGui::PushID(document.selected.c_str());
            ImGui::TextUnformatted(document.selected.c_str());
            ImGui::BeginDisabled(index == 0);
            if (ImGui::Button("上へ移動")) { moveFrom = index; moveTo = index - 1; }
            ImGui::EndDisabled(); ImGui::SameLine();
            ImGui::BeginDisabled(index + 1 == static_cast<int>(document.draft.size()));
            if (ImGui::Button("下へ移動")) { moveFrom = index; moveTo = index + 1; }
            ImGui::EndDisabled(); ImGui::SameLine();
            if (ImGui::Button("パスを削除")) removeIndex = index;
            const auto node = std::find_if(nodes_.begin(), nodes_.end(), [&](const auto& n) { return n.editorName == document.selected && n.editorVisible; });
            if (node != nodes_.end()) {
                for (const auto& required : node->prerequisites) ImGui::TextDisabled("先に実行する必要があるパス：%s", required.c_str());
                if (ImGui::Button("パスの設定を初期化")) {
                    FlushEditorChange(); const auto before = document.draft;
                    (*selected)["settings"] = defaultPassSettings_.at(node->editorName); RecordEditorChange(before);
                }
                // 実行用インスタンスを一時的に編集用に貸し、描画前には必ず適用済み設定へ戻す。
                const auto appliedSettings = node->pass->SaveEditorSettings();
                const auto before = document.draft;
                try {
                    node->pass->LoadEditorSettings(defaultPassSettings_.at(node->editorName));
                    node->pass->LoadEditorSettings(selected->at("settings"));
                    const auto beforeSettings = node->pass->SaveEditorSettings();
                    node->pass->DrawEditor();
                    const auto afterSettings = node->pass->SaveEditorSettings();
                    if (afterSettings != beforeSettings) {
                        if (document.pendingBefore.is_null()) document.pendingBefore = before;
                        (*selected)["settings"] = afterSettings;
                    }
                } catch (const std::exception&) { ImGui::TextWrapped("設定が不正です。「パスの設定を初期化」で修復してください。"); }
                node->pass->LoadEditorSettings(appliedSettings);
            }
            ImGui::PopID();
        } else ImGui::TextWrapped("一覧からパスを選択すると設定を編集できます。パスは一覧の上から順に実行されます。");
        ImGui::EndChild(); ImGui::EndTable();
        if (removeIndex >= 0 || (moveFrom >= 0 && moveTo >= 0 && moveFrom != moveTo)) {
            FlushEditorChange(); const auto before = document.draft;
            if (removeIndex >= 0) document.draft.erase(document.draft.begin() + removeIndex);
            else {
                auto entry = document.draft[moveFrom];
                document.draft.erase(document.draft.begin() + moveFrom);
                document.draft.insert(document.draft.begin() + moveTo, std::move(entry));
            }
            RecordEditorChange(before);
        }
    }
    if (!ImGui::IsAnyItemActive()) FlushEditorChange();
    ImGui::Separator();
    errors = ValidateConfiguration(document.draft);
    ImGui::Text("検証結果：エラー %zu 件", errors.size());
    ImGui::BeginChild("Pass diagnostics", ImVec2(0, 0));
    for (size_t i = 0; i < errors.size(); ++i) {
        const auto& error = errors[i];
        ImGui::PushID(static_cast<int>(i));
        if (!error.pass.empty() && ImGui::SmallButton(error.pass.c_str())) document.selected = error.pass;
        if (!error.pass.empty()) ImGui::SameLine();
        ImGui::TextWrapped("%s", error.message.c_str()); ImGui::PopID();
    }
    if (errors.empty()) ImGui::TextUnformatted("適用・保存できます。");
    ImGui::EndChild(); ImGui::End();
    if (reload) {
        const auto previousDocument = editorDocument_;
        const auto before = previousDocument->draft;
        editorDocuments_.erase(loadedSceneName_);
        editorDocument_.reset();
        passConfigLoaded_ = false;
        LoadPassConfiguration(registry);
        // 再読込後も同じDocumentを使い、既存Undoが切り離されたデータを操作しないようにする。
        const auto selection = previousDocument->selected;
        *previousDocument = *editorDocument_;
        previousDocument->selected = selection;
        editorDocument_ = previousDocument;
        editorDocuments_[loadedSceneName_] = previousDocument;
        RecordEditorChange(before);
    }
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
	resourceRegistry.CreateColorBuffer("ShadowFilterScratch", windowSize.x, windowSize.y, DXGI_FORMAT_R8_UNORM);
	resourceRegistry.CreateColorBuffer("WorldPosition", windowSize.x, windowSize.y, 1, DXGI_FORMAT_R32G32B32A32_FLOAT, Math::Color(0.f, 0.f, 0.f, 0.f));
	resourceRegistry.CreateColorBuffer("Normal", windowSize.x, windowSize.y, DXGI_FORMAT_R10G10B10A2_UNORM);
	resourceRegistry.CreateColorBuffer("ObjectID", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM);

	// ブルーム
	resourceRegistry.CreateColorBuffer("BloomThreshold", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	resourceRegistry.CreateColorBuffer("BloomBlurA", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	resourceRegistry.CreateColorBuffer("BloomBlurB", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	resourceRegistry.CreateColorBuffer("BloomComposite", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

	resourceRegistry.CreateColorBuffer("PostEffect", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	resourceRegistry.CreateColorBuffer("DepthOfField", windowSize.x, windowSize.y, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
	resourceRegistry.CreateColorBuffer("VolumetricFog", windowSize.x, windowSize.y, 1, DXGI_FORMAT_R16G16B16A16_FLOAT,
		Math::Color(0.f, 0.f, 0.f, 1.f));
	renderPassScheduler.SetScreenDrawBuffer("MainColor");
	
	resourceRegistry.CreateDepthBuffer("MainDepth", windowSize.x, windowSize.y);

	renderPassScheduler.AddPass(std::make_unique<BLASUpdatePass>(), "BLAS update");
	renderPassScheduler.AddPass(std::make_unique<TLASBuildPass>(), "TLAS build", true, true, {"BLAS update"});
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

	renderPassScheduler.AddPass(std::move(raytracingShadowPass), "Raytracing shadow", true, true, {"TLAS build", "Lighting"});

	auto meshPass = std::make_unique<MeshPass>();
	meshPass->AddInput("ShadowMask", "ShadowMask");
	meshPass->AddOutput("MainColor");
	meshPass->SetDepthOutput("MainDepth", true);
	meshPass->SetClearTarget(true);
	renderPassScheduler.AddPass(std::move(meshPass), "Opaque mesh", true, true, {"Lighting", "Pre-render"});


	auto depthBasedOutlinePass = std::make_unique<DepthBasedOutlinePass>();
	depthBasedOutlinePass->AddInput("InputColor", "MainColor");
	depthBasedOutlinePass->AddInput("InputDepth", "MainDepth", EditorModel::ResourceType::Depth);
	depthBasedOutlinePass->AddOutput("PostEffect");
	renderPassScheduler.AddPass(std::move(depthBasedOutlinePass), "Depth outline");

	auto depthOfField = std::make_unique<DepthOfFieldPass>();
	depthOfField->AddInput("InputColor", "PostEffect");
	depthOfField->AddInput("InputDepth", "MainDepth", EditorModel::ResourceType::Depth);
	depthOfField->AddOutput("MainColor");
	depthOfField->SetFocusRange(60.0f);
	renderPassScheduler.AddPass(std::move(depthOfField), "Depth of field");

	auto fogPass = std::make_unique<VolumetricFogPass>();
	fogPass->AddInput("InputDepth", "MainDepth", EditorModel::ResourceType::Depth);
	fogPass->AddUAVOutput("VolumetricFog");
	renderPassScheduler.AddPass(std::move(fogPass), "Fog integrate", false, true, {"TLAS build", "Lighting", "Opaque mesh"});

	auto fogCompositePass = std::make_unique<VolumetricFogCompositePass>();
	fogCompositePass->AddInput("Fog", "VolumetricFog");
	fogCompositePass->AddOutput("MainColor");
	renderPassScheduler.AddPass(std::move(fogCompositePass), "Fog composite", false, true, {"Fog integrate"});

	auto transparentMeshPass = std::make_unique<TransparentMeshPass>();
	transparentMeshPass->AddInput("ShadowMask", "ShadowMask");
	transparentMeshPass->AddOutput("MainColor");
	transparentMeshPass->SetDepthOutput("MainDepth");
	renderPassScheduler.AddPass(std::move(transparentMeshPass), "Transparent mesh", true, true, {"Opaque mesh", "Lighting"});

	auto cpuParticlePass = std::make_unique<CPUParticlePass>();
	cpuParticlePass->AddOutput("MainColor");
	cpuParticlePass->SetDepthOutput("MainDepth");
	renderPassScheduler.AddPass(std::move(cpuParticlePass), "CPU particles", true, true, {"Opaque mesh"});

	auto particlePass = std::make_unique<ParticlePass>();
	particlePass->AddOutput("MainColor");
	particlePass->SetDepthOutput("MainDepth");
	renderPassScheduler.AddPass(std::move(particlePass), "GPU particles", true, true, {"Opaque mesh"});

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
	renderPassScheduler.AddPass(std::move(spritePass), "Main sprites", true, true, {"Opaque mesh"});

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
	renderPassScheduler.AddPass(std::move(skyBox), "Skybox", false, true, {"Opaque mesh"});
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
	renderPassScheduler.AddPass(std::move(raytracingShadowPass), "Debug raytracing shadow", true, false, {"TLAS build", "Lighting"});

	auto meshPass = std::make_unique<MeshPass>();
	meshPass->AddInput("ShadowMask", "ShadowMask");
	meshPass->AddOutput("DebugColor");
	meshPass->SetDepthOutput("MainDepth", true);
	meshPass->SetClearTarget(true);
	meshPass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(meshPass), "Debug opaque mesh", true, false, {"Lighting"});

	auto transparentMeshPass = std::make_unique<TransparentMeshPass>();
	transparentMeshPass->AddInput("ShadowMask", "ShadowMask");
	transparentMeshPass->AddOutput("DebugColor");
	transparentMeshPass->SetDepthOutput("MainDepth");
	transparentMeshPass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(transparentMeshPass), "Debug transparent mesh", true, false);

	auto cpuParticlePass = std::make_unique<CPUParticlePass>(true);
	cpuParticlePass->AddOutput("DebugColor");
	cpuParticlePass->SetDepthOutput("MainDepth");
	cpuParticlePass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(cpuParticlePass), "Debug CPU particles", true, false);

	auto particlePass = std::make_unique<ParticlePass>();
	particlePass->AddOutput("DebugColor");
	particlePass->SetDepthOutput("MainDepth");
	particlePass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(particlePass), "Debug GPU particles", true, false);

    // 色の加工が終わったシーンへ重ね、深度を書き込むデバッグ線より先に描く。
    auto gridPass = std::make_unique<GridPass>();
    gridPass->AddOutput("DebugColor");
    gridPass->SetDepthOutput("MainDepth");
    gridPass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
    renderPassScheduler.AddPass(std::move(gridPass), "Debug grid", true, false);

    auto primitivePass = std::make_unique<PrimitivePass>();
    primitivePass->AddOutput("DebugColor");
    primitivePass->SetDepthOutput("MainDepth");
    primitivePass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
    renderPassScheduler.AddPass(std::move(primitivePass), "Debug primitives", true, false);

	auto spritePass = std::make_unique<SpritePass>();
	spritePass->AddOutput("DebugColor");
	spritePass->SetTargetCameraType(RenderPass::TargetCameraType::kDebug);
	renderPassScheduler.AddPass(std::move(spritePass), "Debug sprites", true, false);
#else
	static_cast<void>(renderPassScheduler);
#endif // USE_IMGUI

}

}
