#include "MeshPass.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Utilities/Conversion/ConvertString.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Functions/ECS/Component/Asset/AnimatorComponent.h"
#include "engine/Assets/Model/ModelSaver.h"
#include "engine/Assets/AssetManager.h"
#include "engine/Assets/Texture/TextureManager.h"

#include "engine/Runtime/GameCore.h"

namespace NoEngine {
namespace Render {

using namespace Component;

static bool IsTransparent(MaterialComponent* m) {
	if (m->renderMode == RenderMode::kEmissive) {
		m->blendMode = BlendMode::kAdd;
	}
	return m->blendMode != BlendMode::kNormal || m->color.a < 1.0f;
}

MeshPass::MeshPass() : camera_(nullptr) {

	skyBoxTexture_ = TextureManager::LoadTextureFile("resources/engine/Texture/rostock_laage_airport_4k.dds");
}

void MeshPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(resourceRegistry);

	if (psoIDs_.empty()) {
		auto& context = *GetRenderContext();
		psoIDs_[RenderMode::kDefault][BlendMode::kNormal] = context.GetPSOID("Renderer : Default PSO");
		psoIDs_[RenderMode::kDefault][BlendMode::kAdd] = context.GetPSOID("Renderer : Default Add PSO");
		psoIDs_[RenderMode::kDefault][BlendMode::kSubtract] = context.GetPSOID("Renderer : Default Sub PSO");
		psoIDs_[RenderMode::kDefault][BlendMode::kMultiply] = context.GetPSOID("Renderer : Default Mul PSO");
		psoIDs_[RenderMode::kDefault][BlendMode::kScreen] = context.GetPSOID("Renderer : Default Screen PSO");

		psoIDs_[RenderMode::kToon][BlendMode::kNormal] = context.GetPSOID("Renderer : Toon PSO");
		psoIDs_[RenderMode::kToon][BlendMode::kAdd] = context.GetPSOID("Renderer : Toon Add PSO");
		psoIDs_[RenderMode::kToon][BlendMode::kSubtract] = context.GetPSOID("Renderer : Toon Sub PSO");
		psoIDs_[RenderMode::kToon][BlendMode::kMultiply] = context.GetPSOID("Renderer : Toon Mul PSO");
		psoIDs_[RenderMode::kToon][BlendMode::kScreen] = context.GetPSOID("Renderer : Toon Screen PSO");

		psoIDs_[RenderMode::kEmissive][BlendMode::kAdd] = context.GetPSOID("Renderer : Emissive PSO");

		rootSigIDs_[RenderMode::kDefault] = context.GetRootSignatureID("Renderer : Default PSO");
		rootSigIDs_[RenderMode::kToon] = context.GetRootSignatureID("Renderer : Toon PSO");
		rootSigIDs_[RenderMode::kEmissive] = context.GetRootSignatureID("Renderer : Emissive PSO");
	}

