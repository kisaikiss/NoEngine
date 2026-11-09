#include "MeshPass.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Utilities/Conversion/ConvertString.h"
#include "engine/Runtime/GraphicsCore.h"
#include "engine/Functions/ECS/Component/Asset/AnimatorComponent.h"
#include "engine/Functions/ECS/Component/Common/CameraComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"
#include "engine/Assets/Model/ModelSaver.h"
#include "engine/Assets/AssetManager.h"
#include "engine/Assets/Texture/TextureManager.h"

#include "engine/Runtime/GameCore.h"

namespace NoEngine {
namespace Render {

using namespace Component;

// ============================== MeshPassBase ==============================

MeshPassBase::MeshPassBase() {
	skyBoxTexture_ = TextureManager::LoadTextureFile("resources/engine/Texture/rostock_laage_airport_4k.dds");
}

void MeshPassBase::EnsurePSOsInitialized() {
	if (!psoIDs_.empty()) return;

	auto& context = *GetRenderContext();
	psoIDs_[RenderMode::kDefault][BlendMode::kNormal] = context.GetPSOID("Renderer : Default PSO");
	psoIDs_[RenderMode::kDefault][BlendMode::kAlpha] = context.GetPSOID("Renderer : Default Alpha PSO");
	psoIDs_[RenderMode::kDefault][BlendMode::kAdd] = context.GetPSOID("Renderer : Default Add PSO");
	psoIDs_[RenderMode::kDefault][BlendMode::kSubtract] = context.GetPSOID("Renderer : Default Sub PSO");
	psoIDs_[RenderMode::kDefault][BlendMode::kMultiply] = context.GetPSOID("Renderer : Default Mul PSO");
	psoIDs_[RenderMode::kDefault][BlendMode::kScreen] = context.GetPSOID("Renderer : Default Screen PSO");

	psoIDs_[RenderMode::kToon][BlendMode::kNormal] = context.GetPSOID("Renderer : Toon PSO");
	psoIDs_[RenderMode::kToon][BlendMode::kAlpha] = context.GetPSOID("Renderer : Toon Alpha PSO");
	psoIDs_[RenderMode::kToon][BlendMode::kAdd] = context.GetPSOID("Renderer : Toon Add PSO");
	psoIDs_[RenderMode::kToon][BlendMode::kSubtract] = context.GetPSOID("Renderer : Toon Sub PSO");
	psoIDs_[RenderMode::kToon][BlendMode::kMultiply] = context.GetPSOID("Renderer : Toon Mul PSO");
	psoIDs_[RenderMode::kToon][BlendMode::kScreen] = context.GetPSOID("Renderer : Toon Screen PSO");

	psoIDs_[RenderMode::kEmissive][BlendMode::kAdd] = context.GetPSOID("Renderer : Emissive PSO");

	rootSigIDs_[RenderMode::kDefault] = context.GetRootSignatureID("Renderer : Default PSO");
	rootSigIDs_[RenderMode::kToon] = context.GetRootSignatureID("Renderer : Toon PSO");
	rootSigIDs_[RenderMode::kEmissive] = context.GetRootSignatureID("Renderer : Emissive PSO");
}

void MeshPassBase::RenderItems(GraphicsContext& context, const RenderGraphRegistry& resourceRegistry,
	const std::vector<RenderContext::MeshDrawItem>& items, const CameraComponent& camera, ECS::Registry& registry) {
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
		uint32_t targetPSO = psoIDs_[item.material->renderMode][item.blendMode];
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
			m.world = item.animationLocal->MakeAffineMatrix4x4(registry) * item.transform->MakeAffineMatrix4x4(registry);
			m.worldIT = m.world;
		} else {
			m.world = item.transform->MakeAffineMatrix4x4(registry);
			m.worldIT = m.world;
		}

