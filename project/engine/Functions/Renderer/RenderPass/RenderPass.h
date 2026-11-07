#pragma once
#include "engine/Runtime/Command/GraphicsContext.h"
#include "engine/Functions/ECS/Registry.h"
#include "../RenderContext.h"
#include "../RenderGraph.h"
#include "RenderPassEditorModel.h"
#include "externals/nlohmann/json.hpp"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace NoEngine {
namespace Render {
class RenderPass {
public:

	enum class TargetCameraType {
		kMain,
		kDebug,
		kCustom,
	};
	
	RenderPass() : renderContext_(nullptr) {}
	virtual ~RenderPass() = default;

	virtual void Setup(RenderGraphBuilder& builder) {
		for (const auto& input : inputs_) {
			builder.ReadTexture(input.second);
		}
		for (const auto& output : outputs_) {
			builder.WriteRenderTarget(output, autoClear_);
		}
		for (const auto& uavOutput : uavOutputs_) {
			builder.WriteUnorderedAccessTarget(uavOutput);
		}

		if (!depthOutput_.empty()) {
			builder.WriteDepthStencil(depthOutput_, clearDepth_);
		}
	}

	virtual void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) = 0;
	virtual void DrawEditor() {
#ifdef USE_IMGUI
        if (!ImGui::CollapsingHeader("詳細設定：リソースとカメラ")) return;
        int cameraType = static_cast<int>(targetCameraType_);
        const char* cameraTypes[] = { "メイン", "デバッグ", "名前で指定" };
        if (ImGui::Combo("描画カメラ", &cameraType, cameraTypes, 3)) targetCameraType_ = static_cast<TargetCameraType>(cameraType);
        if (targetCameraType_ == TargetCameraType::kCustom) {
            char cameraName[128]{};
            const size_t length = std::min(sizeof(cameraName) - 1, targetCameraName_.size());
            std::memcpy(cameraName, targetCameraName_.data(), length);
            if (ImGui::InputText("カメラ名", cameraName, sizeof(cameraName))) targetCameraName_ = cameraName;
        }
        if (!outputs_.empty()) ImGui::Checkbox("カラー出力をクリア", &autoClear_);
        if (!depthOutput_.empty()) ImGui::Checkbox("深度出力をクリア", &clearDepth_);
        auto selectResource = [&](const std::string& label, std::string& value) {
            if (ImGui::BeginCombo(label.c_str(), value.c_str())) {
                const auto options = editorResourceOptions_.find(label);
                if (options != editorResourceOptions_.end()) for (const auto& option : options->second) {
                    if (ImGui::Selectable(option.c_str(), option == value)) value = option;
                }
                ImGui::EndCombo();
            }
        };
        // 順序を固定し、unordered_mapの列挙順に表示を依存させない。
        std::vector<std::string> roles;
        for (const auto& [role, resource] : inputs_) { static_cast<void>(resource); roles.push_back(role); }
        std::sort(roles.begin(), roles.end());
        for (const auto& role : roles) selectResource(role + (inputTypes_.at(role) == EditorModel::ResourceType::Depth ? " [深度]" : " [カラー]"), inputs_.at(role));
        for (size_t i = 0; i < outputs_.size(); ++i) selectResource("出力 " + std::to_string(i) + " [カラー]", outputs_[i]);
        for (size_t i = 0; i < uavOutputs_.size(); ++i) selectResource("UAV出力 " + std::to_string(i) + " [UAV]", uavOutputs_[i]);
        if (!depthOutput_.empty()) selectResource("深度出力 [深度]", depthOutput_);
#endif
	}
	virtual nlohmann::json SaveEditorSettings() const {
		nlohmann::json result;
		result["targetCameraType"] = static_cast<int>(targetCameraType_);
		result["targetCameraName"] = targetCameraName_;
		result["autoClear"] = autoClear_;
		result["clearDepth"] = clearDepth_;
		result["inputs"] = inputs_;
		result["outputs"] = outputs_;
		result["uavOutputs"] = uavOutputs_;
		result["depthOutput"] = depthOutput_;
		return result;
	}
	virtual void LoadEditorSettings(const nlohmann::json& settings) {
		if (!settings.is_object()) return;
		if (settings.contains("targetCameraType") && settings["targetCameraType"].is_number_integer()) {
			int value = settings["targetCameraType"].get<int>();
			if (value >= 0 && value <= 2) targetCameraType_ = static_cast<TargetCameraType>(value);
		}
		if (settings.contains("targetCameraName") && settings["targetCameraName"].is_string()) targetCameraName_ = settings["targetCameraName"].get<std::string>();
		if (settings.contains("autoClear") && settings["autoClear"].is_boolean()) autoClear_ = settings["autoClear"].get<bool>();
		if (settings.contains("clearDepth") && settings["clearDepth"].is_boolean()) clearDepth_ = settings["clearDepth"].get<bool>();
		if (settings.contains("inputs") && settings["inputs"].is_object()) for (auto it = settings["inputs"].begin(); it != settings["inputs"].end(); ++it) if (it.value().is_string()) if (inputs_.contains(it.key())) inputs_[it.key()] = it.value().get<std::string>();
		if (settings.contains("outputs") && settings["outputs"].is_array()) for (size_t i = 0; i < std::min(outputs_.size(), settings["outputs"].size()); ++i) if (settings["outputs"][i].is_string()) outputs_[i] = settings["outputs"][i].get<std::string>();
		if (settings.contains("uavOutputs") && settings["uavOutputs"].is_array()) for (size_t i = 0; i < std::min(uavOutputs_.size(), settings["uavOutputs"].size()); ++i) if (settings["uavOutputs"][i].is_string()) uavOutputs_[i] = settings["uavOutputs"][i].get<std::string>();
		if (settings.contains("depthOutput") && settings["depthOutput"].is_string()) depthOutput_ = settings["depthOutput"].get<std::string>();
	}

