#include "stdafx.h"
#include "VolumetricFogPass.h"
#include "../../RaytracingManager.h"
#include "engine/Functions/ECS/Component/Asset/FogVolumeComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Runtime/Command/ComputeContext.h"
#include "engine/Runtime/GameCore.h"
#include <cmath>

namespace NoEngine::Render {

void VolumetricFogPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resources, ECS::Registry& registry) {
	auto* context = GetRenderContext();
	const auto* camera = GetTargetCamera();
	auto& fog = const_cast<ColorBuffer&>(resources.GetColorBuffer(GetEditorUavOutputs().front()));
	auto* pipeline = GetFogRtStateObject();
	if (!pipeline || !camera) {
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
		Math::Matrix4x4 worldToBox;
		float boxDensity;
		float boxEdgeFade;
		float boxNoiseFrequency;
		float boxNoiseStrength;
		Math::Vector3 boxNoiseOffset;
		uint32_t directionalLightCount;
	} constants{};
	static_assert(sizeof(FogConstants) == 256);

	// 1つの有効な Box をレイ生成シェーダーへ渡す。
	for (auto entity : registry.View<Component::FogVolumeComponent, Component::TransformComponent>()) {
		const auto* volume = registry.GetComponent<Component::FogVolumeComponent>(entity);
		if (!volume->enabled || volume->density <= 0.f || !std::isfinite(volume->density) || !std::isfinite(volume->edgeFade)) continue;

		const auto* transform = registry.GetComponent<Component::TransformComponent>(entity);
		auto worldToBox = transform->MakeAffineMatrix4x4(registry);
		const auto& m = worldToBox.m;
		const Math::Vector3 x(m[0][0], m[0][1], m[0][2]);
		const Math::Vector3 y(m[1][0], m[1][1], m[1][2]);
		const Math::Vector3 z(m[2][0], m[2][1], m[2][2]);
		const float determinant = x.Dot(y.Cross(z));
		if (determinant == 0.f || !std::isfinite(determinant)) continue;
		worldToBox.Inverse();
		if (!std::ranges::all_of(worldToBox.m, [](const auto& row) {
			return std::ranges::all_of(row, [](float value) { return std::isfinite(value); });
		})) continue;

		constants.worldToBox = worldToBox;
		constants.boxDensity = volume->density;
		constants.boxEdgeFade = std::clamp(volume->edgeFade, 0.f, 0.5f);
		if (std::isfinite(volume->noiseFrequency) && volume->noiseFrequency > 0.f &&
			std::isfinite(volume->noiseStrength) && volume->noiseStrength > 0.f) {
			constants.boxNoiseFrequency = std::min(volume->noiseFrequency, 32.f);
			constants.boxNoiseStrength = std::min(volume->noiseStrength, 1.f);
			const float time = GameCore::GetElapsedTime();
			const double noiseTime = std::isfinite(time) ? static_cast<double>(time) * constants.boxNoiseFrequency : 0.0;
			const auto noiseOffset = [noiseTime](float velocity) {
				// シェーダーの格子周期で折り返し、長時間の再生でも座標の精度を保つ。
				return std::isfinite(velocity) ? static_cast<float>(std::fmod(velocity * noiseTime, 256.0)) : 0.f;
			};
			constants.boxNoiseOffset = {
				noiseOffset(volume->windVelocity.x), noiseOffset(volume->windVelocity.y), noiseOffset(volume->windVelocity.z)
			};
		}
		break;
	}

	if (density_ <= 0.f && constants.boxDensity <= 0.f) {
		gfx.GetComputeContext().ClearUAV(fog);
		return;
	}

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
	constants.directionalLightCount = context->GetLightNums()->directionalLightNum;
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
	gfx.SetComputeSRV(4, constants.directionalLightCount > 0 ? context->GetDirectionalLightBuffer().GetGpuVirtualAddress() : 0);
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
