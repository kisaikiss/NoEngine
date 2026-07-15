#include "stdafx.h"
#include "ParticleManager.h"
#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Runtime/PipelineStateObject/RootSignature.h"
#include "engine/Runtime/PipelineStateObject/ComputePSO.h"
#include "engine/Utilities/Conversion/ConvertString.h"

namespace NoEngine {
namespace {
StructuredBuffer sParticleBuffer;
StructuredBuffer sFreeCounterBuffer;
ComputePSO sPSO;
RootSignature sRootSignature;
const std::wstring sPsoName = L"InitializeParticle";
constexpr uint32_t sMaxParticle = 1024;
}

void ParticleManager::Initialize(ComputeContext& ctx) {
	ShaderModule initializeCS(ShaderStage::Compute, L"resources/engine/Shaders/Particle/InitializeParticle.CS.hlsl", L"cs_6_0");

	const ShaderReflection& csReflection = initializeCS.GetReflection();


	sPSO = ComputePSO(sPsoName);

	sPSO.SetComputeShader(initializeCS.GetBytecode());

	std::vector<ShaderReflection> reflections;
	reflections.push_back(csReflection);
	RootSignatureBuilder::BuildFromReflection(reflections, sRootSignature, ConvertString(sPsoName));
	sPSO.SetRootSignature(sRootSignature);

	sPSO.Finalize();

	ctx.SetPipelineState(sPSO);
	ctx.SetRootSignature(sRootSignature);

	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap(ConvertString(sPsoName));

	sParticleBuffer.Create(L"ParticleBuffer", sMaxParticle, sizeof(Particle));
	ctx.TransitionResource(sParticleBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	ctx.SetDynamicDescriptor(rootIndex["gParticles"], 0, sParticleBuffer.GetUAV());


	sFreeCounterBuffer.Create(L"FreeCounterBuffer", 1, sizeof(int));
	ctx.TransitionResource(sFreeCounterBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	ctx.SetDynamicDescriptor(rootIndex["gFreeCounter"], 0, sFreeCounterBuffer.GetUAV());

	ctx.Dispatch(1, 1, 1);
	ctx.InsertUAVBarrier(sParticleBuffer);
	ctx.InsertUAVBarrier(sFreeCounterBuffer);

}

void ParticleManager::Reset() {

}

void ParticleManager::Shutdown() {
	sParticleBuffer.Destroy();
	sFreeCounterBuffer.Destroy();
}

StructuredBuffer& ParticleManager::GetParticleBuffer() {
	return sParticleBuffer;
}

StructuredBuffer& ParticleManager::GetFreeCounterBuffer() {
	return sFreeCounterBuffer;
}

uint32_t ParticleManager::GetParticleNum() {
	return sMaxParticle;
}
}