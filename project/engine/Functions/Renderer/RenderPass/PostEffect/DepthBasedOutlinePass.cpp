#include "stdafx.h"
#include "DepthBasedOutlinePass.h"
#include "engine/Functions/Shader/ShaderReflection.h"

namespace NoEngine {
namespace Render {
void DepthBasedOutlinePass::DrawEditor() {
	RenderPass::DrawEditor();
#ifdef USE_IMGUI
	ImGui::Checkbox("チョーク風の輪郭", &chalkEnabled_);
	if (chalkEnabled_) {
		ImGui::DragFloat("線幅 (px)", &chalkWidth_, 0.05f, 0.5f, 5.0f);
		ImGui::DragFloat("輪郭のしきい値", &chalkThreshold_, 0.001f, 0.001f, 0.2f, "%.3f");
		ImGui::SliderFloat("チョークのかすれ", &chalkRoughness_, 0.0f, 1.0f);
		ImGui::SliderFloat("面の明るさ", &chalkSurfaceBrightness_, 0.0f, 1.0f);
		ImGui::ColorEdit3("チョークの色", &chalkColor_.x);
	}
#endif
}

nlohmann::json DepthBasedOutlinePass::SaveEditorSettings() const {
	auto settings = RenderPass::SaveEditorSettings();
	settings["chalkEnabled"] = chalkEnabled_;
	settings["chalkWidth"] = chalkWidth_;
	settings["chalkThreshold"] = chalkThreshold_;
	settings["chalkRoughness"] = chalkRoughness_;
	settings["chalkSurfaceBrightness"] = chalkSurfaceBrightness_;
	settings["chalkColor"] = { chalkColor_.x, chalkColor_.y, chalkColor_.z };
	return settings;
}

void DepthBasedOutlinePass::LoadEditorSettings(const nlohmann::json& settings) {
	if (!settings.is_object()) return;
	RenderPass::LoadEditorSettings(settings);
	if (settings.contains("chalkEnabled") && settings["chalkEnabled"].is_boolean()) chalkEnabled_ = settings["chalkEnabled"].get<bool>();
	auto loadFloat = [&](const char* key, float& value, float minimum, float maximum) {
		if (settings.contains(key) && settings[key].is_number()) {
			const float loaded = settings[key].get<float>();
			if (std::isfinite(loaded)) value = std::clamp(loaded, minimum, maximum);
		}
	};
	loadFloat("chalkWidth", chalkWidth_, 0.5f, 5.0f);
	loadFloat("chalkThreshold", chalkThreshold_, 0.001f, 0.2f);
	loadFloat("chalkRoughness", chalkRoughness_, 0.0f, 1.0f);
	loadFloat("chalkSurfaceBrightness", chalkSurfaceBrightness_, 0.0f, 1.0f);
	if (settings.contains("chalkColor") && settings["chalkColor"].is_array() && settings["chalkColor"].size() == 3) {
		auto color = settings["chalkColor"];
		if (color[0].is_number() && color[1].is_number() && color[2].is_number()) {
			const Math::Vector3 loaded = { color[0].get<float>(), color[1].get<float>(), color[2].get<float>() };
			if (std::isfinite(loaded.x) && std::isfinite(loaded.y) && std::isfinite(loaded.z))
				chalkColor_ = { std::clamp(loaded.x, 0.0f, 1.0f), std::clamp(loaded.y, 0.0f, 1.0f), std::clamp(loaded.z, 0.0f, 1.0f) };
		}
	}
}

void DepthBasedOutlinePass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(registry);
	if (!GetTargetCamera()) return;

	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : Outline");
	auto renderCtx = GetRenderContext();
	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : Outline PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : Outline"));
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, resourceRegistry.GetColorBuffer(GetInput("InputColor")).GetSRV());
	gfx.SetDynamicDescriptor(rootIndex["gDepthTexture"], 0, resourceRegistry.GetDepthBuffer(GetInput("InputDepth")).GetDepthSRV());
	auto projection = GetTargetCamera()->projection;
	projection.Inverse();

	// HLSLの16バイト境界に合わせ、通常の輪郭とチョーク表現の定数を渡す。
	_declspec(align(16)) struct {
		Math::Matrix4x4 projectionInverse;
		float chalkEnabled;
		float chalkWidth;
		float chalkThreshold;
		float chalkRoughness;
		Math::Vector3 chalkColor;
		float chalkSurfaceBrightness;
	} constants{ projection, chalkEnabled_ ? 1.0f : 0.0f, chalkWidth_, chalkThreshold_, chalkRoughness_, chalkColor_, chalkSurfaceBrightness_ };
	gfx.SetDynamicConstantBufferView(rootIndex["gMaterial"], sizeof(constants), &constants);
	gfx.Draw(3);
}
}
}