	Collect(registry);
	Sort();
	RenderItems(gfx, resourceRegistry, opaqueItems_);      // 不透明を先に
	RenderItems(gfx, resourceRegistry, transparentItems_); // 半透明を後に
	RenderOutline(gfx);
}

void MeshPass::Collect(ECS::Registry& registry) {
	auto view = registry.View<
		TransformComponent,
		MeshComponent,
		MaterialComponent
	>();
	opaqueItems_.clear();
	transparentItems_.clear();

	camera_ = GetTargetCamera();
	if (camera_ == nullptr) return;
	Math::Vector3 cameraPos = camera_->forGPU.worldPosition;

	for (auto entity : view) {
		auto* mesh = registry.GetComponent<MeshComponent>(entity);
		if (!mesh->isVisible) continue;
		auto* material = registry.GetComponent<MaterialComponent>(entity);
		auto* transform = registry.GetComponent<TransformComponent>(entity);
		auto* anime = registry.GetComponent<AnimatorComponent>(entity);
		Transform* animeLocal = nullptr;
		if (anime) {
			animeLocal = &anime->local;
		}

		float distance = MathCalculations::LengthSquared(transform->translate - cameraPos);

		DrawItem item{ mesh->handle, material->handles, material, transform, animeLocal, distance };

		if (IsTransparent(material)) {
			transparentItems_.push_back(std::move(item));
		} else {
			opaqueItems_.push_back(std::move(item));
		}
	}
}

void MeshPass::Sort() {
	// 不透明: Early-Z効率を上げるため前→奥。PSO単位でまとめてステート変更コストを削減してもよい
	std::sort(opaqueItems_.begin(), opaqueItems_.end(),
		[](const DrawItem& a, const DrawItem& b) {
			return a.distanceToCamera < b.distanceToCamera;
		});

	// 半透明: PSOを無視して奥→前(距離降順)で厳密にソートし、正しい合成順を保証する
	std::sort(transparentItems_.begin(), transparentItems_.end(),
		[](const DrawItem& a, const DrawItem& b) {
			return a.distanceToCamera > b.distanceToCamera;
		});
}

void MeshPass::RenderItems(GraphicsContext& context, const RenderGraphRegistry& resourceRegistry, const std::vector<DrawItem>& items) {
	(void)resourceRegistry;
	uint32_t currentPSO = 0xFFFFFFFF;
	auto* renderCtx = GetRenderContext();
	for (auto& item : items) {
		std::string basePsoName = "Renderer : Default PSO";
		if (item.material->renderMode == RenderMode::kToon) {
			basePsoName = "Renderer : Toon PSO";
		} else if (item.material->renderMode == RenderMode::kEmissive) {
			basePsoName = "Renderer : Emissive PSO";
		}
		auto& rootIndex = RootSignatureBuilder::GetRootIndexMap(basePsoName);

		// psoIDs_から対象のPSO IDとRootSignature IDを取得する
		uint32_t targetPSO = psoIDs_[item.material->renderMode][item.material->blendMode];
		uint32_t targetRootSig = rootSigIDs_[item.material->renderMode];

		if (targetPSO != currentPSO) {
			context.SetPipelineState(renderCtx->GetGraphicsPSO(targetPSO));
			context.SetRootSignature(renderCtx->GetRootSignature(targetRootSig));
			context.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			currentPSO = targetPSO;
		}

		struct MeshConstants {
			Math::Matrix4x4 world;
			Math::Matrix4x4 worldIT;
		};
		MeshConstants m;
		if (item.animationLocal) {
			m.world = item.animationLocal->MakeAffineMatrix4x4() * item.transform->MakeAffineMatrix4x4();
			m.worldIT = m.world;
		} else {
			m.world = item.transform->MakeAffineMatrix4x4();
			m.worldIT = m.world;
		}

		m.worldIT.Inverse();
		m.worldIT.Transpose();
		context.SetDynamicConstantBufferView(rootIndex["gWorldMatrix"], sizeof(MeshConstants), &m);
		context.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Component::CameraForGPU), &camera_->forGPU);

		{
			if (item.material->renderMode != RenderMode::kEmissive) {
				_declspec(align(16)) struct {
					uint32_t directionalLightNum = 0;
					uint32_t pointLightNum = 0;
					uint32_t spotLightNum = 0;
					uint32_t pad = 0;
				}constants;
				constants.directionalLightNum = renderCtx->GetLightNums()->directionalLightNum;
				constants.pointLightNum = renderCtx->GetLightNums()->pointLightNum;
				constants.spotLightNum = renderCtx->GetLightNums()->spotLightNum;
				context.SetDynamicConstantBufferView(rootIndex["gLightNums"], sizeof(constants), &constants);

				if (constants.directionalLightNum)
					context.SetDynamicDescriptor(rootIndex["gDirectionalLights"], 0, renderCtx->GetDirectionalLightSRV());
				if (constants.pointLightNum)
					context.SetDynamicDescriptor(rootIndex["gPointLights"], 0, renderCtx->GetPointLightSRV());
				if (constants.spotLightNum)
					context.SetDynamicDescriptor(rootIndex["gSpotLights"], 0, renderCtx->GetSpotLightSRV());
			} else {
				/*
				struct EmissiveMaterial
{
    float4 color; // ベースカラー（発光色）
    float intensity; // 発光強度
    float rimPower; // フレネル(縁)の鋭さ
    float scrollSpeed; // ノイズのスクロール速度
    float time; // 経過時間
};
				*/
				_declspec(align(16)) struct { 
					Math::Color color = Math::Color::BLACK; 
					float intensity = 0.0f; 
					float rimPower = 0.0f; 
					float scrollSpeed = 0.0f;
					float time = 0.0f;
				} emissiveConstants{
					item.material->color,
					item.material->emissiveIntensity,
					item.material->rimPower,
					item.material->noiseScrollSpeed,
					GameCore::GetElapsedTime()
				};
				context.SetDynamicConstantBufferView(rootIndex["gEmissive"], sizeof(emissiveConstants), &emissiveConstants);
				if (!item.material->noiseTextureHandle.IsValid()) {
					item.material->noiseTextureHandle = TextureManager::LoadCovertTexture(AssetManager::GetFilePathFromAddressableName(item.material->noiseTextureName));
				}
				context.SetDynamicDescriptor(rootIndex["gNoiseTexture"], 0, item.material->noiseTextureHandle.GetSRV());
			}
			
		}
		auto* mesh = ModelSaver::Get().GetMesh(item.meshHandle);
		if (!mesh) continue;
		context.SetVertexBuffer(0, mesh->useVertexBuffer.VertexBufferView());
		context.SetIndexBuffer(mesh->indexBuffer.IndexBufferView());

