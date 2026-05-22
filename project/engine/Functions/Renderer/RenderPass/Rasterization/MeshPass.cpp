#include "MeshPass.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Utilities/Conversion/ConvertString.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
namespace Render {

using namespace Component;

MeshPass::MeshPass() : camera_(nullptr) {
	
	skyBoxTexture_ = TextureManager::LoadTextureFile("resources/engine/Texture/rostock_laage_airport_4k.dds");
}

void MeshPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(resourceRegistry);
	Collect(registry);
	Sort();
	Render(gfx, resourceRegistry);
	RenderOutline(gfx);
}

void MeshPass::Collect(ECS::Registry& registry) {
	auto view = registry.View<
		TransformComponent,
		MeshComponent,
		MaterialComponent
	>();
	items_.clear();

	camera_ = GetTargetCamera();
	if (camera_ == nullptr) return;
	Math::Vector3 cameraPos = camera_->forGPU.worldPosition;

	for (auto entity : view) {
		auto* mesh = registry.GetComponent<MeshComponent>(entity);
		if (!mesh->isVisible)continue;
		auto* material = registry.GetComponent<MaterialComponent>(entity);
		auto* transform = registry.GetComponent<TransformComponent>(entity);
		auto pso = material->psoId;
		auto rootSig = material->rootSigId;
		auto name = material->psoName;
		float distance = MathCalculations::LengthSquared(transform->translate - cameraPos);

		items_.push_back({ mesh,material,transform, pso, rootSig, ConvertString(name), distance });
	}
}

void MeshPass::Sort() {
	std::sort(items_.begin(), items_.end(),
		[](const DrawItem& a, const DrawItem& b) {
			if (a.psoId != b.psoId) return a.psoId < b.psoId;
			return a.distanceToCamera < b.distanceToCamera;
		});
}

void MeshPass::Render(GraphicsContext& context, const RenderGraphRegistry& resourceRegistry) {
	(void)resourceRegistry;
	// ToDo : currentPsoの値は被りえない値にすべきです。
	uint32_t currentPSO = 110;
	auto* renderCtx = GetRenderContext();
	context.TransitionResource(GraphicsCore::GetShadowMask(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	for (auto& item : items_) {
		auto& rootIndex = RootSignatureBuilder::GetRootIndexMap(item.psoName);
		if (item.psoId != currentPSO) {
			context.SetPipelineState(renderCtx->GetGraphicsPSO(ConvertString(item.material->psoName)));
			context.SetRootSignature(renderCtx->GetRootSignature(ConvertString(item.material->psoName)));
			context.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			currentPSO = item.psoId;
		}

		struct MeshConstants {
			Math::Matrix4x4 world;
			Math::Matrix4x4 worldIT;
		};
		MeshConstants m;
		m.world = item.transform->MakeAffineMatrix4x4();
		m.worldIT = m.world;
		m.worldIT.Inverse();
		m.worldIT.Transpose();
		context.SetDynamicConstantBufferView(rootIndex["gWorldMatrix"], sizeof(MeshConstants), &m);
		context.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Component::CameraForGPU), &camera_->forGPU);

		{
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
		}
		context.SetVertexBuffer(0, item.mesh->mesh->vertexBuffer.VertexBufferView());
		context.SetIndexBuffer(item.mesh->mesh->indexBuffer.IndexBufferView());

		if (item.mesh->mesh->numJoints && item.material->enableSkinning) {

			context.CopyBufferRegion(item.mesh->mesh->paletteResource, 0, item.mesh->mesh->paletteUpload, 0, sizeof(SkeletonWell) * item.mesh->mesh->mappedPalette.size());

			context.SetDynamicDescriptor(rootIndex["gJoints"], 0, item.mesh->mesh->paletteResource.GetSRV());
		}

		for (const auto& subMesh : item.mesh->mesh->subMeshes) {

			_declspec(align(16)) struct {
				Math::Color color;
				float shininess;
				float environmentCoefficient;
				float padding[2];
			}constants;
			constants.color = item.material->color;
			constants.shininess = item.material->shininess;
			constants.environmentCoefficient = item.material->enviromentCoefficient;
			context.SetDynamicConstantBufferView(rootIndex["gMaterial"], sizeof(constants), &constants);
			context.SetDynamicDescriptor(rootIndex["gTexture"], 0, item.material->materials[subMesh.materialIndex].textureHandle.GetSRV());
			context.SetDynamicDescriptor(rootIndex["gShadowMask"], 0, GraphicsCore::GetShadowMask().GetSRV());
			context.SetDynamicDescriptor(rootIndex["gEnvironmentTexture"], 0, skyBoxTexture_.GetSRV());
			context.DrawIndexedInstanced(subMesh.indexCount, 1, subMesh.indexStart, subMesh.vertexStart, 0);
		}
	}
}

void MeshPass::RenderOutline(GraphicsContext& context) {
	if (items_.empty()) return;
	auto* renderCtx = GetRenderContext();
	outlinePSOName_ = "Renderer : outline PSO";
	uint32_t a = renderCtx->GetRootSignatureID(outlinePSOName_);
	a;
	outlinePSOID_ = renderCtx->GetPSOID(outlinePSOName_);
	outlineSkinnedPSOName_ = "Renderer : skinnedOutline PSO";
	outlineSkinnedPSOID_ = renderCtx->GetPSOID(outlineSkinnedPSOName_);

	bool currentPSOEnableSkinning = false;
	std::string currentPSOName = outlinePSOName_;
	context.SetPipelineState(renderCtx->GetGraphicsPSO(outlinePSOID_));
	context.SetRootSignature(renderCtx->GetRootSignature(outlinePSOID_));
	context.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto& item : items_) {
		if (!item.material->drawOutline) continue;

		if (currentPSOEnableSkinning != item.material->enableSkinning) {
			currentPSOEnableSkinning = item.material->enableSkinning;
			if (currentPSOEnableSkinning) {
				currentPSOName = outlineSkinnedPSOName_;
				context.SetPipelineState(renderCtx->GetGraphicsPSO(outlineSkinnedPSOID_));
				context.SetRootSignature(renderCtx->GetRootSignature(outlineSkinnedPSOID_));
				context.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			} else {
				currentPSOName = outlinePSOName_;
				context.SetPipelineState(renderCtx->GetGraphicsPSO(outlinePSOID_));
				context.SetRootSignature(renderCtx->GetRootSignature(outlinePSOID_));
				context.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			}
		}

		auto& rootIndex = RootSignatureBuilder::GetRootIndexMap(currentPSOName);

		Math::Matrix4x4 worldData = item.transform->MakeAffineMatrix4x4();
		context.SetDynamicConstantBufferView(rootIndex["gWorldMatrix"], sizeof(Math::Matrix4x4), &worldData);
		context.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Component::CameraForGPU), &camera_->forGPU);

		context.SetVertexBuffer(0, item.mesh->mesh->vertexBuffer.VertexBufferView());
		context.SetIndexBuffer(item.mesh->mesh->indexBuffer.IndexBufferView());

		if (item.mesh->mesh->numJoints && item.material->enableSkinning) {

			context.SetDynamicDescriptor(rootIndex["gJoints"], 0, item.mesh->mesh->paletteResource.GetSRV());
		}

		for (const auto& subMesh : item.mesh->mesh->subMeshes) {
			context.DrawIndexedInstanced(subMesh.indexCount, 1, subMesh.indexStart, subMesh.vertexStart, 0);
		}
	}
}

}
}