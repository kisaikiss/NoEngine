#include "stdafx.h"
#include "PrePass.h"

#include "engine/Runtime/GraphicsCore.h"
#include "engine/Functions/Renderer/RenderSystem.h"
#include "engine/Functions/Shader/ShaderReflection.h"

namespace NoEngine {
namespace Render {

using namespace Component;

namespace {
uint32_t sPSOID;
uint32_t sRootSignatureID;
uint32_t sPSOSkinnedID;
uint32_t sRootSignatureSkinnedID;
}

PrePass::PrePass() {
	sPSOID = GetPSOID(L"Renderer : PreRender PSO");
	sRootSignatureID = GetRootSignatureID(L"Renderer : PreRender PSO");
	sPSOSkinnedID = GetPSOID(L"Renderer : PreRenderSkinned PSO");
	sRootSignatureSkinnedID = GetRootSignatureID(L"Renderer : PreRenderSkinned PSO");
}

void PrePass::Execute(GraphicsContext& gfx, ECS::Registry& registry) {
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViews[] = {
		GraphicsCore::sWorldPositionGBuffer.GetRTV(),
		GraphicsCore::sNormalGBuffer.GetRTV()
	};
	gfx.SetRenderTargets(2, renderTargetViews, GraphicsCore::GetDepth().GetDSV());
	items_.clear();

	auto view = registry.View<
		TransformComponent,
		MeshComponent,
		MaterialComponent
	>();

	for (auto entity : view) {
		auto* mesh = registry.GetComponent<MeshComponent>(entity);
		if (!mesh->isVisible)continue;
		auto* material = registry.GetComponent<MaterialComponent>(entity);
		auto* transform = registry.GetComponent<TransformComponent>(entity);

		items_.push_back({ mesh,material,transform });
	}

	auto cameraView = registry.View<TransformComponent, CameraComponent, ActiveCameraTag>();
	CameraComponent* camera = nullptr;
	Math::Vector3 cameraPos{};
	for (auto entity : cameraView) {
		auto* cameraTransform = registry.GetComponent<TransformComponent>(entity);
		cameraPos = cameraTransform->GetWorldPosition();
		camera = registry.GetComponent<CameraComponent>(entity);
	}

	if (camera == nullptr) assert(false);

	for (auto& item : items_) {
		std::string currentPsoName;
		if (item.material->enableSkinning) {
			gfx.SetPipelineState(GetPSO(sPSOSkinnedID));
			gfx.SetRootSignature(GetRootSignature(sRootSignatureSkinnedID));
			currentPsoName = "Renderer : PreRenderSkinned PSO";
		} else {
			gfx.SetPipelineState(GetPSO(sPSOID));
			gfx.SetRootSignature(GetRootSignature(sRootSignatureID));
			currentPsoName = "Renderer : PreRender PSO";
		}
		auto& rootIndex = RootSignatureBuilder::GetRootIndexMap(currentPsoName);
		gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		struct MeshConstants {
			Math::Matrix4x4 world;
			Math::Matrix4x4 worldIT;
		};
		MeshConstants m;
		m.world = item.transform->MakeAffineMatrix4x4();
		m.worldIT = m.world;
		m.worldIT.Inverse();
		m.worldIT.Transpose();
		gfx.SetDynamicConstantBufferView(rootIndex["gWorldMatrix"], sizeof(MeshConstants), &m);
		gfx.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Component::CameraForGPU), &camera->forGPU);
		gfx.SetVertexBuffer(0, item.mesh->mesh->vertexBuffer.VertexBufferView());
		gfx.SetIndexBuffer(item.mesh->mesh->indexBuffer.IndexBufferView());
		if (item.mesh->mesh->numJoints && item.material->enableSkinning) {

			gfx.CopyBufferRegion(item.mesh->mesh->paletteResource, 0, item.mesh->mesh->paletteUpload, 0, sizeof(SkeletonWell) * item.mesh->mesh->mappedPalette.size());

			gfx.SetDynamicDescriptor(rootIndex["gJoints"], 0, item.mesh->mesh->paletteResource.GetSRV());
		}
		for (const auto& subMesh : item.mesh->mesh->subMeshes) {
			gfx.DrawIndexedInstanced(subMesh.indexCount, 1, subMesh.indexStart, subMesh.vertexStart, 0);
		}
	}
}
}

}
