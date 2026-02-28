#pragma once
#include "engine/Functions/ECS/System/ISystem.h"

/// <summary>
/// 撃破演出システム
/// DeathParticleComponentを持つエンティティの演出を更新する
/// </summary>
class DeathEffectSystem : public NoEngine::ECS::ISystem {
public:
	DeathEffectSystem();
	~DeathEffectSystem() override = default;

	void Update(NoEngine::ECS::Registry& registry, float deltaTime) override;

private:
	/// <summary>
	/// パーティクルモデルが読み込まれているか確認
	/// </summary>
	bool isModelLoaded_;
};
