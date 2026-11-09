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
	auto rootSignature = std::make_shared<RootSignature>();
	RootSignatureBuilder::BuildFromReflection(reflections, *rootSignature, ConvertString(sPsoName));
	pso_.SetRootSignature(*rootSignature);
	rootSignature_ = std::move(rootSignature);

	pso_.Finalize();
}

void ParticleEmitterSystem::Update(ComputeContext& ctx, Registry& registry, float deltaTime) {
	auto view = registry.View<ParticleEmitterSphereComponent, TransformComponent>();
	for (auto entity : view) {
		auto* emitter = registry.GetComponent<ParticleEmitterSphereComponent>(entity);
		if (!emitter->active) continue;

		emitter->frequencyTime += deltaTime;

		if (emitter->frequency <= emitter->frequencyTime) {
			emitter->frequencyTime -= emitter->frequency;
			emitter->emit = 1;
			
		} else {
			emitter->emit = 0;
		}
		ctx.SetPipelineState(pso_);
		ctx.SetRootSignature(*rootSignature_);

		auto& rootIndex = RootSignatureBuilder::GetRootIndexMap(ConvertString(sPsoName));
		auto* transform = registry.GetComponent<TransformComponent>(entity);
		// 親を持つ場合はワールド座標が必要 (ParticlePassのビルボード計算と同様の手法)
	
		Math::Vector3 worldPos = transform->GetWorldPosition(registry);

		__declspec(align(16)) struct {
			Math::Vector3 position;
			float radius;
			uint32_t count;
			uint32_t emit;
			uint32_t pad[2];
		} emitterConstants;

		emitterConstants.position = worldPos;
		emitterConstants.radius = emitter->radius;
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

		emitter->frequencyTime += deltaTime;
		// 初回にコードから直接設定されたハンドルを保持し、以後の名前変更を反映する。
		if (!emitter->texture.IsValid() || (!emitter->loadedTextureName.empty() && emitter->loadedTextureName != emitter->textureName)
			|| (emitter->loadedTextureGeneration != 0 && emitter->loadedTextureGeneration != AssetManager::GetAddressableGeneration())) {
			emitter->texture = TextureManager::LoadCovertTexture(AssetManager::GetFilePathFromAddressableName(emitter->textureName));
		}
		emitter->loadedTextureName = emitter->textureName;
		emitter->loadedTextureGeneration = AssetManager::GetAddressableGeneration();

		if (emitter->frequency <= emitter->frequencyTime) {
			const auto* transform = registry.GetComponent<TransformComponent>(entity);
			EmitParticle(registry, transform->GetWorldPosition(registry), *emitter);
			emitter->frequencyTime -= emitter->frequency;
		}
	}
}

void ParticleEmitterSystem::EmitParticle(Registry& registry, const Math::Vector3& worldPosition, const Component::ParticleEmitterComponent& emitter) {
	// 発生範囲とオフセットはワールド軸に沿って適用する。
	const auto center = worldPosition + emitter.localPosition;
	const auto halfRange = emitter.emitRange / 2.0f;
	const auto minPosition = center - halfRange;
	const auto maxPosition = center + halfRange;
	for (uint32_t count = 0; count < emitter.count; count++) {
		auto e = registry.GenerateEntity();
		auto* particle = registry.AddComponent<ParticleComponent>(e);
		particle->lifeTime = Random::GetRandomVal(emitter.minLifeTime, emitter.maxLifeTime);
		particle->texture = emitter.texture;
		particle->shape = emitter.shape;
		particle->color = emitter.color;
		registry.AddComponent<VelocityComponent>(e)->linear = Random::GetRandomVal(emitter.minSpeed, emitter.maxSpeed);
		auto* transform = registry.AddComponent<TransformComponent>(e);
		transform->translate = Random::GetRandomVal(minPosition, maxPosition);
		transform->rotation = Math::Quaternion::IDENTITY;
		transform->scale = Random::GetRandomVal(emitter.minScale, emitter.maxScale);
	}
}
}
}
#include "../SystemManager.h"
REGISTER_SYSTEM(NoEngine::ECS::ParticleEmitterSystem, "ParticleEmitterSystem", "Effects")
