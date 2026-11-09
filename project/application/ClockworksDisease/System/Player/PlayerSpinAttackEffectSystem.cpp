#include "stdafx.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "engine/Assets/Model/ModelSaver.h"

namespace {
constexpr float kEmissionInterval = 1.0f / 90.0f;
constexpr float kParticleLifetime = 0.18f;
constexpr float kParticleSize = 0.48f;
constexpr float kMaxSampleDistance = 2.0f;
constexpr int kMaxParticlesPerFrame = 4;

struct SpinAttackEffectState {
	std::optional<No::Vector3> previousToe;
	float timeToNextParticle = kEmissionInterval;
};

std::optional<No::Vector3> GetToePosition(No::Registry& registry, No::Entity entity) {
	const auto* action = registry.GetComponent<PlayerActionComponent>(entity);
	// 攻撃開始フレームはAnimationSystemがまだ攻撃ポーズを計算していない。
	if (action->activeAction != PlayerActionType::kSpinAttack || action->elapsedTime <= 0.0f) return std::nullopt;

	const auto* animator = registry.GetComponent<No::AnimatorComponent>(entity);
	const auto* mesh = registry.GetComponent<No::MeshComponent>(entity);
	if (!mesh->isVisible || !animator->IsPlaying() || animator->runtime.meshHandle != mesh->handle
		|| animator->runtime.skeletonHandle != animator->skeletonHandle
		|| animator->currentAnimation >= animator->animationHandles.size()) return std::nullopt;
	const auto* clip = NoEngine::ModelSaver::Get().GetAnimation(animator->animationHandles[animator->currentAnimation]);
	if (!clip || clip->name != "HurricaneKick") return std::nullopt;

	const auto& skeleton = animator->runtime.skeleton;
	const auto toe = skeleton.jointMap.find("mixamorig:LeftToe_End");
	if (toe == skeleton.jointMap.end()) return std::nullopt;
	const auto* transform = registry.GetComponent<No::TransformComponent>(entity);
	// メッシュ描画と同じ変換を使い、アニメーションによるモデル全体の回転も反映する。
	auto modelToWorld = animator->local.MakeAffineMatrix4x4(registry) * transform->MakeAffineMatrix4x4(registry);
	const auto position = modelToWorld.Transform(skeleton.joints[toe->second].skeletonSpaceMatrix.GetTranslate());
	if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z)) return std::nullopt;
	return position;
}

class PlayerSpinAttackEffectSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override {
		if (!std::isfinite(deltaTime) || deltaTime <= 0.0f) return;

		std::vector<No::Vector3> particlePositions;
		for (auto entity : registry.View<PlayerActionComponent, No::AnimatorComponent,
			No::TransformComponent, No::MeshComponent>()) {
			auto* state = registry.GetComponent<SpinAttackEffectState>(entity);
			const auto toe = GetToePosition(registry, entity);
			if (!toe) {
				if (state) *state = {};
				continue;
			}
			if (!state) state = registry.AddComponent<SpinAttackEffectState>(entity);

			// ワープなどで位置が飛んだときは前の点とつながない。
			if (!state->previousToe || (*toe - *state->previousToe).LengthSquared() > kMaxSampleDistance * kMaxSampleDistance) {
				particlePositions.push_back(*toe);
				state->timeToNextParticle = kEmissionInterval;
			} else {
				// フレーム間を補間し、処理落ち時の発生数に上限を設ける。
				const float frameTime = std::min(deltaTime, kEmissionInterval * kMaxParticlesPerFrame);
				for (int i = 0; i < kMaxParticlesPerFrame && state->timeToNextParticle <= frameTime; ++i) {
					const float t = state->timeToNextParticle / frameTime;
					particlePositions.push_back(*state->previousToe + (*toe - *state->previousToe) * t);
					state->timeToNextParticle += kEmissionInterval;
				}
				state->timeToNextParticle = std::max(0.0f, state->timeToNextParticle - frameTime);
			}
			state->previousToe = *toe;
		}

		if (particlePositions.empty()) return;
		if (!texture_.IsValid()) {
			texture_ = NoEngine::TextureManager::LoadCovertTexture("resources/game/ClockworksDisease/Texture/Effect/getEffect.png");
		}
		if (!texture_.IsValid()) return;
		No::EffectPreset preset;
		preset.count = 1;
		preset.spawnShape = No::EffectSpawnShape::kPoint;
		preset.minScale = preset.maxScale = No::Vector3{ kParticleSize, kParticleSize, kParticleSize };
		preset.minSpeed = preset.maxSpeed = No::Vector3::ZERO;
		preset.minLifeTime = preset.maxLifeTime = kParticleLifetime;
		preset.minRotate = preset.maxRotate = 0.0f;
		preset.color = No::Color(0.3f, 0.85f, 1.0f, 1.0f);
		preset.texture = texture_;
		for (const auto& position : particlePositions) {
			registry.PlayEffect(preset, position);
		}
	}

private:
	NoEngine::TextureRef texture_;
};
}

REGISTER_SYSTEM(PlayerSpinAttackEffectSystem, "PlayerSpinAttackEffectSystem", "Gameplay")
