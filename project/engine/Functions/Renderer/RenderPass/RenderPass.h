#pragma once
#include "engine/Runtime/Command/GraphicsContext.h"
#include "engine/Functions/ECS/Registry.h"
#include "../RenderContext.h"
#include "../RenderGraph.h"
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
		auto copyToBuffer = [](char* destination, size_t capacity, const std::string& source) {
			const size_t length = std::min(capacity - 1, source.size());
			std::memcpy(destination, source.data(), length);
			destination[length] = '\0';
		};
		int cameraType = static_cast<int>(targetCameraType_);
		const char* cameraTypes[] = { "Main", "Debug", "Custom" };
		if (ImGui::Combo("Target camera", &cameraType, cameraTypes, 3)) targetCameraType_ = static_cast<TargetCameraType>(cameraType);
		if (targetCameraType_ == TargetCameraType::kCustom) {
			char cameraName[128]{};
			copyToBuffer(cameraName, sizeof(cameraName), targetCameraName_);
			if (ImGui::InputText("Camera name", cameraName, sizeof(cameraName))) targetCameraName_ = cameraName;
		}
		ImGui::Checkbox("Clear color target", &autoClear_);
		ImGui::Checkbox("Clear depth target", &clearDepth_);
		for (auto& input : inputs_) {
			char label[128]{}; char value[128]{};
			copyToBuffer(label, sizeof(label), input.first);
			copyToBuffer(value, sizeof(value), input.second);
			if (ImGui::InputText(label, value, sizeof(value))) input.second = value;
		}
		for (size_t i = 0; i < outputs_.size(); ++i) {
			char label[64]{}; char value[128]{};
			std::snprintf(label, sizeof(label), "Output %zu", i);
			copyToBuffer(value, sizeof(value), outputs_[i]);
			if (ImGui::InputText(label, value, sizeof(value))) outputs_[i] = value;
		}
		for (size_t i = 0; i < uavOutputs_.size(); ++i) {
			char label[64]{}; char value[128]{};
			std::snprintf(label, sizeof(label), "UAV output %zu", i);
			copyToBuffer(value, sizeof(value), uavOutputs_[i]);
			if (ImGui::InputText(label, value, sizeof(value))) uavOutputs_[i] = value;
		}
		if (!depthOutput_.empty()) {
			char value[128]{}; copyToBuffer(value, sizeof(value), depthOutput_);
			if (ImGui::InputText("Depth output", value, sizeof(value))) depthOutput_ = value;
		}
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
		if (settings.contains("inputs") && settings["inputs"].is_object()) for (auto it = settings["inputs"].begin(); it != settings["inputs"].end(); ++it) if (it.value().is_string()) inputs_[it.key()] = it.value().get<std::string>();
		if (settings.contains("outputs") && settings["outputs"].is_array()) for (size_t i = 0; i < std::min(outputs_.size(), settings["outputs"].size()); ++i) if (settings["outputs"][i].is_string()) outputs_[i] = settings["outputs"][i].get<std::string>();
		if (settings.contains("uavOutputs") && settings["uavOutputs"].is_array()) for (size_t i = 0; i < std::min(uavOutputs_.size(), settings["uavOutputs"].size()); ++i) if (settings["uavOutputs"][i].is_string()) uavOutputs_[i] = settings["uavOutputs"][i].get<std::string>();
		if (settings.contains("depthOutput") && settings["depthOutput"].is_string()) depthOutput_ = settings["depthOutput"].get<std::string>();
	}

	void AddInput(const std::string& role, const std::string& input) { inputs_[role] = input; }
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
