#include "stdafx.h"
#include "VolumetricFogPass.h"
#include "../../RaytracingManager.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Runtime/Command/ComputeContext.h"

namespace NoEngine::Render {

void VolumetricFogPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resources, ECS::Registry& registry) {
	static_cast<void>(registry);
	auto* context = GetRenderContext();
	const auto* camera = GetTargetCamera();
	auto& fog = const_cast<ColorBuffer&>(resources.GetColorBuffer(GetEditorUavOutputs().front()));
	auto* pipeline = GetFogRtStateObject();
	if (!pipeline || !camera || density_ <= 0.f) {
		gfx.GetComputeContext().ClearUAV(fog);
		return;
	}

	struct alignas(16) FogConstants {
		Math::Matrix4x4 inverseViewProjection;
		Math::Vector3 cameraPosition;
		float density;
		float baseHeight;
		float heightFalloff;
		float maxDistance;
		uint32_t stepCount;
		Math::Vector3 spotPosition;
		float spotRange;
		Math::Vector3 spotDirection;
		float spotCosOuter;
		Math::Vector3 spotRadiance;
		float spotCosInner;
		float spotDecay;
		float ambient;
		float scatteringAlbedo;
		uint32_t traceShadows;
	} constants{};
	static_assert(sizeof(FogConstants) == 160);

	constants.inverseViewProjection = camera->forGPU.viewProjection;
	constants.inverseViewProjection.Inverse();
	constants.cameraPosition = camera->forGPU.worldPosition;
	constants.density = density_;
	constants.baseHeight = baseHeight_;
	constants.heightFalloff = heightFalloff_;
	constants.maxDistance = maxDistance_;
	constants.stepCount = static_cast<uint32_t>(stepCount_);
	constants.ambient = ambient_;
	constants.scatteringAlbedo = scatteringAlbedo_;
	constants.traceShadows = traceShadows_ && context->IsTLASReady();
	if (const auto& light = context->GetFogSpotLight(); light && light->intensity > 0.f && light->distance > 0.f) {
		constants.spotPosition = light->position;
		constants.spotRange = light->distance;
		constants.spotDirection = light->direction.Normalize();
		constants.spotCosOuter = light->cosAngle;
		constants.spotRadiance = light->color.ToVector3() * light->intensity;
		constants.spotCosInner = light->cosFalloffStart;
		constants.spotDecay = std::max(0.f, light->decay);
	}

	auto& depth = const_cast<DepthBuffer&>(resources.GetDepthBuffer(GetInput("InputDepth")));
	gfx.TransitionResource(depth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	gfx.SetRaytracingRootSignature(GetFogRtRootSignature());
	const auto& tlas = context->GetTLAS();
	gfx.SetComputeSRV(0, constants.traceShadows ? tlas->GetGPUVirtualAddress() : 0);
	gfx.SetRaytracingDynamicDescriptor(1, 0, depth.GetDepthSRV());
	gfx.SetRaytracingDynamicDescriptor(2, 0, fog.GetUAV());
	gfx.SetRaytracingDynamicConstantBufferView(3, sizeof(constants), &constants);
	gfx.GetCommandList()->SetPipelineState1(pipeline);

	auto dispatch = GetFogDispatchRaysDesc();
	dispatch.Width = fog.GetWidth();
	dispatch.Height = fog.GetHeight();
	gfx.DispatchRays(dispatch);
}

void VolumetricFogPass::DrawEditor() {
	RenderPass::DrawEditor();
#ifdef USE_IMGUI
	ImGui::DragFloat("Density", &density_, 0.001f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloat("Base height", &baseHeight_, 0.1f, -10000.f, 10000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloat("Height falloff", &heightFalloff_, 0.001f, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloat("Max distance", &maxDistance_, 1.f, 1.f, 10000.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloat("Ambient", &ambient_, 0.01f, 0.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragFloat("Scattering albedo", &scatteringAlbedo_, 0.01f, 0.f, 1.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::DragInt("Steps", &stepCount_, 1.f, 1, 64, "%d", ImGuiSliderFlags_AlwaysClamp);
	ImGui::Checkbox("Trace fog shadows", &traceShadows_);
#endif
}

nlohmann::json VolumetricFogPass::SaveEditorSettings() const {
	auto settings = RenderPass::SaveEditorSettings();
	settings["density"] = density_;
	settings["baseHeight"] = baseHeight_;
	settings["heightFalloff"] = heightFalloff_;
	settings["maxDistance"] = maxDistance_;
	settings["ambient"] = ambient_;
	settings["scatteringAlbedo"] = scatteringAlbedo_;
	settings["stepCount"] = stepCount_;
	settings["traceShadows"] = traceShadows_;
	return settings;
}

void VolumetricFogPass::LoadEditorSettings(const nlohmann::json& settings) {
	if (!settings.is_object()) return;
	RenderPass::LoadEditorSettings(settings);
	density_ = std::clamp(settings.value("density", density_), 0.f, 1.f);
	baseHeight_ = std::clamp(settings.value("baseHeight", baseHeight_), -10000.f, 10000.f);
	heightFalloff_ = std::clamp(settings.value("heightFalloff", heightFalloff_), 0.f, 1.f);
	maxDistance_ = std::clamp(settings.value("maxDistance", maxDistance_), 1.f, 10000.f);
	ambient_ = std::clamp(settings.value("ambient", ambient_), 0.f, 1.f);
	scatteringAlbedo_ = std::clamp(settings.value("scatteringAlbedo", scatteringAlbedo_), 0.f, 1.f);
	stepCount_ = std::clamp(settings.value("stepCount", stepCount_), 1, 64);
	traceShadows_ = settings.value("traceShadows", traceShadows_);
}

void VolumetricFogCompositePass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resources, ECS::Registry& registry) {
	static_cast<void>(registry);
	auto* context = GetRenderContext();
	gfx.SetPipelineState(context->GetGraphicsPSO("Renderer : VolumetricFogComposite PSO"));
	gfx.SetRootSignature(context->GetRootSignature("Renderer : VolumetricFogComposite"));
	gfx.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	const auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : VolumetricFogComposite");
	gfx.SetDynamicDescriptor(rootIndex.at("gFog"), 0, resources.GetColorBuffer(GetInput("Fog")).GetSRV());
	gfx.Draw(3);
}

}
