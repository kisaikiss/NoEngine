#include "TLASBuildPass.h"

#include "engine/Functions/ECS/Component/MeshComponent.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
namespace Render {
void TLASBuildPass::Execute(GraphicsContext& gfx, ECS::Registry& registry) {
	if (!GraphicsCore::IsEnableRaytracing()) return;
	BuildRaytracingInstances(registry);
	BuildTLAS(gfx);
}

void TLASBuildPass::BuildRaytracingInstances(ECS::Registry& registry) {
	instances_.clear();

	auto view = registry.View<Component::MeshComponent, Component::TransformComponent>();

	for (auto entity : view) {
		auto* meshComp = registry.GetComponent<Component::MeshComponent>(entity);
		auto* transform = registry.GetComponent<Component::TransformComponent>(entity);
		if (!meshComp || !transform || !meshComp->mesh || !meshComp->mesh->raytracingMesh) continue;

		RaytracingInstance inst = {};
		inst.rtMesh = meshComp->mesh->raytracingMesh.get();

		auto& desc = inst.desc;
		memset(&desc, 0, sizeof(desc));

		desc.InstanceMask = 0xFF;
		desc.InstanceID = static_cast<UINT>(entity); 
		desc.InstanceContributionToHitGroupIndex = 0;

		FillInstanceTransform(desc, *transform);
		desc.AccelerationStructure = inst.rtMesh->blas->GetGPUVirtualAddress();

		instances_.push_back(inst);
	}

}

void TLASBuildPass::BuildTLAS(GraphicsContext& gfx) {
	if (instances_.empty()) return;

	auto* renderContext = GetRenderContext();

	// インスタンスバッファ
	const UINT instanceBufferSize = UINT(sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instances_.size());

	auto instDesc = CD3DX12_RESOURCE_DESC::Buffer(instanceBufferSize);
	D3D12_HEAP_PROPERTIES uploadHeap{};
	uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

	renderContext->CreateRaytraceInstanceBuffer(uploadHeap, instDesc);

	auto& instanceBuffer = renderContext->GetRaytraceInstanceBuffer();

	// マップしてコピー
	{
		void* mapped = nullptr;
		instanceBuffer->Map(0, nullptr, &mapped);
		auto* dst = reinterpret_cast<D3D12_RAYTRACING_INSTANCE_DESC*>(mapped);
		for (size_t i = 0; i < instances_.size(); ++i) {
			dst[i] = instances_[i].desc;
		}
		instanceBuffer->Unmap(0, nullptr);
	}

	// TLAS inputs
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	inputs.NumDescs = (UINT)instances_.size();
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.InstanceDescs = instanceBuffer->GetGPUVirtualAddress();

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild{};
	GraphicsCore::sGraphicsDevice->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuild);

	// TLAS / Scratch
	auto tlasDesc = CD3DX12_RESOURCE_DESC::Buffer(prebuild.ResultDataMaxSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	auto scratchDesc = CD3DX12_RESOURCE_DESC::Buffer(prebuild.ScratchDataSizeInBytes,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

	D3D12_HEAP_PROPERTIES defaultHeap{};
	defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

	renderContext->CreateTLAS(tlasDesc);

	Microsoft::WRL::ComPtr<ID3D12Resource> tlasScratch;
	GraphicsCore::sGraphicsDevice->GetDevice()->CreateCommittedResource(
		&defaultHeap,
		D3D12_HEAP_FLAG_NONE,
		&scratchDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&tlasScratch)
	);

	// Build TLAS
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build{};
	build.Inputs = inputs;
	build.ScratchAccelerationStructureData = tlasScratch->GetGPUVirtualAddress();
	build.DestAccelerationStructureData = renderContext->GetTLAS()->GetGPUVirtualAddress();

	gfx.BuildRaytracingAccelerationStructure(build, tlasScratch.Get());
}

void TLASBuildPass::FillInstanceTransform(D3D12_RAYTRACING_INSTANCE_DESC& instance, const Transform& transform) {
	Math::Matrix4x4 t = transform.MakeAffineMatrix4x4();
	t.Transpose();

	// Matrix4x4 を Matrix3x4へ設定するために Transpose したものをコピーする。
	std::memcpy(instance.Transform, &t, sizeof(instance.Transform));
}

}
}
