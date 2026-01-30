#include "MeshPass.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Functions/Renderer/RenderSystem.h"
#include "engine/Utilities/Conversion/ConvertString.h"

namespace NoEngine {
namespace Render {

using namespace Component;

void MeshPass::Execute(GraphicsContext& gfx, ECS::Registry& registry) {
	Collect(registry);
	Sort();
	Render(gfx);
}

void MeshPass::Collect(ECS::Registry& registry) {
	auto view = registry.View<
		TransformComponent,
		MeshComponent,
		MaterialComponent
	>();

	items_.clear();

	const Vector3& cameraPos = GetCamera()->GetTransform().translate;
	for (auto entity : view) {
		auto* mesh = registry.GetComponent<MeshComponent>(entity);
		auto* material = registry.GetComponent<MaterialComponent>(entity);
		auto* transform = registry.GetComponent<TransformComponent>(entity);
		auto pso = material->psoId;
		auto rootSig = material->rootSigId;
		auto name = material->psoName;
		float distance = MathCalculations::LengthSquared(transform->translate - cameraPos);

		items_.push_back({ mesh,material,transform, pso, rootSig, ConvertString(name), distance});
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
	MaterialComponent* currentMaterial = nullptr;

	
	for (auto& item : items_) {
		auto& rootIndex = RootSignatureBuilder::GetRootIndexMap(item.psoName);
		if (item.psoId != currentPSO) {
			context.SetPipelineState(GetPSO(item.psoId));
			context.SetRootSignature(GetRootSignature(item.rootSigId));
			context.SetDynamicDescriptor(rootIndex["gDirectionalLights"], 0, GetRenderContext()->GetDirectionalLightSRV());
			context.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			currentPSO = item.psoId;
		}

		Matrix4x4 worldData = item.transform->MakeAffineMatrix4x4();
		context.SetDynamicConstantBufferView(rootIndex["gWorldMatrix"], sizeof(Matrix4x4), &worldData);
		context.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Matrix4x4), &GetCamera()->GetViewProjMatrix());
		{
			_declspec(align(16)) struct {
				uint32_t directionalLightNum;
				uint32_t pointLightNum = 0;
				uint32_t spotLightNum = 0;
				uint32_t pad;
			}constants;
			constants.directionalLightNum = GetRenderContext()->GetLightNums()->directionalLightNum;
			context.SetDynamicConstantBufferView(rootIndex["gLightNums"], sizeof(RenderContext::LightNums), &constants);
		}
		context.SetVertexBuffer(0, item.mesh->mesh->vertexBuffer.VertexBufferView());
		context.SetIndexBuffer(item.mesh->mesh->indexBuffer.IndexBufferView());
		
		if (item.mesh->mesh->numJoints) {

			context.CopyBufferRegion(item.mesh->mesh->paletteResource, 0, item.mesh->mesh->paletteUpload, 0, sizeof(SkeletonWell) * item.mesh->mesh->mappedPalette.size());
			
			context.SetDynamicDescriptor(rootIndex["gJoints"], 0, item.mesh->mesh->paletteResource.GetSRV());
		}

		for (const auto& subMesh : item.mesh->mesh->subMeshes) {
	
			_declspec(align(16)) struct {
				Vector4 color;
			}constants;
			constants.color = item.material->materials[subMesh.materialIndex].color.ToVector4();
			context.SetDynamicConstantBufferView(rootIndex["gMaterial"], sizeof(Color), &constants);
			context.SetDynamicDescriptor(rootIndex["gTexture"], 0, item.material->materials[subMesh.materialIndex].textureHandle.GetSRV());

			currentMaterial = item.material;

			context.DrawIndexedInstanced(subMesh.indexCount, 1, subMesh.indexStart, subMesh.vertexStart, 0);
		}
	}
}

}
}