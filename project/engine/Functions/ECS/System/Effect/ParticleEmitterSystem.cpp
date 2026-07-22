#include "stdafx.h"
#include "ParticleEmitterSystem.h"
#include "engine/Utilities/Random.h"
#include "engine/Assets/AssetManager.h"
#include "../../Component/Common/VelocityComponent.h"
#include "engine/Utilities/Conversion/ConvertString.h"
#include "engine/Functions/Particle/ParticleManager.h"
#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Runtime/GameCore.h"

namespace NoEngine {
namespace ECS {
using namespace Component;
namespace {
const std::wstring sPsoName = L"EmitParticle";
}

ParticleEmitterSystem::ParticleEmitterSystem() {
	ShaderModule initializeCS(ShaderStage::Compute, L"resources/engine/Shaders/Particle/EmitParticle.CS.hlsl", L"cs_6_0");

	const ShaderReflection& csReflection = initializeCS.GetReflection();


	pso_ = ComputePSO(sPsoName);

	pso_.SetComputeShader(initializeCS.GetBytecode());

	std::vector<ShaderReflection> reflections;
	reflections.push_back(csReflection);
	RootSignatureBuilder::BuildFromReflection(reflections, rootSignature_, ConvertString(sPsoName));
	pso_.SetRootSignature(rootSignature_);

	pso_.Finalize();
}

void ParticleEmitterSystem::Update(ComputeContext& ctx, Registry& registry, float deltaTime) {
	auto view = registry.View<ParticleEmitterSphereComponent, TransformComponent>();
	for (auto entity : view) {
		auto* emitter = registry.GetComponent<ParticleEmitterSphereComponent>(entity);
		emitter->frequencyTime += deltaTime;

		if (emitter->frequency <= emitter->frequencyTime) {
			emitter->frequencyTime -= emitter->frequency;
			emitter->emit = 1;
			
		} else {
			emitter->emit = 0;
		}
		ctx.SetPipelineState(pso_);
		ctx.SetRootSignature(rootSignature_);

		auto& rootIndex = RootSignatureBuilder::GetRootIndexMap(ConvertString(sPsoName));
		__declspec(align(16))struct {
			uint32_t count;
			uint32_t emit;
			uint32_t pad[2];
		}emitterConstants;

		emitterConstants.count = emitter->count;
		emitterConstants.emit = emitter->emit;
		ctx.SetDynamicConstantBufferView(rootIndex["gEmitter"], sizeof(emitterConstants), &emitterConstants);

		__declspec(align(16))struct {
			float time;
			float deltaTime;
			float pad[2];
		}timeConstants;
		timeConstants.time = GameCore::GetElapsedTime();
		timeConstants.deltaTime = deltaTime;

		ctx.SetDynamicConstantBufferView(rootIndex["gPerFrame"], sizeof(timeConstants), &timeConstants);

		auto& freeCounter = ParticleManager::GetFreeCounterBuffer();
		ctx.SetDynamicDescriptor(rootIndex["gFreeCounter"], 0, freeCounter.GetUAV());

		auto& particleBuffer = ParticleManager::GetParticleBuffer();
		ctx.TransitionResource(particleBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		ctx.SetDynamicDescriptor(rootIndex["gParticles"], 0, particleBuffer.GetUAV());
		ctx.Dispatch(1, 1, 1);
		ctx.InsertUAVBarrier(particleBuffer);
		ctx.InsertUAVBarrier(freeCounter);
		


	}
	Update(registry, deltaTime);
}

void ParticleEmitterSystem::Update(Registry& registry, float deltaTime) {
	auto view = registry.View<ParticleEmitterComponent, TransformComponent>();
	for (auto entity : view) {
		auto* emitter = registry.GetComponent<ParticleEmitterComponent>(entity);
		if (!emitter->active) {
			continue;
		}

		auto* transform = registry.GetComponent<TransformComponent>(entity);
		TransformComponent t = *transform;
		emitter->frequencyTime += deltaTime;
		if (!emitter->texture.IsValid()) {
			emitter->texture = TextureManager::LoadCovertTexture(AssetManager::GetFilePathFromAddressableName(emitter->textureName));
		}

		if (emitter->frequency <= emitter->frequencyTime) {
				EmitParticle(registry, t, emitter);
				emitter->frequencyTime -= emitter->frequency;
		}
	}
}

void ParticleEmitterSystem::EmitParticle(Registry& registry, const Component::TransformComponent& emitterTransform, Component::ParticleEmitterComponent* emitter) {
	for (uint32_t count = 0; count < emitter->count; count++) {
		auto e = registry.GenerateEntity();
		auto* particle = registry.AddComponent<ParticleComponent>(e);
		particle->lifeTime = Random::GetRandomVal(emitter->minLifeTime, emitter->maxLifeTime);
		particle->texture = emitter->texture;
		particle->shape = emitter->shape;
		particle->color = emitter->color;
		registry.AddComponent<VelocityComponent>(e)->linear = Random::GetRandomVal(emitter->minSpeed, emitter->maxSpeed);
		auto* transform = registry.AddComponent<TransformComponent>(e);
		transform->translate = GetNewPosition(emitterTransform, emitter);
		transform->rotation = Math::Quaternion::IDENTITY; 
		transform->scale = Random::GetRandomVal(emitter->minScale, emitter->maxScale);
	}
}

Math::Vector3 ParticleEmitterSystem::GetNewPosition(const Component::TransformComponent& emitterTransform, Component::ParticleEmitterComponent* emitter) {
	Math::Vector3 min, max;
	max = emitterTransform.translate + (emitter->emitRange / 2.f);
	min = emitterTransform.translate - (emitter->emitRange / 2.f);
	Math::Vector3 result;
	result = Random::GetRandomVal(min, max) + emitter->localPosition;
	return result;
}


}
}