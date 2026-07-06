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
		if (mesh == nullptr || mesh->numJoints == 0) {
			continue;
		}
		auto& rtMesh = mesh->raytracingMesh;
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc = {};
		buildDesc.Inputs = rtMesh->inputs;
		buildDesc.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;

		buildDesc.SourceAccelerationStructureData = rtMesh->blas->GetGPUVirtualAddress(); // in-place更新
		buildDesc.DestAccelerationStructureData = rtMesh->blas->GetGPUVirtualAddress();
		buildDesc.ScratchAccelerationStructureData = rtMesh->updateScratch->GetGPUVirtualAddress();

		CommandContext::BuildRaytracingAccelerationStructure(buildDesc, rtMesh->updateScratch.Get());
	}
	static_cast<void>(gfx);
	static_cast<void>(resourceRegistry);
}
}
}