	void AddInput(const std::string& role, const std::string& input, EditorModel::ResourceType type = EditorModel::ResourceType::Color) { inputs_[role] = input; inputTypes_[role] = type; }
    const auto& GetEditorInputTypes() const { return inputTypes_; }
    void SetEditorResourceOptions(std::unordered_map<std::string, std::vector<std::string>> options) { editorResourceOptions_ = std::move(options); }
	const std::unordered_map<std::string, std::string>& GetEditorInputs() const { return inputs_; }
	const std::vector<std::string>& GetEditorUavOutputs() const { return uavOutputs_; }
	void AddOutput(const std::string& output) { outputs_.emplace_back(output); }
	void AddUAVOutput(const std::string& output) { uavOutputs_.emplace_back(output); }
	void SetDepthOutput(const std::string& depth, bool clearDepth = false) { depthOutput_ = depth; clearDepth_ = clearDepth; }
	void SetClearTarget(bool autoClear) { autoClear_ = autoClear; }
	void SetTargetCameraType(TargetCameraType type) { targetCameraType_ = type; }
	void SetTargetCamera(const std::string& targetCamera) { 
		targetCameraType_ = TargetCameraType::kCustom;
		targetCameraName_ = targetCamera;
	}

	void SetRenderContext(RenderContext* renderContext) { renderContext_ = renderContext; }
protected:
	Component::CameraComponent* GetTargetCamera() { 
		switch (targetCameraType_) {
		case NoEngine::Render::RenderPass::TargetCameraType::kMain:
			return renderContext_->GetCamera();
			break;
		case NoEngine::Render::RenderPass::TargetCameraType::kDebug:
			return renderContext_->GetDebugCamera();
			break;
		case NoEngine::Render::RenderPass::TargetCameraType::kCustom:
			return renderContext_->GetCamera(targetCameraName_);
			break;
		default:
			return renderContext_->GetCamera();
			break;
		}
	}

	Component::Camera2DComponent* GetTargetCamera2D() {
		switch (targetCameraType_) {
		case NoEngine::Render::RenderPass::TargetCameraType::kMain:
			return renderContext_->GetCamera2D();
			break;
		case NoEngine::Render::RenderPass::TargetCameraType::kDebug:
			return renderContext_->GetDebugCamera2D();
			break;
		case NoEngine::Render::RenderPass::TargetCameraType::kCustom:
			return renderContext_->GetCamera2D(targetCameraName_);
			break;
		default:
			return renderContext_->GetCamera2D();
			break;
		}
	}
	RenderContext* GetRenderContext() { return renderContext_; }
	const std::string& GetInput(const std::string& role){
		if (!inputs_.contains(role))assert(false);
		return inputs_[role];
	}

private:
	std::unordered_map<std::string, std::string> inputs_;
    std::unordered_map<std::string, EditorModel::ResourceType> inputTypes_;
    std::unordered_map<std::string, std::vector<std::string>> editorResourceOptions_;
	std::vector<std::string> outputs_;
	std::vector<std::string> uavOutputs_;
	std::string depthOutput_;
	TargetCameraType targetCameraType_ = TargetCameraType::kMain;
	std::string targetCameraName_ = "GameCamera";
	bool autoClear_ = false;
	bool clearDepth_ = false;

	RenderContext* renderContext_;
};
}
}
