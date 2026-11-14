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
	CameraComponent* camera = GetTargetCamera();
	if (camera == nullptr) return;

	// 本描画と同じ分類を使い、半透明のサブメッシュを影計算用の情報に書き込まない。
	const auto& items = renderCtx->GetMeshDrawLists(registry, *camera).opaque;

	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : PreRender PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : PreRender PSO"));
	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : PreRender PSO");
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (const auto& item : items) {


		struct MeshConstants {
			Math::Matrix4x4 world;
			Math::Matrix4x4 worldIT;
		};
		MeshConstants m;
		m.world = item.world;
		m.worldIT = m.world;
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

		auto* mesh = item.mesh;
		auto& vertices = item.animator ? item.animator->GetVertexBuffer(*mesh) : mesh->useVertexBuffer;
		gfx.TransitionResource(vertices, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);
		gfx.SetVertexBuffer(0, vertices.VertexBufferView());
		gfx.SetIndexBuffer(mesh->indexBuffer.IndexBufferView());
		for (const auto& subMesh : item.subMeshes) {
			gfx.DrawIndexedInstanced(subMesh.indexCount, 1, subMesh.indexStart, subMesh.vertexStart, 0);
		}
		// 次回のCompute更新へ描画専用の状態を持ち越さない。
		if (item.animator) gfx.TransitionResource(vertices, D3D12_RESOURCE_STATE_COMMON, true);
	}
}
}

}
