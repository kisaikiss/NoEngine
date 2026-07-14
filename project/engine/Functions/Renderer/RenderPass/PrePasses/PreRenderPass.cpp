#include "stdafx.h"
#include "PreRenderPass.h"

#include "engine/Runtime/GraphicsCore.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Functions/ECS/Component/Asset/AnimatorComponent.h"
#include "engine/Assets/Model/ModelSaver.h"

namespace NoEngine {
namespace Render {

using namespace Component;

PreRenderPass::PreRenderPass() {}

void PreRenderPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	(void)resourceRegistry;
	auto* renderCtx = GetRenderContext();
	items_.clear();

	auto view = registry.View<
		TransformComponent,
		MeshComponent,
		MaterialComponent
	>();

	for (auto entity : view) {
		auto* mesh = registry.GetComponent<MeshComponent>(entity);
		if (!mesh->isVisible) continue;
		auto* material = registry.GetComponent<MaterialComponent>(entity);

		// 半透明オブジェクトは深度プリパスの対象から除外する。
		// ここで深度を確定させると、背後の不透明オブジェクトが
		// MeshPassの不透明描画時に深度テストで弾かれてしまう。
		//if (material->blendMode != BlendMode::kNormal || material->color.a < 1.0f) continue;

		auto* transform = registry.GetComponent<TransformComponent>(entity);
		auto* anime = registry.GetComponent<AnimatorComponent>(entity);
		Transform* animeLocal = nullptr;
		if (anime) {
			animeLocal = &anime->local;
		}

		items_.push_back({ mesh->handle, material, transform, animeLocal, static_cast<uint32_t>(entity) });
	}

	CameraComponent* camera = GetTargetCamera();
	if (camera == nullptr) return;

	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : PreRender PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : PreRender PSO"));
	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : PreRender PSO");
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (auto& item : items_) {


		struct MeshConstants {
			Math::Matrix4x4 world;
			Math::Matrix4x4 worldIT;
		};
		MeshConstants m;
		if (item.local) {
			m.world = item.local->MakeAffineMatrix4x4(registry) * item.transform->MakeAffineMatrix4x4(registry);
			m.worldIT = m.world;
		} else {
			m.world = item.transform->MakeAffineMatrix4x4(registry);
			m.worldIT = m.world;
		}
		m.worldIT.Inverse();
		m.worldIT.Transpose();
		gfx.SetDynamicConstantBufferView(rootIndex["gWorldMatrix"], sizeof(MeshConstants), &m);
		gfx.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Component::CameraForGPU), &camera->forGPU);
		__declspec(align(16))struct {
			float id[4];
		}constants;
		Math::Color color = Math::Color(item.id);
		constants.id[0] = color.r;
		constants.id[1] = color.g;
		constants.id[2] = color.b;
		constants.id[3] = color.a;

		gfx.SetDynamicConstantBufferView(rootIndex["gObjectID"], sizeof(constants), &constants);

		auto* mesh = ModelSaver::Get().GetMesh(item.mesh);
		if (!mesh) continue;
		gfx.SetVertexBuffer(0, mesh->useVertexBuffer.VertexBufferView());
		gfx.SetIndexBuffer(mesh->indexBuffer.IndexBufferView());
		for (const auto& subMesh : mesh->subMeshes) {
			gfx.DrawIndexedInstanced(subMesh.indexCount, 1, subMesh.indexStart, subMesh.vertexStart, 0);
		}
	}
}
}

}
