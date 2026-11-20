#include "stdafx.h"
#include "PreRenderPass.h"

#include "engine/Runtime/GraphicsCore.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Functions/ECS/Component/Asset/AnimatorComponent.h"
#include "engine/Assets/Model/ModelSaver.h"

namespace NoEngine {
namespace Render {

using namespace Component;

namespace {
void DrawMeshID(GraphicsContext& gfx, const RenderContext::MeshDrawItem& item,
	const std::unordered_map<std::string, uint32_t>& rootIndex, bool sampleAlpha = false) {
	struct MeshConstants {
		Math::Matrix4x4 world;
		Math::Matrix4x4 worldIT;
	};
	MeshConstants matrices{ item.world, item.world };
	matrices.worldIT.Inverse();
	matrices.worldIT.Transpose();
	gfx.SetDynamicConstantBufferView(rootIndex.at("gWorldMatrix"), sizeof(matrices), &matrices);

	alignas(16) const Math::Color id(item.id);
	gfx.SetDynamicConstantBufferView(rootIndex.at("gObjectID"), sizeof(id), &id);

	auto* mesh = item.mesh;
	auto& vertices = item.animator ? item.animator->GetVertexBuffer(*mesh) : mesh->useVertexBuffer;
	gfx.TransitionResource(vertices, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);
	gfx.SetVertexBuffer(0, vertices.VertexBufferView());
	gfx.SetIndexBuffer(mesh->indexBuffer.IndexBufferView());
	for (const auto& subMesh : item.subMeshes) {
		if (sampleAlpha) {
			gfx.SetDynamicDescriptor(rootIndex.at("gTexture"), 0, item.material->ResolveTextureSRV(subMesh.material));
		}
		gfx.DrawIndexedInstanced(subMesh.indexCount, 1, subMesh.indexStart, subMesh.vertexStart, 0);
	}
	// 次回のCompute更新へ描画専用の状態を持ち越さない。
	if (item.animator) gfx.TransitionResource(vertices, D3D12_RESOURCE_STATE_COMMON, true);
}
}

void PreRenderPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	(void)resourceRegistry;
	auto* renderCtx = GetRenderContext();
	CameraComponent* camera = GetTargetCamera();
	if (camera == nullptr) return;

	// 本描画と同じ分類を使い、半透明のサブメッシュを影計算用の情報に書き込まない。
	const auto& items = renderCtx->GetMeshDrawLists(registry, *camera).opaque;

	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : PreRender PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : PreRender PSO"));
	const auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : PreRender PSO");
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.SetDynamicConstantBufferView(rootIndex.at("gCameraMatrix"), sizeof(camera->forGPU), &camera->forGPU);
	for (const auto& item : items) {
		DrawMeshID(gfx, item, rootIndex);
	}
}

void TransparentMeshIDPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry&, ECS::Registry& registry) {
	const auto* camera = GetTargetCamera();
	if (!camera) return;

	auto& renderCtx = *GetRenderContext();
	const auto& items = renderCtx.GetMeshDrawLists(registry, *camera).transparent;
	if (items.empty()) return;

	const char* psoName = "Renderer : Transparent PreRender PSO";
	const auto& rootIndex = RootSignatureBuilder::GetRootIndexMap(psoName);
	const auto& meshPSO = renderCtx.GetGraphicsPSO(psoName);
	const auto& emissivePSO = renderCtx.GetGraphicsPSO("Renderer : Emissive ObjectID PSO");
	gfx.SetRootSignature(renderCtx.GetRootSignature(psoName));
	gfx.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.SetDynamicConstantBufferView(rootIndex.at("gCameraMatrix"), sizeof(camera->forGPU), &camera->forGPU);

	// 本描画と同じく奥から手前へ描き、手前側のIDで上書きする。
	for (const auto& item : items) {
		const bool emissive = item.material->renderMode == RenderMode::kEmissive;
		const bool sampleAlpha = !emissive && (item.blendMode == BlendMode::kAlpha ||
			item.blendMode == BlendMode::kAdd || item.blendMode == BlendMode::kSubtract);
		gfx.SetPipelineState(emissive ? emissivePSO : meshPSO);

		struct PickMaterial {
			Math::Matrix4x4 uvTransform;
			float opacity;
			float padding[3]{};
		};
		const PickMaterial material{
			sampleAlpha ? item.material->MakeUVTransform() : Math::Matrix4x4::IDENTITY,
			sampleAlpha ? item.material->color.a : 1.0f,
		};
		gfx.SetDynamicConstantBufferView(rootIndex.at("gPickMaterial"), sizeof(material), &material);
		if (!sampleAlpha) {
			gfx.SetDynamicDescriptor(rootIndex.at("gTexture"), 0, Graphics::GetDefaultTexture(Graphics::kWhiteOpaque2D));
		}
		DrawMeshID(gfx, item, rootIndex, sampleAlpha);
	}
}
}

}
