#include "stdafx.h"
#include "BLASUpdatePass.h"

#include "engine/Functions/ECS/Component/Asset/MeshComponent.h"
#include "engine/Functions/ECS/Component/Asset/AnimatorComponent.h"
#include  "engine/Assets/Model/ModelSaver.h"


#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
namespace Render {
using namespace Component;
void BLASUpdatePass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	if (!GraphicsCore::IsEnableRaytracing()) return;
	for (auto e : registry.View<MeshComponent, AnimatorComponent>()) {
		auto* animeComp = registry.GetComponent<AnimatorComponent>(e);
		if (!animeComp->enableSkinning) {
			continue;
		}

		auto* meshComp = registry.GetComponent<MeshComponent>(e);
		auto& modelSaver = ModelSaver::Get();
		auto* mesh = modelSaver.GetMesh(meshComp->handle);
		if (mesh == nullptr || !animeComp->runtime.skinning) {
			continue;
		}
		auto& skinning = *animeComp->runtime.skinning;
		auto& rtMesh = skinning.raytracingMesh;
		if (!rtMesh) {
			rtMesh = std::make_unique<RaytracingMesh>(ModelLoader::ProcessRaytracingMesh(*mesh, &skinning.vertexBuffer));
			continue;
		}
		// 共有中は保存済みの描画結果なので、BLASも書き換えない。
		if (animeComp->runtime.skinning.use_count() > 1) continue;
		gfx.TransitionResource(skinning.vertexBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, true);
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
		buildDesc.Inputs = rtMesh->inputs;
		buildDesc.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;

		buildDesc.SourceAccelerationStructureData = rtMesh->blas->GetGPUVirtualAddress(); // in-place更新
		buildDesc.DestAccelerationStructureData = rtMesh->blas->GetGPUVirtualAddress();
		buildDesc.ScratchAccelerationStructureData = rtMesh->updateScratch->GetGPUVirtualAddress();

		// 個体ごとの更新を同じ描画コマンド列に積み、TLASからの参照前に完了させる。
		// 更新用バッファはCOMMONで作成し、使用する区間だけUAVに遷移する。
		auto scratchBarrier = CD3DX12_RESOURCE_BARRIER::Transition(rtMesh->updateScratch.Get(),
			D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		gfx.GetCommandList()->ResourceBarrier(1, &scratchBarrier);
		gfx.GetCommandList()->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);
		auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(rtMesh->blas.Get());
		gfx.GetCommandList()->ResourceBarrier(1, &barrier);
		scratchBarrier = CD3DX12_RESOURCE_BARRIER::Transition(rtMesh->updateScratch.Get(),
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COMMON);
		gfx.GetCommandList()->ResourceBarrier(1, &scratchBarrier);
	}
	static_cast<void>(gfx);
	static_cast<void>(resourceRegistry);
}
}
}
