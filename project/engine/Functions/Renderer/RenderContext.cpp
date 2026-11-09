#include "RenderContext.h"
#include "engine/Assets/Model/ModelSaver.h"
#include "engine/Functions/ECS/Component/Asset/AnimatorComponent.h"
#include "engine/Functions/ECS/Component/Asset/LightComponent.h"
#include "engine/Functions/ECS/Component/Asset/MaterialComponent.h"
#include "engine/Functions/ECS/Component/Asset/MeshComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Editor/Utilities/EditUtils.h"

#include "Initializer/RenderInitializer.h"
#include "LightForGPU.h"

namespace NoEngine {
using namespace Component;

RenderContext::RenderContext() {
	if (isInitialized_) return;
	RenderInitializer::Initialize(*this);

	isInitialized_ = true;
}

void RenderContext::Update(ECS::Registry& registry) {
	// 収集済みのリストを無効化し、確保済みの領域は次の描画で再利用する。
	activeMeshDrawListCount_ = 0;

	// 3Dカメラ収集
	{
		auto cameraView = registry.View<CameraComponent, ActiveCameraTag>();
		for (auto e : cameraView) {
			camera_ = registry.GetComponent<CameraComponent>(e);
		}

		auto debugCameraView = registry.View<CameraComponent, DebugCameraComponent>();
		for (auto e : debugCameraView) {
			debugCamera_ = registry.GetComponent<CameraComponent>(e);
		}

		auto view = registry.View<CameraComponent, Editor::EditTag>();
		cameras_.clear();
		for (auto e : view) {
			auto name = registry.GetComponent<Editor::EditTag>(e)->name;
			auto camera = registry.GetComponent<CameraComponent>(e);
			cameras_[name] = camera;
		}
	}

	// 2Dカメラ収集
	{
		auto cameraView = registry.View<Camera2DComponent, ActiveCamera2DTag>();
		for (auto e : cameraView) {
			camera2d_ = registry.GetComponent<Camera2DComponent>(e);
		}

		auto debugCameraView = registry.View<Camera2DComponent, DebugCamera2DComponent>();
		for (auto e : debugCameraView) {
			debugCamera2d_ = registry.GetComponent<Camera2DComponent>(e);
		}

		auto view = registry.View<Camera2DComponent, Editor::EditTag>();
		cameras2d_.clear();
		for (auto e : view) {
			auto name = registry.GetComponent<Editor::EditTag>(e)->name;
			auto camera = registry.GetComponent<Camera2DComponent>(e);
			cameras2d_[name] = camera;
		}
	}

}

const RenderContext::MeshDrawLists& RenderContext::GetMeshDrawLists(ECS::Registry& registry, const CameraComponent& camera) {
	for (size_t i = 0; i < activeMeshDrawListCount_; ++i) {
		if (meshDrawLists_[i].camera == &camera) return meshDrawLists_[i].lists;
	}

	if (activeMeshDrawListCount_ == meshDrawLists_.size()) meshDrawLists_.emplace_back();
	auto& entry = meshDrawLists_[activeMeshDrawListCount_];
	auto& lists = entry.lists;
	lists.opaque.clear();
	lists.transparent.clear();

	const auto cameraPos = camera.forGPU.worldPosition;
	for (auto entity : registry.View<TransformComponent, MeshComponent, MaterialComponent>()) {
		const auto* meshComponent = registry.GetComponent<MeshComponent>(entity);
		if (!meshComponent->isVisible) continue;
		auto* mesh = ModelSaver::Get().GetMesh(meshComponent->handle);
		if (!mesh || mesh->subMeshes.empty()) continue;
		auto* material = registry.GetComponent<MaterialComponent>(entity);
		auto* transform = registry.GetComponent<TransformComponent>(entity);
		auto* animator = registry.GetComponent<AnimatorComponent>(entity);
		Transform* animationLocal = animator ? &animator->local : nullptr;
		const float distance = MathCalculations::LengthSquared(transform->translate - cameraPos);

		auto addDraw = [&](std::span<const SubMesh> subMeshes, MeshMaterialSettings& settings) {
			BlendMode blendMode = settings.blendMode;
			if (settings.renderMode == RenderMode::kEmissive) {
				blendMode = BlendMode::kAdd;
			} else if (blendMode == BlendMode::kNormal && settings.color.a < 1.0f) {
				blendMode = BlendMode::kAlpha;
			}
			auto& items = blendMode == BlendMode::kNormal ? lists.opaque : lists.transparent;
			items.push_back({mesh, subMeshes, &settings, blendMode, transform, animationLocal, distance, animator});
		};

		const auto& modelName = meshComponent->meshName;
		const std::span<const SubMesh> subMeshes = mesh->subMeshes;
		const bool hasOverrides = meshComponent->loadedMeshName == modelName &&
			std::ranges::any_of(material->subMeshOverrides, [&](const auto& entry) {
				return entry.meshName == modelName && entry.subMeshIndex < subMeshes.size();
			});
		if (!hasOverrides) {
			addDraw(subMeshes, *material);
			continue;
		}
		// 共通設定が続く範囲はまとめ、差し替えたサブメッシュだけを別々に振り分ける。
		uint32_t sharedStart = 0;
		for (uint32_t i = 0; i < subMeshes.size(); ++i) {
			if (auto* slot = material->FindSubMeshOverride(modelName, i)) {
				if (sharedStart < i) addDraw(subMeshes.subspan(sharedStart, i - sharedStart), *material);
				addDraw(subMeshes.subspan(i, 1), *slot);
				sharedStart = i + 1;
			}
		}
		if (sharedStart < subMeshes.size()) addDraw(subMeshes.subspan(sharedStart), *material);
	}

	// 不透明は手前から奥、半透明は奥から手前の順に描画する。
	std::sort(lists.opaque.begin(), lists.opaque.end(), [](const MeshDrawItem& a, const MeshDrawItem& b) {
		return a.distanceToCamera < b.distanceToCamera;
	});
	std::sort(lists.transparent.begin(), lists.transparent.end(), [](const MeshDrawItem& a, const MeshDrawItem& b) {
		return a.distanceToCamera > b.distanceToCamera;
	});

	entry.camera = &camera;
	++activeMeshDrawListCount_;
	return lists;
}

void RenderContext::SetDirectionalLight(GraphicsContext& gfx, UploadBuffer& directionalLightUpload, uint32_t directionalLightNum) {
	bool recreate = false;
	if (lightNums_.directionalLightNum != directionalLightNum) recreate = true;
	if (directionalLightNum == 0) return;
	
	if (recreate) {
		lightNums_.directionalLightNum = directionalLightNum;
		directionalLightBuffer_.Create(
			L"DirectionalLights",
			directionalLightNum,
			sizeof(DirectionalLightComponent),
			directionalLightUpload
		);
	}
	

	gfx.CopyBufferRegion(directionalLightBuffer_, 0, directionalLightUpload, 0, sizeof(DirectionalLightComponent) * directionalLightNum);
	gfx.TransitionResource(directionalLightBuffer_, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

}

void RenderContext::SetPointLight(GraphicsContext& gfx, UploadBuffer& pointLightUpload, uint32_t pointLightNum) {
	bool recreate = false;
	if (lightNums_.pointLightNum != pointLightNum) recreate = true;
	if (pointLightNum == 0) return;

	if (recreate) {
		lightNums_.pointLightNum = pointLightNum;
		pointLightBuffer_.Create(
			L"PointLights",
			pointLightNum,
			sizeof(PointLightForGPU),
			pointLightUpload
		);
	}


	gfx.CopyBufferRegion(pointLightBuffer_, 0, pointLightUpload, 0, sizeof(PointLightForGPU) * pointLightNum);
	gfx.TransitionResource(pointLightBuffer_, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);

}

void RenderContext::SetSpotLight(GraphicsContext& gfx, UploadBuffer& spotLightUpload, uint32_t spotLightNum) {
	bool recreate = false;
	if (lightNums_.spotLightNum != spotLightNum) recreate = true;
	if (spotLightNum == 0) return;

	if (recreate) {
		lightNums_.spotLightNum = spotLightNum;
		spotLightBuffer_.Create(
			L"SpotLights",
			spotLightNum,
			sizeof(SpotLightForGPU),
			spotLightUpload
		);
	}


	gfx.CopyBufferRegion(spotLightBuffer_, 0, spotLightUpload, 0, sizeof(SpotLightForGPU) * spotLightNum);
	gfx.TransitionResource(spotLightBuffer_, D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE);
}

HRESULT RenderContext::EnsureTLASCapacity(UINT64 sizeInBytes) {
	if (tlas_ && tlas_->GetDesc().Width >= sizeInBytes) return S_OK;

	const auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	const auto tlasDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	return GraphicsCore::sGraphicsDevice->GetDevice()->CreateCommittedResource(
		&defaultHeap,
		D3D12_HEAP_FLAG_NONE,
		&tlasDesc,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		nullptr,
		IID_PPV_ARGS(&tlas_)
	);
}

}
