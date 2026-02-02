#include "MeshPass.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Functions/Renderer/RenderSystem.h"
#include "engine/Utilities/Conversion/ConvertString.h"

namespace NoEngine {
namespace Render {

using namespace Component;

MeshPass::MeshPass() {
	outlinePSOName_ = "Renderer : outline PSO";
	outlinePSOID_ = GetPSOID(ConvertString(outlinePSOName_));
	outlineSkinnedPSOName_ = "Renderer : skinnedOutline PSO";
	outlineSkinnedPSOID_ = GetPSOID(ConvertString(outlineSkinnedPSOName_));
}

void MeshPass::Execute(GraphicsContext& gfx, ECS::Registry& registry) {
	Collect(registry);
	Sort();
	Render(gfx);
	RenderOutline(gfx);
}

void MeshPass::Collect(ECS::Registry& registry) {
	auto view = registry.View<
		TransformComponent,
		MeshComponent,
		MaterialComponent
	>();
	if (view.Empty()) return;
	items_.clear();

	const Vector3& cameraPos = GetCamera()->GetTransform().translate;
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

void MeshPass::Render(GraphicsContext& context) {
	// ToDo : currentPsoの値は被りえない値にすべきです。
	uint32_t currentPSO = 110;

	for (auto& item : items_) {
		auto& rootIndex = RootSignatureBuilder::GetRootIndexMap(item.psoName);
		if (item.psoId != currentPSO) {
			context.SetPipelineState(GetPSO(item.psoId));
			context.SetRootSignature(GetRootSignature(item.rootSigId));
			context.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			currentPSO = item.psoId;
		}

		Matrix4x4 worldData = item.transform->MakeAffineMatrix4x4();
		context.SetDynamicConstantBufferView(rootIndex["gWorldMatrix"], sizeof(Matrix4x4), &worldData);
		context.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(CameraBase::CameraForGPU), &GetCamera()->GetCameraForGPU());
		context.SetDynamicDescriptor(rootIndex["gDirectionalLights"], 0, GetRenderContext()->GetDirectionalLightSRV());
		{
			_declspec(align(16)) struct {
				uint32_t directionalLightNum;
				uint32_t pointLightNum = 0;
				uint32_t spotLightNum = 0;
				uint32_t pad;
			}constants;
			constants.directionalLightNum = GetRenderContext()->GetLightNums()->directionalLightNum;
			context.SetDynamicConstantBufferView(rootIndex["gLightNums"], sizeof(constants), &constants);
		}
		context.SetVertexBuffer(0, item.mesh->mesh->vertexBuffer.VertexBufferView());
		context.SetIndexBuffer(item.mesh->mesh->indexBuffer.IndexBufferView());

		if (item.mesh->mesh->numJoints && item.material->enableSkinning) {

			context.CopyBufferRegion(item.mesh->mesh->paletteResource, 0, item.mesh->mesh->paletteUpload, 0, sizeof(SkeletonWell) * item.mesh->mesh->mappedPalette.size());

			context.SetDynamicDescriptor(rootIndex["gJoints"], 0, item.mesh->mesh->paletteResource.GetSRV());
		}

		for (const auto& subMesh : item.mesh->mesh->subMeshes) {

			_declspec(align(16)) struct {
				Color color;
			}constants;
			constants.color = item.material->color;
			context.SetDynamicConstantBufferView(rootIndex["gMaterial"], sizeof(constants), &constants);
			context.SetDynamicDescriptor(rootIndex["gTexture"], 0, item.material->materials[subMesh.materialIndex].textureHandle.GetSRV());

			context.DrawIndexedInstanced(subMesh.indexCount, 1, subMesh.indexStart, subMesh.vertexStart, 0);
		}
	}
}

void MeshPass::RenderOutline(GraphicsContext& context) {
	if (items_.empty()) return;
	bool currentPSOEnableSkinning = false;
	std::string currentPSOName = outlinePSOName_;
	context.SetPipelineState(GetPSO(GetPSOID(ConvertString(outlinePSOName_))));
	context.SetRootSignature(GetRootSignature(GetPSOID(ConvertString(outlinePSOName_))));
	context.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto& item : items_) {
		if (!item.material->drawOutline) continue;

		if (currentPSOEnableSkinning != item.material->enableSkinning) {
			currentPSOEnableSkinning = item.material->enableSkinning;
			if (currentPSOEnableSkinning) {
				currentPSOName = outlineSkinnedPSOName_;
				context.SetPipelineState(GetPSO(GetPSOID(ConvertString(currentPSOName))));
				context.SetRootSignature(GetRootSignature(GetPSOID(ConvertString(currentPSOName))));
				context.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			} else {
				currentPSOName = outlinePSOName_;
				context.SetPipelineState(GetPSO(GetPSOID(ConvertString(outlinePSOName_))));
				context.SetRootSignature(GetRootSignature(GetPSOID(ConvertString(outlinePSOName_))));
				context.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			}
		}
	
		auto& rootIndex = RootSignatureBuilder::GetRootIndexMap(currentPSOName);

		Matrix4x4 worldData = item.transform->MakeAffineMatrix4x4();
		context.SetDynamicConstantBufferView(rootIndex["gWorldMatrix"], sizeof(Matrix4x4), &worldData);
		context.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(CameraBase::CameraForGPU), &GetCamera()->GetCameraForGPU());
		
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