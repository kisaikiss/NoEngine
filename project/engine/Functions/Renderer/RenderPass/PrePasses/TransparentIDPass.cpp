#include "stdafx.h"
#include "TransparentIDPass.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Functions/ECS/Component/Asset/AnimatorComponent.h"
#include "engine/Assets/Model/ModelSaver.h"

namespace NoEngine {
namespace Render {
using namespace Component;

void TransparentIDPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(resourceRegistry);
    auto* renderCtx = GetRenderContext();
    items_.clear();

    auto view = registry.View<TransformComponent, MeshComponent, MaterialComponent>();
    Math::Vector3 cameraPos = GetTargetCamera()->forGPU.worldPosition;

    for (auto entity : view) {
        auto* mesh = registry.GetComponent<MeshComponent>(entity);
        if (!mesh->isVisible) continue;
        auto* material = registry.GetComponent<MaterialComponent>(entity);
        if (material->blendMode == BlendMode::kNormal || material->color.a >= 1.0f) continue; // 半透明のみ対象

        auto* transform = registry.GetComponent<TransformComponent>(entity);
        auto* anime = registry.GetComponent<AnimatorComponent>(entity);
        Transform* animeLocal = anime ? &anime->local : nullptr;

        float distance = (transform->translate - cameraPos).LengthSquared();
        items_.push_back({ mesh->handle, material, transform, animeLocal,
                            static_cast<uint32_t>(entity), distance });
    }

    // MeshPassの半透明描画と同じく奥→前でソートし、
    // 一番手前にある半透明オブジェクトのIDが最終的にピクセルに残るようにする
    std::sort(items_.begin(), items_.end(),
        [](const auto& a, const auto& b) { return a.distanceToCamera > b.distanceToCamera; });

    CameraComponent* camera = GetTargetCamera();
    if (camera == nullptr) return;

    // "Renderer : Transparent PreRender PSO" は
    // DepthEnable=true, DepthWriteMask=ZERO, DepthFunc=LESS_EQUAL で
    // IDDepthバッファに対してテストのみ行う(不透明の裏にある半透明はIDを書かない)
    gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : Transparent PreRender PSO"));
    gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : Transparent PreRender PSO"));
    auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : Transparent PreRender PSO");
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