#include "stdafx.h"
#include "ParticleSystem.h"
#include "engine/Utilities/Random.h"
#include "../../Component/Common/VelocityComponent.h"

#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Functions/Particle/ParticleManager.h"
#include "engine/Utilities/Conversion/ConvertString.h"
#include "engine/Runtime/GameCore.h"
namespace NoEngine {
namespace ECS {
using namespace Component;
namespace {
const std::wstring sPsoName = L"ParticleUpdate";
}

ParticleSystem::ParticleSystem() { 
	SetStopInPause(false);

	ShaderModule initializeCS(ShaderStage::Compute, L"resources/engine/Shaders/Particle/UpdateParticle.CS.hlsl", L"cs_6_0");

	const ShaderReflection& csReflection = initializeCS.GetReflection();


	pso_ = ComputePSO(sPsoName);

	pso_.SetComputeShader(initializeCS.GetBytecode());

	std::vector<ShaderReflection> reflections;
	reflections.push_back(csReflection);
	RootSignatureBuilder::BuildFromReflection(reflections, rootSignature_, ConvertString(sPsoName));
	pso_.SetRootSignature(rootSignature_);

	pso_.Finalize();
}

void ParticleSystem::Update(ComputeContext& ctx, Registry& registry, float deltaTime) {
	ctx.SetPipelineState(pso_);
	ctx.SetRootSignature(rootSignature_);

	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap(ConvertString(sPsoName));

	__declspec(align(16))struct {
		float time;
		float deltaTime;
		float pad[2];
	}timeConstants;
	timeConstants.time = GameCore::GetElapsedTime();
	timeConstants.deltaTime = deltaTime;

	ctx.SetDynamicConstantBufferView(rootIndex["gPerFrame"], sizeof(timeConstants), &timeConstants);

	auto& freeIndex = ParticleManager::GetFreeListIndexBuffer();
	ctx.SetDynamicDescriptor(rootIndex["gFreeListIndex"], 0, freeIndex.GetUAV());

	auto& freeList = ParticleManager::GetFreeListBuffer();
	ctx.SetDynamicDescriptor(rootIndex["gFreeList"], 0, freeList.GetUAV());
	auto& particleBuffer = ParticleManager::GetParticleBuffer();
	ctx.TransitionResource(particleBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	ctx.SetDynamicDescriptor(rootIndex["gParticles"], 0, particleBuffer.GetUAV());
	ctx.Dispatch(1, 1, 1);
	ctx.InsertUAVBarrier(particleBuffer);
	ctx.InsertUAVBarrier(freeIndex);
	ctx.InsertUAVBarrier(freeList);

	Update(registry, deltaTime);
}

void ParticleSystem::Update(Registry& registry, float deltaTime) {
	auto particleView = registry.View<ParticleComponent, TransformComponent, VelocityComponent>();
	for (auto entity : particleView) {
		auto* particle = registry.GetComponent<ParticleComponent>(entity);
		particle->currentTime += deltaTime;
		particle->color.a = 1.0f - (particle->currentTime / particle->lifeTime);
		if (particle->currentTime >= particle->lifeTime) {
			registry.DestroyEntity(entity);
		}
	}
}
}
}