#include "TLASBuildPass.h"

#include "engine/Functions/ECS/Component/Asset/MeshComponent.h"
#include "engine/Functions/ECS/Component/Asset/LightComponent.h"
#include "engine/Runtime/GraphicsCore.h"

#include  "engine/Assets/Model/ModelSaver.h"

namespace NoEngine {
namespace Render {

namespace {
// bit0: 「通常のシャドウキャスターとして扱われるか」を表すビット
constexpr UINT8 kInstanceMaskShadowCasterBit = 0x01;

// 通常のメッシュ：全ビットON（シャドウレイにも他の用途にも常にヒットする）
constexpr UINT8 kInstanceMaskDefault = 0xFF;

// マテリアルでシャドウオフ、または発光体オブジェクトの場合：シャドウキャスタービットだけOFF
constexpr UINT8 kInstanceMaskShadowOff = kInstanceMaskDefault & ~kInstanceMaskShadowCasterBit; // 0xFE
}

void TLASBuildPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(resourceRegistry);
	if (!GraphicsCore::IsEnableRaytracing()) return;
	BuildRaytracingInstances(registry);
	BuildTLAS(gfx);
}

void TLASBuildPass::BuildRaytracingInstances(ECS::Registry& registry) {
	instances_.clear();

	auto view = registry.View<Component::MeshComponent, Component::MaterialComponent, Component::TransformComponent>();

	for (auto entity : view) {
		auto* meshComp = registry.GetComponent<Component::MeshComponent>(entity);
		auto* transform = registry.GetComponent<Component::TransformComponent>(entity);

		auto* mesh = ModelSaver::Get().GetMesh(meshComp->handle);
		if (!mesh) continue;
		auto* animator = registry.GetComponent<Component::AnimatorComponent>(entity);
		auto* rtMesh = animator ? animator->GetRaytracingMesh(*mesh) : mesh->raytracingMesh.get();
		if (!rtMesh) continue;

		D3D12_RAYTRACING_INSTANCE_DESC desc{};

		// このエンティティが自身にPoint/SpotLightを持っているか(=発光体メッシュか)を判定
		const bool isShadowOff =
			registry.Has<Component::PointLightComponent>(entity) ||
			registry.Has<Component::SpotLightComponent>(entity) ||
			!registry.GetComponent<Component::MaterialComponent>(entity)->castShadow;

		desc.InstanceMask = isShadowOff ? kInstanceMaskShadowOff : kInstanceMaskDefault;
		desc.InstanceID = static_cast<UINT>(entity);
		desc.InstanceContributionToHitGroupIndex = 0;

		FillInstanceTransform(desc, transform, animator, registry);
		desc.AccelerationStructure = rtMesh->blas->GetGPUVirtualAddress();

		instances_.push_back(desc);
	}

}

void TLASBuildPass::BuildTLAS(GraphicsContext& gfx) {
	if (instances_.empty()) return;

	auto* renderContext = GetRenderContext();
	auto& tlas = renderContext->GetTLAS();

	const size_t instanceBufferSize = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instances_.size();
	const auto upload = gfx.ReserveUploadMemory(instanceBufferSize);
	std::memcpy(upload.DataPtr, instances_.data(), instanceBufferSize);

	// TLAS inputs
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	inputs.NumDescs = static_cast<UINT>(instances_.size());
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.InstanceDescs = upload.GpuAddress;

	// 同じ設定では、照会済みのインスタンス数以下のビルドにも確保済みの容量を使える。
	if (inputs.NumDescs > instanceCapacity_ || !tlas) {
		auto* device = GraphicsCore::sGraphicsDevice->GetDevice();
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild{};
		device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &prebuild);
		if (prebuild.ResultDataMaxSizeInBytes == 0) {
			tlas.Reset();
			LogError("TLAS の必要容量を取得できませんでした。");
			return;
		}

		if (!tlasScratch_ || tlasScratch_->GetDesc().Width < prebuild.ScratchDataSizeInBytes) {
			const auto scratchDesc = CD3DX12_RESOURCE_DESC::Buffer(prebuild.ScratchDataSizeInBytes,
				D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
			const auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			if (FAILED(device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE,
				&scratchDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&tlasScratch_)))) {
				tlas.Reset();
				LogError("TLAS のスクラッチを確保できませんでした。");
				return;
			}
		}

		if (FAILED(renderContext->EnsureTLASCapacity(prebuild.ResultDataMaxSizeInBytes))) {
			LogError("TLAS の結果バッファを確保できませんでした。");
			return;
		}
		instanceCapacity_ = inputs.NumDescs;
	}

	// Build TLAS
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC build{};
	build.Inputs = inputs;
	build.ScratchAccelerationStructureData = tlasScratch_->GetGPUVirtualAddress();
	build.DestAccelerationStructureData = tlas->GetGPUVirtualAddress();

	// BLAS 更新と同じコマンドリストに記録し、後続パスからの TLAS 参照を同期する。
	auto* commandList = gfx.GetCommandList();
	// スクラッチはフレームのコマンド実行完了時に COMMON に戻る。
	const auto scratchBarrier = CD3DX12_RESOURCE_BARRIER::Transition(tlasScratch_.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	commandList->ResourceBarrier(1, &scratchBarrier);
	commandList->BuildRaytracingAccelerationStructure(&build, 0, nullptr);
	const auto tlasBarrier = CD3DX12_RESOURCE_BARRIER::UAV(tlas.Get());
	commandList->ResourceBarrier(1, &tlasBarrier);
	renderContext->MarkTLASReady();
}

void TLASBuildPass::FillInstanceTransform(D3D12_RAYTRACING_INSTANCE_DESC& instance, const Component::TransformComponent* transform, Component::AnimatorComponent* animator, ECS::Registry& registry) {
	Math::Matrix4x4 t = transform->MakeAffineMatrix4x4(registry);
	if (animator) t = animator->local.MakeAffineMatrix4x4() * t;
	t.Transpose();

	// Matrix4x4 を Matrix3x4へ設定するために Transpose したものをコピーする。
	std::memcpy(instance.Transform, &t, sizeof(instance.Transform));
}

}
}