		m.worldIT.Inverse();
		m.worldIT.Transpose();
		context.SetDynamicConstantBufferView(rootIndex["gWorldMatrix"], sizeof(MeshConstants), &m);
		context.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Component::CameraForGPU), &camera.forGPU);

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

				_declspec(align(16)) struct {
					Math::Color color = Math::Color::BLACK;
					Math::Color edgeColor = Math::Color::WHITE;
					float intensity = 0.0f;
					float rimPower = 0.0f;
					float scrollSpeed = 0.0f;
					float time = 0.0f;
				} emissiveConstants{
					item.material->color,
					item.material->edgeColor,
					item.material->emissiveIntensity,
					item.material->rimPower,
					item.material->noiseScrollSpeed,
					GameCore::GetElapsedTime()
				};
				context.SetDynamicConstantBufferView(rootIndex["gEmissive"], sizeof(emissiveConstants), &emissiveConstants);
				if (!item.material->noiseTextureHandle.IsValid() || item.material->loadedNoiseTextureName != item.material->noiseTextureName
					|| item.material->loadedNoiseTextureGeneration != AssetManager::GetAddressableGeneration()) {
					item.material->noiseTextureHandle = TextureManager::LoadCovertTexture(AssetManager::GetFilePathFromAddressableName(item.material->noiseTextureName));
					item.material->loadedNoiseTextureName = item.material->noiseTextureName;
					item.material->loadedNoiseTextureGeneration = AssetManager::GetAddressableGeneration();
				}
				context.SetDynamicDescriptor(rootIndex["gNoiseTexture"], 0, item.material->noiseTextureHandle.GetSRV());
			}

		}
		auto* mesh = ModelSaver::Get().GetMesh(item.meshHandle);
		if (!mesh) continue;
		auto& vertices = item.animator ? item.animator->GetVertexBuffer(*mesh) : mesh->useVertexBuffer;
		context.TransitionResource(vertices, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);
		context.SetVertexBuffer(0, vertices.VertexBufferView());
		context.SetIndexBuffer(mesh->indexBuffer.IndexBufferView());

		for (const auto& subMesh : mesh->subMeshes) {

			if (item.material->renderMode != RenderMode::kEmissive) {
				_declspec(align(16)) struct {
					Math::Color color;
					float shininess;
					float environmentCoefficient;
					float unlit;
					float receiveShadow;
					Math::Matrix4x4 uvTransform;
				}constants;
				constants.color = item.material->color;
				constants.shininess = item.material->shininess;
				constants.environmentCoefficient = item.material->environmentCoefficient;
				constants.unlit = item.material->unlit ? 1.0f : 0.0f;
				constants.receiveShadow = item.material->receiveShadow ? 1.0f : 0.0f;

				// uvTransform
				Math::Quaternion uvRotate;
				uvRotate.FromAxisAngle(Math::Vector3::FORWARD, item.material->uvRotate);
				constants.uvTransform.MakeAffine(
					Math::Vector3(item.material->uvScale.x, item.material->uvScale.y, 1.0f),
					uvRotate,
					Math::Vector3(item.material->uvPosition.x, item.material->uvPosition.y, 0.0f)
				);
				context.SetDynamicConstantBufferView(rootIndex["gMaterial"], sizeof(constants), &constants);
				const auto* material = ModelSaver::Get().GetMaterial(subMesh.material);
				auto textureSRV = material ? material->textureHandle.GetSRV() : Graphics::GetDefaultTexture(Graphics::kWhiteOpaque2D);
				if (!item.material->textureName.empty()) {
					const uint64_t generation = AssetManager::GetAddressableGeneration();
					if (item.material->loadedTextureGeneration != generation ||
						item.material->loadedTextureName != item.material->textureName) {
						const std::string path = AssetManager::GetFilePathFromAddressableName(item.material->textureName);
						item.material->textureHandle = path.empty() ? TextureRef{} : TextureManager::LoadCovertTexture(path);
						item.material->loadedTextureName = item.material->textureName;
						item.material->loadedTextureGeneration = generation;
					}
					if (item.material->textureHandle.IsValid()) textureSRV = item.material->textureHandle.GetSRV();
				}
				context.SetDynamicDescriptor(rootIndex["gTexture"], 0, textureSRV);
				context.SetDynamicDescriptor(rootIndex["gShadowMask"], 0, resourceRegistry.GetColorBuffer(GetInput("ShadowMask")).GetSRV());
				context.SetDynamicDescriptor(rootIndex["gEnvironmentTexture"], 0, skyBoxTexture_.GetSRV());
			}

			context.DrawIndexedInstanced(subMesh.indexCount, 1, subMesh.indexStart, subMesh.vertexStart, 0);
		}
		// 次回のCompute更新へ描画専用の状態を持ち越さない。
		if (item.animator) context.TransitionResource(vertices, D3D12_RESOURCE_STATE_COMMON, true);
	}
}

// =================================== MeshPass ===================================

void MeshPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	const auto* camera = GetTargetCamera();
	if (!camera) return;
	EnsurePSOsInitialized();

	const auto& items = GetRenderContext()->GetMeshDrawLists(registry, *camera).opaque;
	RenderItems(gfx, resourceRegistry, items, *camera, registry);
	RenderOutline(gfx, items, *camera, registry);
}

void MeshPass::RenderOutline(GraphicsContext& context, const std::vector<RenderContext::MeshDrawItem>& items,
	const CameraComponent& camera, ECS::Registry& registry) {
	if (items.empty()) return;
	auto* renderCtx = GetRenderContext();
	outlinePSOName_ = "Renderer : outline PSO";
	uint32_t a = renderCtx->GetRootSignatureID(outlinePSOName_);
	a;
	outlinePSOID_ = renderCtx->GetPSOID(outlinePSOName_);
	uint32_t outlineRootSigID = renderCtx->GetRootSignatureID(outlinePSOName_);

	context.SetPipelineState(renderCtx->GetGraphicsPSO(outlinePSOID_));
	context.SetRootSignature(renderCtx->GetRootSignature(outlineRootSigID));
	context.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (const auto& item : items) {
		if (!item.material->drawOutline) continue;

		auto& rootIndex = RootSignatureBuilder::GetRootIndexMap(outlinePSOName_);

		Math::Matrix4x4 worldData = item.transform->MakeAffineMatrix4x4(registry);
		if (item.animationLocal) {
			worldData = item.animationLocal->MakeAffineMatrix4x4(registry) * worldData;
		}
		context.SetDynamicConstantBufferView(rootIndex["gWorldMatrix"], sizeof(Math::Matrix4x4), &worldData);
		context.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Component::CameraForGPU), &camera.forGPU);

		auto* mesh = ModelSaver::Get().GetMesh(item.meshHandle);
		if (!mesh) continue;
		auto& vertices = item.animator ? item.animator->GetVertexBuffer(*mesh) : mesh->useVertexBuffer;
		context.TransitionResource(vertices, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);
		context.SetVertexBuffer(0, vertices.VertexBufferView());
		context.SetIndexBuffer(mesh->indexBuffer.IndexBufferView());

		for (const auto& subMesh : mesh->subMeshes) {
			context.DrawIndexedInstanced(subMesh.indexCount, 1, subMesh.indexStart, subMesh.vertexStart, 0);
		}
		if (item.animator) context.TransitionResource(vertices, D3D12_RESOURCE_STATE_COMMON, true);
	}
}

}
}