		for (const auto& subMesh : mesh->subMeshes) {

			_declspec(align(16)) struct {
				Math::Color color;
				float shininess;
				float environmentCoefficient;
				float padding[2];
				Math::Matrix4x4 uvTransform;
			}constants;
			constants.color = item.material->color;
			constants.shininess = item.material->shininess;
			constants.environmentCoefficient = item.material->enviromentCoefficient;

			// uvTransform
			Math::Quaternion uvRotate;
			uvRotate.FromAxisAngle(Math::Vector3::FORWARD, item.material->uvRotate);
			constants.uvTransform.MakeAffine(
				Math::Vector3(item.material->uvScale.x, item.material->uvScale.y, 1.0f),
				uvRotate,
				Math::Vector3(item.material->uvPosition.x, item.material->uvPosition.y, 0.0f)
			);
			if (item.material->renderMode != RenderMode::kEmissive) {
				context.SetDynamicConstantBufferView(rootIndex["gMaterial"], sizeof(constants), &constants);
				auto* material = ModelSaver::Get().GetMaterial(item.materialHandles[subMesh.materialIndex]);
				context.SetDynamicDescriptor(rootIndex["gTexture"], 0, material->textureHandle.GetSRV());
				context.SetDynamicDescriptor(rootIndex["gShadowMask"], 0, resourceRegistry.GetColorBuffer("ShadowMask").GetSRV());
				context.SetDynamicDescriptor(rootIndex["gEnvironmentTexture"], 0, skyBoxTexture_.GetSRV());
			}
			
			context.DrawIndexedInstanced(subMesh.indexCount, 1, subMesh.indexStart, subMesh.vertexStart, 0);
		}
	}
}

void MeshPass::RenderOutline(GraphicsContext& context) {
	if (opaqueItems_.empty()) return;
	auto* renderCtx = GetRenderContext();
	outlinePSOName_ = "Renderer : outline PSO";
	uint32_t a = renderCtx->GetRootSignatureID(outlinePSOName_);
	a;
	outlinePSOID_ = renderCtx->GetPSOID(outlinePSOName_);
	uint32_t outlineRootSigID = renderCtx->GetRootSignatureID(outlinePSOName_);

	context.SetPipelineState(renderCtx->GetGraphicsPSO(outlinePSOID_));
	context.SetRootSignature(renderCtx->GetRootSignature(outlineRootSigID));
	context.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto& item : opaqueItems_) {
		if (!item.material->drawOutline) continue;

		auto& rootIndex = RootSignatureBuilder::GetRootIndexMap(outlinePSOName_);

		Math::Matrix4x4 worldData = item.transform->MakeAffineMatrix4x4();
		if (item.animationLocal) {
			worldData = item.animationLocal->MakeAffineMatrix4x4() * worldData;
		}
		context.SetDynamicConstantBufferView(rootIndex["gWorldMatrix"], sizeof(Math::Matrix4x4), &worldData);
		context.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Component::CameraForGPU), &camera_->forGPU);

		auto* mesh = ModelSaver::Get().GetMesh(item.meshHandle);
		if (!mesh) continue;
		context.SetVertexBuffer(0, mesh->useVertexBuffer.VertexBufferView());
		context.SetIndexBuffer(mesh->indexBuffer.IndexBufferView());

		for (const auto& subMesh : mesh->subMeshes) {
			context.DrawIndexedInstanced(subMesh.indexCount, 1, subMesh.indexStart, subMesh.vertexStart, 0);
		}
	}
}

}
}