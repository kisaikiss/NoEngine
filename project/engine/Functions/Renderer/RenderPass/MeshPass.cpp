#include "MeshPass.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Functions/Renderer/RenderSystem.h"

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
		float distance = MathCalculations::LengthSquared(transform->translate - cameraPos);

		items_.push_back({ mesh,material,transform, distance });
	}
}

void MeshPass::Sort() {
	std::sort(items_.begin(), items_.end(),
		[](const DrawItem& a, const DrawItem& b) {
			if(a.material->pso != b.material->pso) return a.material->pso < b.material->pso;
			if (a.material->textureHandle.Get() != b.material->textureHandle.Get()) return a.material->textureHandle < b.material->textureHandle;
			return a.distanceToCamera < b.distanceToCamera;
		});
}

void MeshPass::Render(GraphicsContext& context) {
	GraphicsPSO* currentPSO = nullptr;
	MaterialComponent* currentMaterial = nullptr;
	
	// ToDo : これでは一つの特定のルートシグネチャしか利用できません。pso切り替え時に別のルートシグネチャをセットする場合は別のrootIndexMapを取得できるようにすべきです。
	std::unordered_map<std::string, uint32_t>& rootIndex = RootSignatureBuilder::GetRootIndexMap("defaultRootSignature");
	for (auto& item : items_) {

		if (item.material->pso != currentPSO) {
			context.SetPipelineState(*item.material->pso);
			context.SetRootSignature(GetRootSignature("defaultRootSignature"));
			context.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			currentPSO = item.material->pso;
		}

		if (item.material != currentMaterial) {
			context.SetDynamicDescriptor(rootIndex["gTexture"], 0, item.material->textureHandle->GetSRV());
			context.SetDynamicConstantBufferView(rootIndex["gMaterial"], sizeof(Color), &item.material->color);
			currentMaterial = item.material;
		}

		Matrix4x4 worldData = item.transform->MakeAffineMatrix4x4();
		context.SetDynamicConstantBufferView(rootIndex["gWorldMatrix"], sizeof(Matrix4x4), &worldData);
		context.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Matrix4x4), &GetCamera()->GetViewProjMatrix());
		context.SetVertexBuffer(0, item.mesh->mesh->vertexBuffer.VertexBufferView(0, sizeof(Vertex) * static_cast<uint32_t>(item.mesh->mesh->vertices.size()), sizeof(Vertex)));

		context.Draw(UINT(item.mesh->mesh->vertices.size()));
	}
}

}
}